/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2019-2020, Adriaan de Groot <groot@kde.org>
 *   Copyright 2020, Camilo Higuita <milo.h@aol.com>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARTITION_CONFIG_H
#define PARTITION_CONFIG_H

#include "Job.h"

#include "core/OsproberEntry.h"
#include "core/PartitionActions.h"
#include "core/DeviceModel.h"

#include <QMutex>
#include <QPointer>
#include <QSet>

class PartitionCoreModule;

using SwapChoiceSet = QSet< PartitionActions::Choices::SwapChoice >;

class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY( DeviceModel* devices READ devices NOTIFY devicesChanged FINAL )
    Q_PROPERTY( QString prettyStatus READ prettyStatus NOTIFY prettyStatusChanged FINAL )
    Q_PROPERTY( InstallChoice installChoice READ installChoice WRITE setInstallChoice NOTIFY installChoiceChanged)

public:
    enum InstallChoice
    {
        NoChoice,
        Alongside,
        Erase,
        Replace,
        Manual
    };
    Q_ENUM(InstallChoice)

    Config( QObject* parent = nullptr );

    void init(const SwapChoiceSet& swapChoices, PartitionCoreModule* core);

    QString prettyStatus() const;

    void onActivate();
    void finalize();

    void setInstallChoice(const InstallChoice &value);
    InstallChoice installChoice() const;

private:
    DeviceModel* m_deviceModel;
    InstallChoice m_installChoice = InstallChoice::NoChoice;
    PartitionCoreModule* m_core;

protected:
    DeviceModel* devices() const;

signals:
    void prettyStatusChanged();
    void devicesChanged();
    void installChoiceChanged();
};

#endif
