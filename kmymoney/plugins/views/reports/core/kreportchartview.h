/*
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KREPORTCHARTVIEW_H
#define KREPORTCHARTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KChartChart>

// ----------------------------------------------------------------------------
// Project Includes

#include "pivotgrid.h"
#include "mymoneyreport.h"

using namespace KChart;

namespace reports
{


class KReportChartView: public Chart
{
    Q_OBJECT
public:
    explicit KReportChartView(QWidget* parent);
    ~KReportChartView() {}

    /**
      * Returns the labels for the X axis
      * @see m_abscissaNames
      */
    QStringList& abscissaNames() {
        return m_abscissaNames;
    }

    /**
      * Draw the chart for a pivot table report
      */
    void drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numberColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList);

    /**
      * Draw a limit chart
      * @param limit is either a maximum credit or minimum balance for an account
      */
    void drawLimitLine(const double limit);

    /**
      * Remove the chart legend
      */
    void removeLegend();

private:

    /**
     * Adjust vertical range if data model is represented by a horizontal line
     * or the range starts below precision limit when logarithmic axis is used
     */
    void adjustVerticalRange(const int precision = 2);

    /**
      * Draw a PivotGridRow in a chart
      */
    int drawPivotGridRow(int rowNum, const PivotGridRow& gridRow, const QString& legendText, const int startColumn = 0, const int columnsToDraw = 0, const int precision = 2, const bool invertValue = false);

    /**
      * Set cell data
      */
    void setDataCell(int row, int column, const double value, QString tip = QString());

    /**
      * Make sure the model has the right size
      */
    void justifyModelSize(int rows, int columns);

    /**
      * Adjust line width of all datasets
      */
    void setLineWidth(const int lineWidth);

    /**
      * Set the accountSeries
      * @see m_accountSeries
      */
    void setAccountSeries(bool accountSeries) {
        m_accountSeries = accountSeries;
    }

    /**
      * Returns accountSeries
      * @see m_accountSeries
      */
    bool accountSeries() {
        return m_accountSeries;
    }

    /**
      * Set the seriesTotals
      * @see m_seriesTotals
      */
    void setSeriesTotals(bool seriesTotals) {
        m_seriesTotals = seriesTotals;
    }

    /**
      * Returns accountSeries
      * @see m_seriesTotals
      */
    bool seriesTotals() {
        return m_seriesTotals;
    }

    /**
      * Set the number of columns
      * @see m_numColumns
      */
    void setNumColumns(int numColumns) {
        m_numColumns = numColumns;
    }

    /**
      * Returns number of columns
      * @see m_numColumns
      */
    int numColumns() {
        return m_numColumns;
    }

    /**
      * The labels of the X axis
      */
    QStringList m_abscissaNames;

    /**
      * whether series (rows) are accounts (true) or months (false). This causes a lot
      * of complexity in the charts.  The problem is that circular reports work best with
      * an account in a COLUMN, while line/bar prefer it in a ROW.
      */
    bool m_accountSeries;

    /**
      * whether to limit the chart to use series totals only.  Used for reports which only
      * show one dimension (pie)
      */
    bool m_seriesTotals;

    /**
      * Number of columns on the report
      */
    int m_numColumns;

    /**
      * Model to store chart data
      */
    QStandardItemModel m_model;

    /**
      * whether to skip values if zero
      */
    bool m_skipZero;

    /**
      * The cached background brush obtained from the style.
      */
    QBrush m_backgroundBrush;

    /**
      * The cached foreground brush obtained from the style.
      */
    QBrush m_foregroundBrush;

    /**
      * The cached precision obtained from report's data
      */
    int m_precision;
};

} // end namespace reports

#endif // KREPORTCHARTVIEW_H
