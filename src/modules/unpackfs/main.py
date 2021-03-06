#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# === This file is part of Calamares - <https://github.com/calamares> ===
#
#   Copyright 2014, Teo Mrnjavac <teo@kde.org>
#   Copyright 2014, Daniel Hillenbrand <codeworkx@bbqlinux.org>
#   Copyright 2014, Philip Müller <philm@manjaro.org>
#   Copyright 2017, Alf Gaida <agaida@siduction.org>
#   Copyright 2019, Kevin Kofler <kevin.kofler@chello.at>
#   Copyright 2020, Adriaan de Groot <groot@kde.org>
#   Copyright 2020, Gabriel Craciunescu <crazy@frugalware.org>
#
#   Calamares is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Calamares is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Calamares. If not, see <http://www.gnu.org/licenses/>.

import os
import re
import shutil
import subprocess
import sys
import tempfile

from libcalamares import *
from libcalamares.utils import mount

import gettext
_ = gettext.translation("calamares-python",
                        localedir=utils.gettext_path(),
                        languages=utils.gettext_languages(),
                        fallback=True).gettext

def pretty_name():
    return _("Filling up filesystems.")


class UnpackEntry:
    """
    Extraction routine using rsync.

    :param source:
    :param sourcefs:
    :param destination:
    """
    __slots__ = ['source', 'sourcefs', 'destination', 'copied', 'total', 'exclude', 'excludeFile']

    def __init__(self, source, sourcefs, destination):
        """
        @p source is the source file name (might be an image file, or
            a directory, too)
        @p sourcefs is a type indication; "file" is special, as is
            "squashfs".
        @p destination is where the files from the source go. This is
            **already** prefixed by rootMountPoint, so should be a
            valid absolute path within the host system.

        The members copied and total are filled in by the copying process.
        """
        self.source = source
        self.sourcefs = sourcefs
        self.destination = destination
        self.exclude = None
        self.excludeFile = None
        self.copied = 0
        self.total = 0

    def is_file(self):
        return self.sourcefs == "file"


ON_POSIX = 'posix' in sys.builtin_module_names


def global_excludes():
    """
    List excludes for rsync.
    """
    lst = []
    extra_mounts = globalstorage.value("extraMounts")
    if extra_mounts is None:
        extra_mounts = []

    for extra_mount in extra_mounts:
        mount_point = extra_mount["mountPoint"]

        if mount_point:
            lst.extend(['--exclude', mount_point + '/'])

    return lst

def file_copy(source, entry, progress_cb):
    """
    Extract given image using rsync.

    :param source: Source file. This may be the place the entry's
        image is mounted, or if it's a single file, the entry's source value.
    :param entry: The UnpackEntry being copied.
    :param progress_cb: A callback function for progress reporting.
        Takes a number and a total-number.
    """
    dest = entry.destination

    # Environment used for executing rsync properly
    # Setting locale to C (fix issue with tr_TR locale)
    at_env = os.environ
    at_env["LC_ALL"] = "C"

    # `source` *must* end with '/' otherwise a directory named after the source
    # will be created in `dest`: ie if `source` is "/foo/bar" and `dest` is
    # "/dest", then files will be copied in "/dest/bar".
    if not source.endswith("/") and not os.path.isfile(source):
        source += "/"

    num_files_total_local = 0
    num_files_copied = 0  # Gets updated through rsync output

    args = ['rsync', '-aHAXr']
    args.extend(global_excludes())
    if entry.excludeFile:
        args.extend(["--exclude-from=" + entry.excludeFile])
    if entry.exclude:
        for f in entry.exclude:
            args.extend(["--exclude", f])
    args.extend(['--progress', source, dest])
    process = subprocess.Popen(
        args, env=at_env,
        stdout=subprocess.PIPE, close_fds=ON_POSIX
        )
    # last_num_files_copied trails num_files_copied, and whenever at least 100 more
    # files have been copied, progress is reported and last_num_files_copied is updated.
    last_num_files_copied = 0

    for line in iter(process.stdout.readline, b''):
        # rsync outputs progress in parentheses. Each line will have an
        # xfer and a chk item (either ir-chk or to-chk) as follows:
        #
        # - xfer#x => Interpret it as 'file copy try no. x'
        # - ir-chk=x/y, where:
        #     - x = number of files yet to be checked
        #     - y = currently calculated total number of files.
        # - to-chk=x/y, which is similar and happens once the ir-chk
        #   phase (collecting total files) is over.
        #
        # If you're copying directory with some links in it, the xfer#
        # might not be a reliable counter (for one increase of xfer, many
        # files may be created).
        m = re.findall(r'xfr#(\d+), ..-chk=(\d+)/(\d+)', line.decode())

        if m:
            # we've got a percentage update
            num_files_remaining = int(m[0][1])
            num_files_total_local = int(m[0][2])
            # adjusting the offset so that progressbar can be continuesly drawn
            num_files_copied = num_files_total_local - num_files_remaining

            # I guess we're updating every 100 files...
            if num_files_copied - last_num_files_copied >= 100:
                last_num_files_copied = num_files_copied
                progress_cb(num_files_copied, num_files_total_local)

    process.wait()
    progress_cb(num_files_copied, num_files_total_local)  # Push towards 100%

    # 23 is the return code rsync returns if it cannot write extended
    # attributes (with -X) because the target file system does not support it,
    # e.g., the FAT EFI system partition. We need -X because distributions
    # using file system capabilities and/or SELinux require the extended
    # attributes. But distributions using SELinux may also have SELinux labels
    # set on files under /boot/efi, and rsync complains about those. The only
    # clean way would be to split the rsync into one with -X and
    # --exclude /boot/efi and a separate one without -X for /boot/efi, but only
    # if /boot/efi is actually an EFI system partition. For now, this hack will
    # have to do. See also:
    # https://bugzilla.redhat.com/show_bug.cgi?id=868755#c50
    # for the same issue in Anaconda, which uses a similar workaround.
    if process.returncode != 0 and process.returncode != 23:
        utils.warning("rsync failed with error code {}.".format(process.returncode))
        return _("rsync failed with error code {}.").format(process.returncode)

    return None


