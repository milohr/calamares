/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2019-2020, Adriaan de Groot <groot@kde.org>
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

#include <QDebug>

void
RequirementsModel::setRequirementsList( const Calamares::RequirementsList& requirements )
{
    emit beginResetModel();
    m_requierements = requirements;
    m_satisfiedRequirements = true;

    for(const auto requirement : m_requierements)
    {
        if(requirement.mandatory && !requirement.satisfied)
        {
//             m_satisfiedRequirements = false;
            break;
        }
    }

    emit satisfiedRequirementsChanged(m_satisfiedRequirements);
    emit endResetModel();
}

int
RequirementsModel::rowCount( const QModelIndex& ) const
{
    return m_requierements.count();
}

QVariant
RequirementsModel::data( const QModelIndex& index, int role ) const
{
    const auto requirement = m_requierements.at( index.row() );

	switch ( role )
	{
		case Roles::Name:
            return requirement.name;
        case Roles::Details:
            return requirement.enumerationText();
        case Roles::NegatedText:
            return requirement.negatedText();
        case Roles::Satisfied:
            return requirement.satisfied;
        case Roles::Mandatory:
            return requirement.mandatory;
        default:
			return QVariant();
	}
}

QHash<int, QByteArray>
RequirementsModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
	roles[Roles::Name] = "name";
	roles[Roles::Details] = "details";
	roles[Roles::NegatedText] = "negatedText";
	roles[Roles::Satisfied] = "satisfied";
	roles[Roles::Mandatory] = "mandatory";
	return roles;
}

Config::Config( QObject* parent ) : QObject( parent )
    ,m_requirementsModel( new RequirementsModel( this ))
{
    connect(m_requirementsModel, &RequirementsModel::satisfiedRequirementsChanged, this, &Config::setIsNextEnabled);
}

CalamaresUtils::Locale::LabelModel*
Config::languagesModel() const
{
	return CalamaresUtils::Locale::availableTranslations();
}

QString
Config::languageIcon() const
{
	return m_languageIcon;
}

void
Config::setCountryCode( const QString& countryCode )
{
	m_countryCode = countryCode;
	m_localeIndex = CalamaresUtils::Locale::availableTranslations()->find( m_countryCode );

	emit countryCodeChanged( m_countryCode );
	emit localeIndexChanged( m_localeIndex );
}

void
Config::setLanguageIcon( const QString languageIcon )
{
	m_languageIcon = languageIcon;
}

RequirementsModel&
Config::requirementsModel() const
{
    return *m_requirementsModel;
}

void
Config::setIsBackEnabled( const bool& isBackEnabled )
{
    m_isBackEnabled = isBackEnabled;
    emit isBackEnabledChanged( m_isBackEnabled );
}

void
Config::setIsNextEnabled( const bool& isNextEnabled )
{
    m_isNextEnabled = isNextEnabled;
    emit isNextEnabledChanged( m_isNextEnabled );
}






