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
#include "core/BootLoaderModel.h"

#include <kpmcore/core/partitiontable.h>

#include <QMutex>
#include <QPointer>
#include <QSet>
#include <QVector>

class PartitionCoreModule;

using SwapChoiceSet = QSet< PartitionActions::Choices::SwapChoice >;

class PreviewBar : public QObject
{
    Q_OBJECT
    explicit PreviewBar(QObject * parent = nullptr ) : QObject(parent) {}
};

class Opt : public QObject // to wrap any optional choice
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool checked READ checked NOTIFY checkedChanged FINAL)
    Q_PROPERTY(QString label READ label NOTIFY labelChanged FINAL)
    Q_PROPERTY(QString tooltip READ tooltip NOTIFY tooltipChanged FINAL)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged FINAl)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged FINAl)

public:
    explicit Opt(QObject * parent = nullptr ) : QObject(parent) {}

    void setVisible(const bool &value)
    {
        if(m_visible == value)
            return;

        m_visible = value;
        emit visibleChanged();
    }

    bool visible() const
    {
        return m_visible;
    }

    void setEnabled(const bool &value)
    {
        if(m_enabled == value)
            return;

        m_enabled = value;
        emit enabledChanged();
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void setChecked(const bool &value)
    {
        if(m_checked == value)
            return;

        m_checked = value;
        emit checkedChanged();
    }

    bool checked() const
    {
        return m_checked;
    }

    void setLabel(const QString &value)
    {
        if(m_label == value)
            return;

        m_label = value;
        emit labelChanged();
    }

    QString label() const
    {
        return m_label;
    }

    void setMessage(const QString &value)
    {
        if(m_message == value)
            return;

        m_message = value;
        emit messageChanged();
    }

    QString message() const
    {
        return m_message;
    }

    void setTooltip(const QString &value)
    {
        if(m_tooltip == value)
            return;

        m_tooltip = value;
        emit tooltipChanged();
    }

    QString tooltip() const
    {
        return m_tooltip;
    }

    void setIcon(const QString &value)
    {
        if(m_icon == value)
            return;

        m_icon = value;
        emit iconChanged();
    }

    QString icon() const
    {
        return m_icon;
    }

    void hide()
    {
        this->setVisible(false);
    }

    void show()
    {
        this->setVisible(true);
    }

private:
    QString m_label;
    QString m_message;
    QString m_tooltip;
    QString m_icon;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_checked = false;

signals:
    void visibleChanged();
    void enabledChanged();
    void checkedChanged();
    void labelChanged();
    void tooltipChanged();
    void messageChanged();
    void iconChanged();
};

class OptGroup : public QObject //to group opt that are autoexclusive
{
    Q_OBJECT
    using Opts = QHash<int, Opt*>;
public:
    explicit OptGroup(QObject * parent = nullptr) : QObject(parent) {}

    void addOpt(Opt * opt, const int &id = -1)
    {
        m_opts.insert(id, opt);
        connect(opt, &Opt::checkedChanged, [this, id]()
        {
            auto opt = m_opts[id];
            if(opt->checked())
            {
                this->m_checkedOpt = opt;
            }

           emit this->optToggled(id, opt->checked());
        });
    }

    Opt* checkedOpt()
    {
        return m_checkedOpt;
    }

    void setAutoExclusive(const bool &value)
    {
        if(m_autoExclusive == value)
            return;

        m_autoExclusive = value;
        emit autoExclusiveChanged();

        uncheckAll();
    }

    void uncheckAll()
    {
        for(auto opt : m_opts)
            opt->setChecked(false);
    }

private:
   Opts m_opts;
   Opt * m_checkedOpt = nullptr;
   bool m_autoExclusive = true;

signals:
    void optToggled(int id, bool checked);
    void autoExclusiveChanged();
};

class EncryptOpt : public Opt
{
  Q_OBJECT
  Q_PROPERTY( QString encryptionPassphrase WRITE setEncryptionPassphrase NOTIFY encryptionPassphraseChanged)
  Q_PROPERTY( QString encryptionConfirmPassphrase WRITE setEncryptionConfirmPassphrase NOTIFY encryptionConfirmPassphraseChanged)

public:
    enum class Encryption : unsigned short
    {
        Disabled = 0,
        Unconfirmed,
        Confirmed
    };

    using Opt::Opt;
    void setEncryptionConfirmPassphrase(const QString &value);
    void setEncryptionPassphrase(const QString &value);
    EncryptOpt::Encryption encryptionState() const;

private:
    QString m_encryptionPassphrase;
    QString m_encryptionConfirmPassphrase;

    void updateEncryptionState();

    EncryptOpt::Encryption  m_encryptionState = EncryptOpt::Encryption::Unconfirmed;

signals:
    void encryptionConfirmPassphraseChanged();
    void encryptionPassphraseChanged();
    void encryptionStateChanged();

};

class DeviceInfo : public QObject
{
  Q_OBJECT
  Q_PROPERTY (QString label MEMBER m_ptLabel NOTIFY tableTypeChanged)
  Q_PROPERTY (QString icon MEMBER m_ptIcon NOTIFY tableTypeChanged)

public :
  explicit DeviceInfo(QObject *parent = nullptr);
  void setPartitionTableType( PartitionTable::TableType type );

private:
    QString m_ptIcon;
    QString m_ptLabel;
    PartitionTable::TableType m_tableType;

signals:
    void tableTypeChanged();
};

class EfiModel : public QAbstractListModel
{
Q_OBJECT
using QAbstractListModel::QAbstractListModel;

public:
    int currentIndex() const;
};