class UnpackOperation:
    """
    Extraction routine using unsquashfs.

    :param entries:
    """

    def __init__(self, entries):
        self.entries = entries
        self.entry_for_source = dict((x.source, x) for x in self.entries)

    def report_progress(self):
        """
        Pass progress to user interface
        """
        progress = float(0)

        done = 0
        total = 0
        complete = 0
        for entry in self.entries:
            if entry.total == 0:
                continue
            total += entry.total
            done += entry.copied
            if entry.total == entry.copied:
                complete += 1

        if done > 0 and total > 0:
            progress = 0.05 + (0.90 * done / total) + (0.05 * complete / len(self.entries))

        job.setprogress(progress)

    def run(self):
        """
        Extract given image using unsquashfs.

        :return:
        """
        source_mount_path = tempfile.mkdtemp()

        try:
            for entry in self.entries:
                imgbasename = os.path.splitext(
                    os.path.basename(entry.source))[0]
                imgmountdir = os.path.join(source_mount_path, imgbasename)
                os.makedirs(imgmountdir, exist_ok=True)

                self.mount_image(entry, imgmountdir)

                fslist = ""

                if entry.sourcefs == "squashfs":
                    if shutil.which("unsquashfs") is None:
                        utils.warning("Failed to find unsquashfs")

                        return (_("Failed to unpack image \"{}\"").format(entry.source),
                                _("Failed to find unsquashfs, make sure you have the squashfs-tools package installed"))

                    fslist = subprocess.check_output(
                        ["unsquashfs", "-l", entry.source]
                        )

                elif entry.sourcefs == "ext4":
                    fslist = subprocess.check_output(
                        ["find", imgmountdir, "-type", "f"]
                        )

                elif entry.is_file():
                    # Hasn't been mounted, copy directly; find handles both
                    # files and directories.
                    fslist = subprocess.check_output(["find", entry.source, "-type", "f"])

                entry.total = len(fslist.splitlines())

                self.report_progress()
                error_msg = self.unpack_image(entry, imgmountdir)

                if error_msg:
                    return (_("Failed to unpack image \"{}\"").format(entry.source),
                            error_msg)

            return None
        finally:
            shutil.rmtree(source_mount_path, ignore_errors=True, onerror=None)

    def mount_image(self, entry, imgmountdir):
        """
        Mount given @p entry as loop device on @p imgmountdir.

        A *file* entry (e.g. one with *sourcefs* set to *file*)
        is not mounted and just ignored.

        :param entry: the entry to mount (source is the important property)
        :param imgmountdir: where to mount it

        :returns: None, but throws if the mount failed
        """
        if entry.is_file():
            return

        if os.path.isdir(entry.source):
            r = mount(entry.source, imgmountdir, "", "--bind")
        elif os.path.isfile(entry.source):
            r = mount(entry.source, imgmountdir, entry.sourcefs, "loop")
        else: # entry.source is a device
            r = mount(entry.source, imgmountdir, entry.sourcefs, "")

        if r != 0:
            raise subprocess.CalledProcessError(r, "mount")


    def unpack_image(self, entry, imgmountdir):
        """
        Unpacks image.

        :param entry:
        :param imgmountdir:
        :return:
        """
        def progress_cb(copied, total):
            """ Copies file to given destination target.

            :param copied:
            """
            entry.copied = copied
            if total > entry.total:
                entry.total = total
            self.report_progress()

        try:
            if entry.is_file():
                source = entry.source
            else:
                source = imgmountdir

            return file_copy(source, entry, progress_cb)
        finally:
            if not entry.is_file():
                subprocess.check_call(["umount", "-l", imgmountdir])


