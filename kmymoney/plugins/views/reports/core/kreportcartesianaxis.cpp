/*
 * Copyright 2020       Robert Szczesiak <dev.rszczesiak@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kreportcartesianaxis.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include <KChartCartesianCoordinatePlane>

// ----------------------------------------------------------------------------
// Project Includes


KReportCartesianAxis::KReportCartesianAxis (const QLocale& locale, int precision, KChart::AbstractCartesianDiagram* diagram )
    : CartesianAxis ( diagram )
    , m_locale(locale)
{
    if (precision > 10 || precision <= 0) // assure that conversion through QLocale::toString() will always work
        m_precision = 1;
    else
        m_precision = precision;
}

const QString KReportCartesianAxis::customizedLabel( const QString& label ) const
{
    bool ok;
    const auto separator = m_locale.groupSeparator();
    const auto decimalPoint = m_locale.decimalPoint();

    // convert label to double just to covert it back to string with desired precision
    // but without trailing zeros, separator, or decimal point
    const qreal labelValue = label.toDouble( &ok );
    if ( ok ) {
        return m_locale.toString( labelValue, 'f', m_precision )
                .remove( separator )
                .remove( QRegularExpression( "0+$" ) )
                .remove( QRegularExpression( "\\" + decimalPoint + "$" ) );
    } else
        return label;
}
