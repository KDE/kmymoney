/*
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    // convert label to double just to convert it back to string with desired precision
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
