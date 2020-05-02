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

#ifndef KREPORTCARTESIANAXIS_H
#define KREPORTCARTESIANAXIS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include <KChartCartesianAxis>

// ----------------------------------------------------------------------------
// Project Includes


class KReportCartesianAxis: public KChart::CartesianAxis
{
    Q_OBJECT

    Q_DISABLE_COPY( KReportCartesianAxis )

public:
    explicit KReportCartesianAxis( const QLocale& locale, int precision, KChart::AbstractCartesianDiagram* diagram = nullptr );

    const QString customizedLabel( const QString& label ) const Q_DECL_OVERRIDE;

private:
    QLocale m_locale;
    int m_precision;
};

#endif // KREPORTCARTESIANAXIS_H
