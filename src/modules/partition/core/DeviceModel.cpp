/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2014, Aurélien Gâteau <agateau@kde.org>
 *   Copyright 2014, Teo Mrnjavac <teo@kde.org>
 *   Copyright 2019, Adriaan de Groot <groot@kde.org>
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
#include "core/DeviceModel.h"

#include "core/PartitionModel.h"

#include "utils/CalamaresUtilsGui.h"
#include "utils/Logger.h"

// KPMcore
#include <kpmcore/core/device.h>

// KF5
#include <KFormat>

#include <QIcon>
#include <QStandardItemModel>

// STL
#include <algorithm>

static void
sortDevices( DeviceModel::DeviceList& l )
{
    std::sort( l.begin(), l.end(), []( const Device* dev1, const Device* dev2 ) {
        return dev1->deviceNode() < dev2->deviceNode();
    } );
}

DeviceModel::DeviceModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

DeviceModel::~DeviceModel() {}

void
DeviceModel::init( const DeviceList& devices )
{
    beginResetModel();
    m_devices = devices;
    sortDevices( m_devices );
    endResetModel();
}

int
DeviceModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : m_devices.count();
}

QVariant
DeviceModel::data( const QModelIndex& index, int role ) const
{
    int row = index.row();
    if ( row < 0 || row >= m_devices.count() )
    {
        return QVariant();
    }

    Device* device = m_devices.at( row );

    switch ( role )
    {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        if ( device->name().isEmpty() )
        {
            return device->deviceNode();
        }
        else
        {
            if ( device->logicalSize() >= 0 && device->totalLogical() >= 0 )
            {
                //: device[name] - size[number] (device-node[name])
                return tr( "%1 - %2 (%3)" )
                    .arg( device->name() )
                    .arg( KFormat().formatByteSize( device->capacity() ) )
                    .arg( device->deviceNode() );
            }
            else
            {
                // Newly LVM VGs don't have capacity property yet (i.e.
                // always has 1B capacity), so don't show it for a while.
                //
                //: device[name] - (device-node[name])
                return tr( "%1 - (%2)" ).arg( device->name() ).arg( device->deviceNode() );
            }
        }
    case Qt::DecorationRole:
        return CalamaresUtils::defaultPixmap(
            CalamaresUtils::PartitionDisk,
            CalamaresUtils::Original,
            QSize( CalamaresUtils::defaultIconSize().width() * 3, CalamaresUtils::defaultIconSize().height() * 3 ) );
    default:
        return QVariant();
    }
}


Device*
DeviceModel::deviceForIndex( const QModelIndex& index ) const
{
    int row = index.row();
    if ( row < 0 || row >= m_devices.count() )
    {
        return nullptr;
    }
    return m_devices.at( row );
}


void
DeviceModel::swapDevice( Device* oldDevice, Device* newDevice )
{
    Q_ASSERT( oldDevice );
    Q_ASSERT( newDevice );

    int indexOfOldDevice = m_devices.indexOf( oldDevice );
    if ( indexOfOldDevice < 0 )
    {
        return;
    }

    m_devices[ indexOfOldDevice ] = newDevice;

    emit dataChanged( index( indexOfOldDevice ), index( indexOfOldDevice ) );
}

void
DeviceModel::addDevice( Device* device )
{
    beginResetModel();
    m_devices << device;
    sortDevices( m_devices );
    endResetModel();
}

void
DeviceModel::removeDevice( Device* device )
{
    beginResetModel();
    m_devices.removeAll( device );
    sortDevices( m_devices );
    endResetModel();
}
