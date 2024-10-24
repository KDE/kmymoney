/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <ace.jones@hotpop.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KREPORTSVIEW_H
#define KREPORTSVIEW_H

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

#ifdef _CHECK_MEMORY
#include "mymoneyutils.h"
#endif

#include "mymoneyreport.h"

namespace reports {
class KReportChartView;
}
namespace reports {
class ReportTable;
}

class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;
class TocItemGroup;
class ReportControl;
class ReportGroup;
class SelectedObjects;

class KMMTextBrowser;

/**
  * Displays a page where reports can be placed.
  *
  * @author Ace Jones
  *
  * @short A view for reports.
**/
class KReportsViewPrivate;
class KReportsView : public KMyMoneyViewBase
{
    Q_OBJECT

    typedef enum {
        NoConfigureOption,
        LoadReportOnCancel,
    } ConfigureOption;

public:
    typedef enum {
        OpenImmediately,
        OpenAfterConfiguration,
    } OpenOption;

    /**
      * Standard constructor.
      *
      * @param parent The QWidget this is used in.
      * @param name The QT name.
      *
      * @return An object of type KReportsView
      *
      * @see ~KReportsView
      */
    explicit KReportsView(QWidget *parent = nullptr);
    ~KReportsView() override;

    void executeAction(eMenu::Action action, const SelectedObjects& selections) override;

    void refresh();

    bool hasClosableView() const override;
    void closeCurrentView() override;

Q_SIGNALS:
    /**
      * This signal is emitted whenever a report is selected
      */
    void reportSelected(const MyMoneyReport&);

    /**
      * This signal is emitted whenever a transaction is selected
      */
    void transactionSelected(const QString&, const QString&);

    void switchViewRequested(View view);

protected:
    /**
      * Overridden so we can reload the view if necessary.
      *
      * @return Nothing.
      */
    void showEvent(QShowEvent * event) override;

    /**
     * Overridden so we can handle closing the search filter on ESC
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

public Q_SLOTS:
    void slotOpenUrl(const QUrl &url);

    void slotPrintView();
    void slotPrintPreviewView();
    void slotExportView();
    void slotConfigure();
    void slotDuplicate();
    void slotToggleChart();
    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
    void slotOpenReport(const QString&);
    void slotOpenReport(const MyMoneyReport&);
    void slotCloseCurrent();
    void slotClose(int index);
    void slotCloseAll();
    void slotDelete();
    void slotListContextMenu(const QPoint &);
    void slotOpenFromList();
    void slotPrintFromList();
    void slotExportFromList();
    void slotConfigureFromList();
    void slotNewFromList();
    void slotDeleteFromList();

    void updateActions(const SelectedObjects& selections) override;

private:
    Q_DECLARE_PRIVATE(KReportsView)

    /**
     * internal handling of slotItemDoubleClicked
     */
    void doItemDoubleClicked(QTreeWidgetItem* item, int column, OpenOption openOption);

    void doConfigure(ConfigureOption configureOption);
};

#endif
