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

#include <kpmcore/core/partitiontable.h>

#include <QMutex>
#include <QPointer>
#include <QSet>

class PartitionCoreModule;

using SwapChoiceSet = QSet< PartitionActions::Choices::SwapChoice >;

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

class BootloaderModel : public QAbstractListModel
{
    Q_OBJECT
    using QAbstractListModel::QAbstractListModel;

public:
    int currentIndex() const;
};


class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY( DeviceModel* devices READ devices NOTIFY devicesChanged FINAL )
    Q_PROPERTY( QString prettyStatus READ prettyStatus NOTIFY prettyStatusChanged FINAL )
    Q_PROPERTY( InstallChoice installChoice READ installChoice WRITE setInstallChoice NOTIFY installChoiceChanged)
    Q_PROPERTY( bool enableEncryption READ enableEncryption WRITE setEnableEncryption NOTIFY enableEncryptionChanged)
    Q_PROPERTY( QString encryptionPassphrase WRITE setEncryptionPassphrase NOTIFY encryptionPassphraseChanged)
    Q_PROPERTY( QString encryptionConfirmPassphrase WRITE setEncryptionConfirmPassphrase NOTIFY encryptionConfirmPassphraseChanged)
    Q_PROPERTY( bool deviceEditable MEMBER m_deviceEditable NOTIFY deviceEditableChanged FINAL)
    Q_PROPERTY( bool canErase MEMBER m_canErase NOTIFY canEraseChanged FINAL)
    Q_PROPERTY( bool canInstallAlongside MEMBER m_canInstallAlongside NOTIFY canInstallAlongsideChanged FINAL)
    Q_PROPERTY( bool canReplace MEMBER m_canReplace NOTIFY canReplaceChanged FINAL)
    Q_PROPERTY( bool canSomethingElse MEMBER m_canSomethingElse NOTIFY canSomethingElseChanged FINAL)

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

    enum class Encryption : unsigned short
    {
        Disabled = 0,
        Unconfirmed,
        Confirmed
    };

    Config(QObject* parent = nullptr );

    void init(const SwapChoiceSet& swapChoices, PartitionCoreModule* core);

    QString prettyStatus() const;

    void onActivate();
    void finalize();

    void setInstallChoice(const InstallChoice &value);
    InstallChoice installChoice() const;

    void setEnableEncryption(const bool &value);
    bool enableEncryption() const;

    void setEncryptionConfirmPassphrase(const QString &value);
    void setEncryptionPassphrase(const QString &value);

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
    QAbstractItemModel * m_bootloaderModel;

    PartitionCoreModule* m_core;
    InstallChoice m_installChoice;
    bool m_isEfi;
    bool m_nextEnabled;

    void updateNextEnabled();
    Config::Encryption encryptionState() const;
    void updateEncryptionState();

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

//     QButtonGroup* m_grp;
//     PrettyRadioButton* m_alongsideButton;
//     PrettyRadioButton* m_eraseButton;
//     PrettyRadioButton* m_replaceButton;
//     PrettyRadioButton* m_somethingElseButton;
//     QComboBox* m_eraseSwapChoiceComboBox;  // UI, see also m_eraseSwapChoice

    DeviceInfo* m_deviceInfo;

//     QPointer< PartitionBarsView > m_beforePartitionBarsView;
//     QPointer< PartitionLabelsView > m_beforePartitionLabelsView;
//     QPointer< PartitionBarsView > m_afterPartitionBarsView;
//     QPointer< PartitionLabelsView > m_afterPartitionLabelsView;
//     QPointer< PartitionSplitterWidget > m_afterPartitionSplitterWidget;
//     QPointer< QComboBox > m_bootloaderComboBox;
//     QPointer< QLabel > m_efiLabel;
//     QPointer< QComboBox > m_efiComboBox;

    int m_lastSelectedDeviceIndex;

    QString m_defaultFsType;
    bool m_enableEncryption;
    SwapChoiceSet m_availableSwapChoices;  // What is available
    PartitionActions::Choices::SwapChoice m_eraseSwapChoice;  // what is selected

    bool m_allowManualPartitioning;

    QMutex m_coreMutex;

    QString m_encryptionPassphrase;
    QString m_encryptionConfirmPassphrase;

    bool m_deviceEditable;
    bool m_canErase;
    bool m_canInstallAlongside;
    bool m_canReplace;
    bool m_canSomethingElse;

    bool m_efiAvailable;
    bool m_bootloaderAvailable;
    bool m_reuseHome;

    Config::Encryption  m_encryptionState = Config::Encryption::Unconfirmed;

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
    DeviceModel* devices() const;

signals:
    void nextStatusChanged( bool );
    void actionChosen();
    void deviceChosen();

    void prettyStatusChanged();
    void devicesChanged();
    void installChoiceChanged();
    void enableEncryptionChanged();
    void encryptionConfirmPassphraseChanged();
    void encryptionPassphraseChanged();
    void deviceEditableChanged();
    void canEraseChanged();
    void canInstallAlongsideChanged();
    void canReplaceChanged();
    void canSomethingElseChanged();
    void encryptionStateChanged();
    void efiAvailableChanged();
    void reuseHomeChanged();
};

#endif