class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY( DeviceModel* devices READ devicesModel NOTIFY devicesModelChanged FINAL )
    Q_PROPERTY( BootLoaderModel* bootloaders READ bootloaderModel NOTIFY bootloaderModelChanged FINAL )
    Q_PROPERTY( QString prettyStatus READ prettyStatus NOTIFY prettyStatusChanged FINAL )
    Q_PROPERTY( InstallChoice installChoice READ installChoice WRITE setInstallChoice NOTIFY installChoiceChanged)

    Q_PROPERTY( bool deviceEditable MEMBER m_deviceEditable NOTIFY deviceEditableChanged FINAL)

    Q_PROPERTY( Opt *eraseOption MEMBER m_eraseOption NOTIFY eraseOptionChanged FINAL)
    Q_PROPERTY( Opt *alongsideOption MEMBER m_alongsideOption NOTIFY alongsideOptionChanged FINAL)
    Q_PROPERTY( Opt *replaceOption MEMBER m_replaceOption NOTIFY replaceOptionChanged FINAL)
    Q_PROPERTY( Opt *somethingElseOption MEMBER m_somethingElseOption NOTIFY somethingElseOptionChanged FINAL)
    Q_PROPERTY( Opt *encryptOption MEMBER m_encryptOption NOTIFY encryptOptionChanged FINAL)

    Q_PROPERTY( bool efiAvailable MEMBER m_efiAvailable NOTIFY efiAvailableChanged FINAL)

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

    Config(QObject* parent = nullptr );

    void init(const SwapChoiceSet& swapChoices, PartitionCoreModule* core);

    QString prettyStatus() const;

    void onActivate();
    void finalize();

    void setInstallChoice(const InstallChoice &value);
    InstallChoice installChoice() const;


    /**
     * @brief applyActionChoice reacts to a choice of partitioning mode.
     * @param choice the partitioning action choice.
     */
    void applyActionChoice( Config::InstallChoice choice );

    int lastSelectedDeviceIndex();
    void setLastSelectedDeviceIndex( int index );

    void setReuseHome(const bool &value);
    bool reuseHome() const;
    /**
     * @brief onLeave runs when control passes from this page to another one.
     */
    void onLeave();

private:
    DeviceModel* m_deviceModel;
    EfiModel * m_efiModel;
    BootLoaderModel * m_bootloaderModel;

    PartitionCoreModule* m_core;
    InstallChoice m_installChoice;
    bool m_isEfi;
    bool m_nextEnabled;

    void updateNextEnabled();

    void setupChoices();
//     QComboBox* createBootloaderComboBox( QWidget* parentButton );
    Device* selectedDevice();

    /* Change the UI depending on the device selected. */
    void hideButtons();  // Hide everything when no device
    void applyDeviceChoice();  // Start scanning new device
    void continueApplyDeviceChoice();  // .. called after scan

    void updateDeviceStatePreview();
    void updateActionChoicePreview( Config::InstallChoice choice );
    void setupActions();
    OsproberEntryList getOsproberEntriesForDevice( Device* device ) const;
    void doAlongsideApply();
    void setupEfiSystemPartitionSelector();

    // Translations support
//     void updateSwapChoicesTr( QComboBox* box );


    QMutex m_previewsMutex;


//     QComboBox* m_drivesCombo;

    OptGroup* m_grp;
    Opt* m_alongsideOption;
    Opt* m_eraseOption;
    Opt* m_replaceOption;
    Opt* m_somethingElseOption;

    EncryptOpt *m_encryptOption;
//     QComboBox* m_eraseSwapChoiceComboBox;  // UI, see also m_eraseSwapChoice

    DeviceInfo* m_deviceInfo;

    QPointer< PreviewBar > m_beforePartitionBarsView;
//     QPointer< PartitionLabelsView > m_beforePartitionLabelsView;
    QPointer< PreviewBar > m_afterPartitionBarsView;
//     QPointer< PartitionLabelsView > m_afterPartitionLabelsView;
//     QPointer< PartitionSplitterWidget > m_afterPartitionSplitterWidget;
//     QPointer< QComboBox > m_bootloaderComboBox;
//     QPointer< QLabel > m_efiLabel;
//     QPointer< QComboBox > m_efiComboBox;

    int m_lastSelectedDeviceIndex;

    QString m_defaultFsType;
    SwapChoiceSet m_availableSwapChoices;  // What is available
    PartitionActions::Choices::SwapChoice m_eraseSwapChoice;  // what is selected

    bool m_allowManualPartitioning;

    QMutex m_coreMutex;


    bool m_deviceEditable;


    bool m_efiAvailable;
    bool m_bootloaderAvailable;
    bool m_reuseHome;


    QString m_prettyStatus;

private slots:
    void onPartitionToReplaceSelected( const QModelIndex& current, const QModelIndex& previous );
    void doReplaceSelectedPartition( const QModelIndex& current );
    void doAlongsideSetupSplitter( const QModelIndex& current, const QModelIndex& previous );
    void onEncryptWidgetStateChanged();
    void onHomeCheckBoxStateChanged();

    /// @brief Calls applyActionChoice() as needed.
    void onActionChanged();
    /// @brief Calls onActionChanged() as needed.
    void onEraseSwapChoiceChanged();

protected:
    DeviceModel* devicesModel() const;

signals:
    void nextStatusChanged( bool );
    void actionChosen();
    void deviceChosen();

    void prettyStatusChanged();
    void devicesModelChanged();
    void bootloaderModelChanged();
    void installChoiceChanged();

    void deviceEditableChanged();

    void eraseOptionChanged();
    void alongsideOptionChanged();
    void replaceOption();
    void somethingElseOptionChanged();
    void encryptOptionChanged();


    void efiAvailableChanged();
    void reuseHomeChanged();
    void isEfiChanged();
};

#endif
