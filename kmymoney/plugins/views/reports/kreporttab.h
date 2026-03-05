/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018-2025 Thomas Baumgart <thb@net-bembel.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-FileCopyrightText: 2021-2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2021-2024 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "kreportsview.h"
#include "mymoneyreport.h"
#include "reporttable.h"

#include <QPointer>
#include <QWidget>

class KMMTextBrowser;
namespace reports {
class KReportChartView;
}
class ReportControl;
class QTabWidget;
class QVBoxLayout;

/**
 * Helper class for KReportView.
 *
 * This is the widget which displays a single report in the TabWidget that comprises this view.
 *
 * @author Ace Jones
 */

class KReportTab : public QWidget
{
private:
    KMMTextBrowser* m_tableView;
    reports::KReportChartView* m_chartView;
    ReportControl* m_control;
    QVBoxLayout* m_layout;
    MyMoneyReport m_report;
    bool m_deleteMe;
    bool m_chartEnabled;
    bool m_showingChart;
    bool m_needReload;
    bool m_isChartViewValid;
    bool m_isTableViewValid;
    QPointer<reports::ReportTable> m_table;

    /**
     * Users character set encoding.
     */
    QByteArray m_encoding;

public:
    KReportTab(QTabWidget* parent, const MyMoneyReport& report, const KReportsView* eventHandler, KReportsView::OpenOption openOption);
    ~KReportTab();
    const MyMoneyReport& report() const
    {
        return m_report;
    }
    void print();
    void printPreview();
    void toggleChart();
    /**
     * Updates information about plotted chart in report's data
     */
    void updateDataRange();
    void copyToClipboard();
    void saveAs(const QString& filename, const QString& selectedMimeType);
    void updateReport();
    QString createTable(const QString& links = QString());
    const ReportControl* control() const
    {
        return m_control;
    }

    bool isReadyToDelete() const
    {
        return m_deleteMe;
    }

    void setReadyToDelete(bool f)
    {
        m_deleteMe = f;
    }

    void modifyReport(const MyMoneyReport& report)
    {
        m_report = report;
        updateReport();
    }

    void enableAllReportActions();

    void showEvent(QShowEvent* event) final override;
    void loadTab();

protected:
    void wheelEvent(QWheelEvent* event) override;
};
