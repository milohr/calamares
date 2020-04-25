/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2019-2020, Adriaan de Groot <groot@kde.org>
 *   Copyright 2020, Camilo Higuita <milo.h@aol.com> *
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

#include "Config.h"
#include "core/BootLoaderModel.h"
#include "core/KPMHelpers.h"
#include "core/OsproberEntry.h"
#include "core/PartUtils.h"
#include "core/PartitionActions.h"
#include "core/PartitionCoreModule.h"
#include "core/PartitionInfo.h"
#include "core/PartitionModel.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "partition/PartitionIterator.h"
#include "partition/PartitionQuery.h"
#include "utils/Logger.h"
#include "utils/Retranslator.h"
#include "utils/Units.h"

#include "Branding.h"
#include "utils/CalamaresUtilsGui.h"

#include <kpmcore/core/device.h>
#include <kpmcore/core/partition.h>
#ifdef WITH_KPMCORE4API
#include <kpmcore/core/softwareraid.h>
#endif

using PartitionActions::Choices::SwapChoice;
using CalamaresUtils::Partition::PartitionIterator;
using CalamaresUtils::Partition::isPartitionFreeSpace;
using CalamaresUtils::Partition::findPartitionByPath;

Config::Config( QObject* parent )
: QObject( parent )
, m_deviceModel(nullptr)
{
}

void
Config::init(const SwapChoiceSet& swapChoices, PartitionCoreModule* core)
{
    m_core = core;
//     m_isEfi = PartUtils::isEfiSystem();

//     setupChoices();


    // We need to do this because a PCM revert invalidates the deviceModel.
    connect( core, &PartitionCoreModule::reverted,
             this, [=]
             {
//                  m_drivesCombo->setModel( core->deviceModel() );
//                  m_drivesCombo->setCurrentIndex( m_lastSelectedDeviceIndex );
             } );
    m_deviceModel = core->deviceModel();
    emit devicesChanged();

//     connect( m_drivesCombo,
//              static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ),
//              this, &ChoicePage::applyDeviceChoice );
//
//     connect( m_encryptWidget, &EncryptWidget::stateChanged,
//              this, &ChoicePage::onEncryptWidgetStateChanged );
//     connect( m_reuseHomeCheckBox, &QCheckBox::stateChanged,
//              this, &ChoicePage::onHomeCheckBoxStateChanged );
//
//     ChoicePage::applyDeviceChoice();
}

QString
Config::prettyStatus() const
{
return "hola";
}

void
Config::onActivate()
{

}

void
Config::finalize()
{

}

DeviceModel *
Config::devices() const
{
    return m_deviceModel;
}

Config::InstallChoice Config::installChoice() const
{
    return m_installChoice;
}

void Config::setInstallChoice(const Config::InstallChoice& value)
{
    if(m_installChoice == value)
        return;

    m_installChoice = value;
    emit installChoiceChanged();
}




