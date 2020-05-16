/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2014-2015, Teo Mrnjavac <teo@kde.org>
 *   Copyright 2018,2020 Adriaan de Groot <groot@kde.org>
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

#include "WelcomeQmlViewStep.h"

#include "checker/GeneralRequirements.h"

#include "locale/LabelModel.h"
#include "utils/Dirs.h"
#include "utils/Logger.h"
#include "utils/Variant.h"

#include "Branding.h"
#include "modulesystem/ModuleManager.h"
#include "utils/Yaml.h"

CALAMARES_PLUGIN_FACTORY_DEFINITION( WelcomeQmlViewStepFactory, registerPlugin< WelcomeQmlViewStep >(); )

WelcomeQmlViewStep::WelcomeQmlViewStep( QObject* parent )
    : Calamares::QmlViewStep( parent )
    , m_config( new Config( this ) )
    , m_requirementsChecker( new GeneralRequirements( this ) )
{
    connect( Calamares::ModuleManager::instance(),
            &Calamares::ModuleManager::requirementsComplete,
            this,
            &WelcomeQmlViewStep::nextStatusChanged );
}


QString
WelcomeQmlViewStep::prettyName() const
{
    return tr( "Welcome" );
}

bool
WelcomeQmlViewStep::isNextEnabled() const
{
    return m_config->requirementsModel()->satisfiedMandatory();
}

bool
WelcomeQmlViewStep::isBackEnabled() const
{
    // TODO: should return true (it's weird that you are not allowed to have welcome *after* anything
    return false;
}


bool
WelcomeQmlViewStep::isAtBeginning() const
{
    // TODO: adjust to "pages" in the QML
    return true;
}


bool
WelcomeQmlViewStep::isAtEnd() const
{
    // TODO: adjust to "pages" in the QML
    return true;
}


Calamares::JobList
WelcomeQmlViewStep::jobs() const
{
    return Calamares::JobList();
}

void
WelcomeQmlViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_config->setConfigurationMap( configurationMap );

    if ( configurationMap.contains( "requirements" )
         && configurationMap.value( "requirements" ).type() == QVariant::Map )
    {
        m_requirementsChecker->setConfigurationMap( configurationMap.value( "requirements" ).toMap() );
    }
    else
    {
        cWarning() << "no valid requirements map found in welcomeq "
                      "module configuration.";
    }

    return QString();
}

void
WelcomeQmlViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
	using Calamares::Branding;
    m_config->setSupportUrl( jobOrBrandingSetting( Branding::SupportUrl, configurationMap, "showSupportUrl" ) );
    m_config->setKnownIssuesUrl( jobOrBrandingSetting( Branding::KnownIssuesUrl, configurationMap, "showKnownIssuesUrl" ) );
    m_config->setReleaseNotesUrl( jobOrBrandingSetting( Branding::ReleaseNotesUrl, configurationMap, "showReleaseNotesUrl" ) );
    m_config->setDonateUrl( CalamaresUtils::getString( configurationMap, "showDonateUrl" ) );

	// TODO: expand Config class and set the remaining fields // with the configurationMap all those properties can be accessed without having to declare a property, get and setter for each

	// TODO: figure out how the requirements (held by ModuleManager) should be accessible
	//          to QML as a model. //will be model as a qvariantmap containing a alert level and the message string
	if ( configurationMap.contains( "requirements" )
		 && configurationMap.value( "requirements" ).type() == QVariant::Map )
	{
		m_requirementsChecker->setConfigurationMap( configurationMap.value( "requirements" ).toMap() );

        m_config->requirementsModel().setRequirementsList( checkRequirements() );
	}
	else
		cWarning() << "no valid requirements map found in welcome "
					  "module configuration.";

	bool ok = false;
	QVariantMap geoip = CalamaresUtils::getSubMap( configurationMap, "geoip", ok );
	if ( ok )
	{
		using FWString = QFutureWatcher< QString >;

		auto* handler = new CalamaresUtils::GeoIP::Handler( CalamaresUtils::getString( geoip, "style" ),
															CalamaresUtils::getString( geoip, "url" ),
															CalamaresUtils::getString( geoip, "selector" ) );
		if ( handler->type() != CalamaresUtils::GeoIP::Handler::Type::None )
		{
			auto* future = new FWString();
			connect( future, &FWString::finished, [view = this, f = future, h = handler]() {
				QString countryResult = f->future().result();
				cDebug() << "GeoIP result for welcome=" << countryResult;
				view->setCountry( countryResult, h );
				f->deleteLater();
				delete h;
			} );
			future->setFuture( handler->queryRaw() );
		}
		else
		{
			// Would not produce useful country code anyway.
			delete handler;
		}
	}

	QString language = CalamaresUtils::getString( configurationMap, "languageIcon" );
	if ( !language.isEmpty() )
	{
		auto icon = Calamares::Branding::instance()->image( language, QSize( 48, 48 ) );
		if ( !icon.isNull() )
		{
			m_config->setLanguageIcon(language);
		}
	}

    Calamares::QmlViewStep::setConfigurationMap( configurationMap ); // call parent implementation last
}

Calamares::RequirementsList
WelcomeQmlViewStep::checkRequirements()
{
    return m_requirementsChecker->checkRequirements();
}

QObject*
WelcomeQmlViewStep::getConfig()
{
    return m_config;
}
