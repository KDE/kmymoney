/*
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