def get_supported_filesystems_kernel():
    """
    Reads /proc/filesystems (the list of supported filesystems
    for the current kernel) and returns a list of (names of)
    those filesystems.
    """
    PATH_PROCFS = '/proc/filesystems'

    if os.path.isfile(PATH_PROCFS) and os.access(PATH_PROCFS, os.R_OK):
        with open(PATH_PROCFS, 'r') as procfile:
            filesystems = procfile.read()
            filesystems = filesystems.replace(
                "nodev", "").replace("\t", "").splitlines()
            return filesystems

    return []


def get_supported_filesystems():
    """
    Returns a list of all the supported filesystems
    (valid values for the *sourcefs* key in an item.
    """
    return ["file"] + get_supported_filesystems_kernel()


def run():
    """
    Unsquash filesystem.
    """
    root_mount_point = globalstorage.value("rootMountPoint")

    if not root_mount_point:
        utils.warning("No mount point for root partition")
        return (_("No mount point for root partition"),
                _("globalstorage does not contain a \"rootMountPoint\" key, "
                "doing nothing"))

    if not os.path.exists(root_mount_point):
        utils.warning("Bad root mount point \"{}\"".format(root_mount_point))
        return (_("Bad mount point for root partition"),
                _("rootMountPoint is \"{}\", which does not "
                "exist, doing nothing").format(root_mount_point))

    supported_filesystems = get_supported_filesystems()

    # Bail out before we start when there are obvious problems
    for entry in job.configuration["unpack"]:
        source = os.path.abspath(entry["source"])
        sourcefs = entry["sourcefs"]

        if sourcefs not in supported_filesystems:
            utils.warning("The filesystem for \"{}\" ({}) is not supported by your current kernel".format(source, sourcefs))
            utils.warning(" ... modprobe {} may solve the problem".format(sourcefs))
            return (_("Bad unsquash configuration"),
                    _("The filesystem for \"{}\" ({}) is not supported by your current kernel").format(source, sourcefs))
        if not os.path.exists(source):
            utils.warning("The source filesystem \"{}\" does not exist".format(source))
            return (_("Bad unsquash configuration"),
                    _("The source filesystem \"{}\" does not exist").format(source))

    unpack = list()

    is_first = True
    for entry in job.configuration["unpack"]:
        source = os.path.abspath(entry["source"])
        sourcefs = entry["sourcefs"]
        destination = os.path.abspath(root_mount_point + entry["destination"])

        if not os.path.isdir(destination) and sourcefs != "file":
            utils.warning(("The destination \"{}\" in the target system is not a directory").format(destination))
            if is_first:
                return (_("Bad unsquash configuration"),
                        _("The destination \"{}\" in the target system is not a directory").format(destination))
            else:
                utils.debug(".. assuming that the previous targets will create that directory.")

        unpack.append(UnpackEntry(source, sourcefs, destination))
        # Optional settings
        if entry.get("exclude", None):
            unpack[-1].exclude = entry["exclude"]
        if entry.get("excludeFile", None):
            unpack[-1].excludeFile = entry["excludeFile"]

        is_first = False

    unpackop = UnpackOperation(unpack)

    return unpackop.run()
