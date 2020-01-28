/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2019-2020 Adriaan de Groot <groot@kde.org>
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

#ifndef LOCALE_QMLVIEWSTEP_H
#define LOCALE_QMLVIEWSTEP_H

#include "Config.h"
#include "geoip/Handler.h"
#include "geoip/Interface.h"
#include "utils/PluginFactory.h"
#include "utils/PluginFactory.h"
#include "viewpages/ViewStep.h"
#include <PluginDllMacro.h>

#include <QObject>

#include "PluginDllMacro.h"

#include <QFutureWatcher>
#include <QObject>

#include <memory>

namespace CalamaresUtils
{
    namespace GeoIP
    {
        class Handler;
    }
}  // namespace CalamaresUtils

class LocalePage;

class QQmlComponent;
class QQuickItem;
class QQuickWidget;


// TODO: Needs a generic Calamares::QmlViewStep as base class
// TODO: refactor and move what makes sense to base class
class PLUGINDLLEXPORT LocaleQmlViewStep : public Calamares::ViewStep
{
    Q_OBJECT
public:
    explicit LocaleQmlViewStep( QObject* parent = nullptr );

    QString prettyName() const override;

    QWidget* widget() override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override;

    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    Calamares::JobList jobs() const override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;
    Config* config() const;

private:
    // TODO: a generic QML viewstep should return a config object from a method
    Config *m_config;
//     GeneralRequirements* m_requirementsChecker;

    // TODO: these need to be in the base class (also a base class of ExecutionViewStep)
    QQuickWidget* m_qmlWidget;
    QQmlComponent* m_qmlComponent;
    QQuickItem* m_qmlItem;

    bool m_nextEnabled;
    QString m_prettyStatus;

    CalamaresUtils::GeoIP::RegionZonePair m_startingTimezone;
    QString m_localeGenPath;

    Calamares::JobList m_jobs;
    std::unique_ptr< CalamaresUtils::GeoIP::Handler > m_geoip;
};

CALAMARES_PLUGIN_FACTORY_DECLARATION( LocaleQmlViewStepFactory )

#endif
