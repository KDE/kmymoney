/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <ace.j@hotpop.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kreportsview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QTreeWidget>
#include <QUrlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KRecentDirs>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_reportcontrol.h"

#include "icons.h"
#include "journalmodel.h"
#include "kbalancechartdlg.h"
#include "kmymoneysettings.h"
#include "kreportchartview.h"
#include "kreportconfigurationfilterdlg.h"
#include "menuenums.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "objectinfotable.h"
#include "pivottable.h"
#include "querytable.h"
#include "reportcontrolimpl.h"
#include "reporttable.h"
#include "tocitem.h"
#include "tocitemgroup.h"
#include "tocitemreport.h"

using namespace reports;
using namespace eMyMoney;
using namespace Icons;

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

/**
  * KReportsView Implementation
  */

KReportsView::KReportsView(QWidget *parent) :
    KMyMoneyViewBase(*new KReportsViewPrivate(this), parent)
{
    connect(pActions[eMenu::Action::ReportNew], &QAction::triggered, this, &KReportsView::slotDuplicate);
    connect(pActions[eMenu::Action::ReportConfigure], &QAction::triggered, this, &KReportsView::slotConfigure);
    connect(pActions[eMenu::Action::ReportExport], &QAction::triggered, this, &KReportsView::slotExportView);
    connect(pActions[eMenu::Action::ReportDelete], &QAction::triggered, this, &KReportsView::slotDelete);
    connect(pActions[eMenu::Action::ReportClose], &QAction::triggered, this, &KReportsView::slotCloseCurrent);

    pActions[eMenu::Action::ReportNew]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    pActions[eMenu::Action::ReportConfigure]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_P));
    pActions[eMenu::Action::ReportDelete]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
    pActions[eMenu::Action::ReportClose]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_W));
}

KReportsView::~KReportsView()
{
}

void KReportsView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_D(KReportsView);
    switch (action) {
    case eMenu::Action::ReportOpen:
        slotOpenReport(selections.firstSelection(SelectedObjects::Report));
        break;

    case eMenu::Action::Print:
        if (d->isActiveView()) {
            slotPrintView();
        }
        break;

    case eMenu::Action::PrintPreview:
        if (d->isActiveView()) {
            slotPrintPreviewView();
        }
        break;

    case eMenu::Action::ChartAccountBalance: {
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(selections.firstSelection(SelectedObjects::Account));
        if (!account.id().isEmpty()) {
            QPointer<KBalanceChartDlg> dlg = new KBalanceChartDlg(account, this);
            dlg->exec();
            delete dlg;
        }
    } break;

    case eMenu::Action::FileClose:
        slotCloseAll();
        break;

    case eMenu::Action::ReportAccountTransactions:
        d->showTransactionReport();
        break;

    default:
        break;
    }
}

void KReportsView::refresh()
{
    Q_D(KReportsView);
    if (isVisible()) {
        d->loadView();
        d->m_needsRefresh = false;
    } else {
        d->m_needsRefresh = true;
    }
}

void KReportsView::showEvent(QShowEvent * event)
{
    Q_D(KReportsView);
    if (d->m_needLoad) {
        d->init();
        connect(d->ui.m_closeButton, &QToolButton::clicked, this, [&]() {
            Q_D(KReportsView);
            d->ui.m_filterContainer->hide();
            d->ui.m_searchWidget->clear();
            d->ui.m_reportTabWidget->setFocus();
        });
        connect(pActions[eMenu::Action::ShowFilterWidget], &QAction::triggered, this, [&]() {
            if (isVisible()) {
                Q_D(KReportsView);
                d->ui.m_filterContainer->show();
                d->ui.m_searchWidget->setFocus();
            }
        });
        connect(d->ui.m_searchWidget, &QLineEdit::textChanged, this, [&](const QString& text) {
            Q_D(KReportsView);
            d->setFilter(text);
        });

        // remove close button from list tab
        QTabBar* bar = d->ui.m_reportTabWidget->findChild<QTabBar*>();
        if (bar) {
            QTabBar::ButtonPosition closeSide =
                (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, d->ui.m_reportTabWidget->widget(0));
            QWidget* w = bar->tabButton(0, closeSide);
            bar->setTabButton(0, closeSide, nullptr);
            w->deleteLater();
        }
    }
    if (d->m_needsRefresh) {
        refresh();
        d->setFilter(d->ui.m_searchWidget->text());
    }

    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget()))
        Q_EMIT reportSelected(tab->report());
    else
        Q_EMIT reportSelected(MyMoneyReport());

    // don't forget base class implementation
    QWidget::showEvent(event);
}

bool KReportsView::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(KReportsView);

    if (event->type() == QEvent::KeyPress) {
        if (watched == d->ui.m_searchWidget || watched == d->ui.m_tocTreeWidget) {
            const auto kev = static_cast<QKeyEvent*>(event);
            if (kev->modifiers() == Qt::NoModifier && kev->key() == Qt::Key_Escape) {
                d->ui.m_closeButton->animateClick();
                return true;
            }
        }

    } else if (event->type() == QEvent::ShortcutOverride) {
        const auto kev = static_cast<QKeyEvent*>(event);
        const auto keySeq = QKeySequence(kev->modifiers() | kev->key());
        const auto buttons = findChildren<QToolButton*>();
        for (const auto& button : qAsConst(buttons)) {
            const auto action = button->defaultAction();
            if (action) {
                const auto actionSeq = action->shortcut();
                if (keySeq == actionSeq) {
                    action->trigger();
                    event->accept();
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void KReportsView::updateActions(const SelectedObjects& selections)
{
    Q_D(KReportsView);
    bool enable = false;

    if (!selections.selection(SelectedObjects::Account).isEmpty()) {
        const auto file = MyMoneyFile::instance();
        const auto accId = selections.selection(SelectedObjects::Account).at(0);
        if (!file->isStandardAccount(accId)) {
            d->m_currentAccount = file->accountsModel()->itemById(accId);
            switch (d->m_currentAccount.accountType()) {
            case eMyMoney::Account::Type::Asset:
            case eMyMoney::Account::Type::Liability:
            case eMyMoney::Account::Type::Equity:
            case eMyMoney::Account::Type::Checkings:
                enable = true;
                break;
            default:
                break;
            }
        }
    }
    pActions[eMenu::Action::ReportAccountTransactions]->setEnabled(enable);

    // only access the widgets if they are initialized
    if (!d->m_needLoad) {
        if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget())) {
            tab->enableAllReportActions();
        }
    }
}

void KReportsView::slotOpenUrl(const QUrl &url)
{
    QString view = url.fileName();
    if (view.isEmpty())
        return;
    QString command = QUrlQuery(url).queryItemValue("command");
    QString id = QUrlQuery(url).queryItemValue("id");
    QString tid = QUrlQuery(url).queryItemValue("tid");

    if (view == VIEW_REPORTS) {

        if (command.isEmpty()) {
            // slotRefreshView();
        } else if (command == QLatin1String("print"))
            slotPrintView();
        else if (command == QLatin1String("save"))
            slotExportView();
        else if (command == QLatin1String("configure"))
            slotConfigure();
        else if (command == QLatin1String("duplicate"))
            slotDuplicate();
        else if (command == QLatin1String("close"))
            slotCloseCurrent();
        else if (command == QLatin1String("delete"))
            slotDelete();
        else
            qWarning() << i18n("Unknown command '%1' in KReportsView::slotOpenUrl()", qPrintable(command));

    } else if (view == VIEW_LEDGER) {
        const auto gotoAccount = pActions[eMenu::Action::GoToAccount];
        gotoAccount->setData(id);

        // convert transaction id to journalEntryId and make it the current selection
        SelectedObjects selection;
        const auto indexes = MyMoneyFile::instance()->journalModel()->indexesByTransactionId(tid);
        for (const auto& idx : indexes) {
            if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == id) {
                tid = idx.data(eMyMoney::Model::IdRole).toString();
                break;
            }
        }
        selection.setSelection(SelectedObjects::JournalEntry, tid);
        Q_EMIT requestSelectionChange(selection);

        gotoAccount->trigger();
    } else {
        qWarning() << i18n("Unknown view '%1' in KReportsView::slotOpenUrl()", qPrintable(view));
    }
}

void KReportsView::slotPrintView()
{
    Q_D(KReportsView);
    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget()))
        tab->print();
}

void KReportsView::slotPrintPreviewView()
{
    Q_D(KReportsView);
    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget()))
        tab->printPreview();
}

void KReportsView::slotExportView()
{
    Q_D(KReportsView);
    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget())) {
        QPointer<QFileDialog> dialog = new QFileDialog(this, i18n("Export as"), KRecentDirs::dir(":kmymoney-export"));
        dialog->setMimeTypeFilters({QStringLiteral("text/csv"), QStringLiteral("text/html"), QStringLiteral("application/xml")});
        dialog->setFileMode(QFileDialog::AnyFile);
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->selectFile(tab->report().name());

        QUrl newURL;
        QString selectedMimeType;
        if (dialog->exec() == QDialog::Accepted) {
            newURL = dialog->selectedUrls().first();
            selectedMimeType = dialog->selectedMimeTypeFilter();
        }
        delete dialog;

        if (!newURL.isEmpty()) {
            KRecentDirs::add(":kmymoney-export", newURL.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path());
            QString newName = newURL.toDisplayString(QUrl::PreferLocalFile);

            try {
                tab->saveAs(newName, selectedMimeType);
            } catch (const MyMoneyException &e) {
                KMessageBox::error(this, i18n("Failed to save: %1", QString::fromLatin1(e.what())));
            }
        }
    }
}

void KReportsView::slotConfigure()
{
    doConfigure(NoConfigureOption);
}

void KReportsView::doConfigure(ConfigureOption configureOption)
{
    Q_D(KReportsView);
    QString cm = "KReportsView::slotConfigure";

    auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget());

    if (!tab) // nothing to do
        return;
    int tabNr = d->ui.m_reportTabWidget->currentIndex();

    tab->updateDataRange(); // range will be needed during configuration, but cannot be obtained earlier

    MyMoneyReport report = tab->report();
    if (report.comment() == i18n("Default Report") || report.comment() == i18n("Generated Report")) {
        report.setComment(i18n("Custom Report"));
        report.setName(i18n("%1 (Customized)", report.name()));
    }

    QPointer<KReportConfigurationFilterDlg> dlg = new KReportConfigurationFilterDlg(report);

    if (dlg->exec()) {
        MyMoneyReport newreport = dlg->getConfig();

        // If this report has an ID, then MODIFY it, otherwise ADD it
        MyMoneyFileTransaction ft;
        try {
            if (! newreport.id().isEmpty()) {
                MyMoneyFile::instance()->modifyReport(newreport);
                ft.commit();
                tab->modifyReport(newreport);

                d->ui.m_reportTabWidget->setTabText(tabNr, newreport.name());
                d->ui.m_reportTabWidget->setCurrentIndex(tabNr);
            } else {
                MyMoneyFile::instance()->addReport(newreport);
                ft.commit();

                QString reportGroupName = newreport.group();

                // find report group
                TocItemGroup* tocItemGroup = d->m_allTocItemGroups[reportGroupName];
                if (!tocItemGroup) {
                    QString error = i18n("Could not find reportgroup \"%1\" for report \"%2\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", reportGroupName, newreport.name());

                    // write to messagehandler
                    qWarning() << cm << error;

                    // also inform user
                    KMessageBox::error(d->ui.m_reportTabWidget, error, i18n("Critical Error"));

                    // cleanup
                    delete dlg;

                    return;
                }

                // do not add TocItemReport to TocItemGroup here,
                // this is done in loadView

                d->addReportTab(newreport, OpenImmediately);
            }
        } catch (const MyMoneyException &e) {
            KMessageBox::error(this, i18n("Failed to configure report: %1", QString::fromLatin1(e.what())));
        }
    } else if (configureOption == LoadReportOnCancel) {
        tab->loadTab();
    }
    delete dlg;
}

void KReportsView::slotDuplicate()
{
    Q_D(KReportsView);
    QString cm = "KReportsView::slotDuplicate";

    auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget());

    if (!tab) {
        // nothing to do
        return;
    }

    MyMoneyReport dupe = tab->report();
    dupe.setName(i18n("Copy of %1", dupe.name()));
    if (dupe.comment() == i18n("Default Report"))
        dupe.setComment(i18n("Custom Report"));
    dupe.clearId();

    QPointer<KReportConfigurationFilterDlg> dlg = new KReportConfigurationFilterDlg(dupe);
    if (dlg->exec()) {
        MyMoneyReport newReport = dlg->getConfig();
        MyMoneyFileTransaction ft;
        try {
            MyMoneyFile::instance()->addReport(newReport);
            ft.commit();

            QString reportGroupName = newReport.group();

            // find report group
            TocItemGroup* tocItemGroup = d->m_allTocItemGroups[reportGroupName];
            if (!tocItemGroup) {
                QString error = i18n("Could not find reportgroup \"%1\" for report \"%2\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", reportGroupName, newReport.name());

                // write to messagehandler
                qWarning() << cm << error;

                // also inform user
                KMessageBox::error(d->ui.m_reportTabWidget, error, i18n("Critical Error"));

                // cleanup
                delete dlg;

                return;
            }

            // do not add TocItemReport to TocItemGroup here,
            // this is done in loadView

            d->addReportTab(newReport, OpenImmediately);
        } catch (const MyMoneyException &e) {
            QString error = i18n("Cannot add report, reason: \"%1\"", e.what());

            // write to messagehandler
            qWarning() << cm << error;

            // also inform user
            KMessageBox::error(d->ui.m_reportTabWidget, error, i18n("Critical Error"));
        }
    }
    delete dlg;
}

void KReportsView::slotDelete()
{
    Q_D(KReportsView);
    auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget());
    if (!tab) {
        // nothing to do
        return;
    }

    MyMoneyReport report = tab->report();
    if (! report.id().isEmpty()) {
        if (KMessageBox::Continue == d->deleteReportDialog(report.name())) {
            // close the tab and then remove the report so that it is not
            // generated again during the following loadView() call
            slotClose(d->ui.m_reportTabWidget->currentIndex());

            MyMoneyFileTransaction ft;
            MyMoneyFile::instance()->removeReport(report);
            ft.commit();
        }
    } else {
        KMessageBox::information(this,
                                 QString("<qt>") +
                                 i18n("<b>%1</b> is a default report, so it cannot be deleted.",
                                      report.name()) + QString("</qt>"), i18n("Delete Report?"));
    }
}


void KReportsView::slotOpenReport(const QString& id)
{
    Q_D(KReportsView);
    if (id.isEmpty()) {
        // nothing to do
        return;
    }

    KReportTab* page = 0;

    // Find the tab which contains the report
    int index = 1;
    while (index < d->ui.m_reportTabWidget->count()) {
        auto current = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(index));

        if (current && current->report().id() == id) {
            page = current;
            break;
        }

        ++index;
    }

    // Show the tab, or create a new one, as needed
    if (page)
        d->ui.m_reportTabWidget->setCurrentIndex(index);
    else
        d->addReportTab(MyMoneyFile::instance()->report(id), OpenImmediately);
}

void KReportsView::slotOpenReport(const MyMoneyReport& report)
{
    Q_D(KReportsView);
    if (d->m_needLoad)
        d->init();
    KReportTab* page = 0;

    // Find the tab which contains the report indicated by this list item
    int index = 1;
    while (index < d->ui.m_reportTabWidget->count()) {
        auto current = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(index));

        if (current && current->report().name() == report.name()) {
            page = current;
            break;
        }

        ++index;
    }

    // Show the tab, or create a new one, as needed
    if (page)
        d->ui.m_reportTabWidget->setCurrentIndex(index);
    else
        d->addReportTab(report, OpenImmediately);

    if (!isVisible())
        Q_EMIT switchViewRequested(View::Reports);
}

void KReportsView::slotItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    doItemDoubleClicked(item, column, OpenImmediately);
}

void KReportsView::doItemDoubleClicked(QTreeWidgetItem* item, int column, OpenOption openOption)
{
    Q_UNUSED(column)

    Q_D(KReportsView);
    auto tocItem = dynamic_cast<TocItem*>(item);
    if (tocItem && !tocItem->isReport()) {
        // toggle the expanded-state for reportgroup-items
        item->setExpanded(item->isExpanded() ? false : true);

        // nothing else to do for reportgroup-items
        return;
    }

    TocItemReport* reportTocItem = dynamic_cast<TocItemReport*>(tocItem);

    MyMoneyReport& report = reportTocItem->getReport();

    KReportTab* page = 0;

    // Find the tab which contains the report indicated by this list item
    int index = 1;
    while (index < d->ui.m_reportTabWidget->count()) {
        auto current = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(index));
        if (current) {
            // If this report has an ID, we'll use the ID to match
            if (! report.id().isEmpty()) {
                if (current->report().id() == report.id()) {
                    page = current;
                    break;
                }
            }
            // Otherwise, use the name to match.  THIS ASSUMES that no 2 default reports
            // have the same name...but that would be pretty a boneheaded thing to do.
            else {
                if (current->report().name() == report.name()) {
                    page = current;
                    break;
                }
            }
        }

        ++index;
    }

    // Show the tab, or create a new one, as needed
    if (page)
        d->ui.m_reportTabWidget->setCurrentIndex(index);
    else
        d->addReportTab(report, openOption);
}

void KReportsView::slotToggleChart()
{
    Q_D(KReportsView);
    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->currentWidget()))
        tab->toggleChart();
}

bool KReportsView::hasClosableView() const
{
    Q_D(const KReportsView);
    return d->ui.m_reportTabWidget->count() > 1;
}

void KReportsView::closeCurrentView()
{
    Q_D(KReportsView);
    const auto idx = d->ui.m_reportTabWidget->currentIndex();
    if (idx > 0) {
        slotClose(idx);
    }
}

void KReportsView::slotCloseCurrent()
{
    Q_D(KReportsView);
    slotClose(d->ui.m_reportTabWidget->currentIndex());
}

void KReportsView::slotClose(int index)
{
    Q_D(KReportsView);
    if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(index))) {
        d->ui.m_reportTabWidget->removeTab(index);
        tab->setReadyToDelete(true);
    }
}

void KReportsView::slotCloseAll()
{
    Q_D(KReportsView);
    if(!d->m_needLoad) {
        while (true) {
            if (auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(1))) {
                d->ui.m_reportTabWidget->removeTab(1);
                tab->setReadyToDelete(true);
            } else
                break;
        }
    }
}


void KReportsView::slotListContextMenu(const QPoint & p)
{
    Q_D(KReportsView);
    const auto items = d->ui.m_tocTreeWidget->selectedItems();

    if (items.isEmpty()) {
        return;
    }

    QList<TocItem*> tocItems;
    for (const auto& item : items) {
        auto tocItem = dynamic_cast<TocItem*>(item);
        if (tocItem && tocItem->isReport()) {
            tocItems.append(tocItem);
        }
    }

    if (tocItems.isEmpty()) {
        return;
    }

    auto contextmenu = new QMenu(this);

    contextmenu->addSection(i18nc("@title:menu Report context menu", "Report options"));
    contextmenu->addAction(i18nc("To open a new report", "&Open"),
                           this, SLOT(slotOpenFromList()));

    contextmenu->addAction(i18nc("To print a report", "&Print"),
                           this, SLOT(slotPrintFromList()));

    if (tocItems.count() == 1) {
        contextmenu->addAction(i18nc("Configure a report", "&Configure"),
                               this, SLOT(slotConfigureFromList()));

        contextmenu->addAction(i18n("&New report"),
                               this, SLOT(slotNewFromList()));

        // Only add this option if it's a custom report. Default reports cannot be deleted

        auto reportTocItem = dynamic_cast<TocItemReport*>(tocItems.at(0));

        if (reportTocItem) {
            MyMoneyReport& report = reportTocItem->getReport();
            if (! report.id().isEmpty()) {
                contextmenu->addAction(i18n("&Delete"),
                                       this, SLOT(slotDeleteFromList()));
            }
        }
    }

    contextmenu->popup(d->ui.m_tocTreeWidget->viewport()->mapToGlobal(p));
}

void KReportsView::slotOpenFromList()
{
    Q_D(KReportsView);

    const auto items = d->ui.m_tocTreeWidget->selectedItems();

    if (items.isEmpty()) {
        return;
    }

    for (const auto& item : items) {
        auto tocItem = dynamic_cast<TocItem*>(item);
        if (tocItem && tocItem->isReport()) {
            slotItemDoubleClicked(tocItem, 0);
        }
    }
}

void KReportsView::slotPrintFromList()
{
    Q_D(KReportsView);

    const auto items = d->ui.m_tocTreeWidget->selectedItems();

    if (items.isEmpty()) {
        return;
    }

    for (const auto& item : items) {
        auto tocItem = dynamic_cast<TocItem*>(item);
        if (tocItem && tocItem->isReport()) {
            slotItemDoubleClicked(tocItem, 0);
            slotPrintView();
        }
    }
}

void KReportsView::slotConfigureFromList()
{
    Q_D(KReportsView);
    if (auto tocItem = dynamic_cast<TocItem*>(d->ui.m_tocTreeWidget->currentItem())) {
        doItemDoubleClicked(tocItem, 0, OpenAfterConfiguration);
        doConfigure(LoadReportOnCancel);
    }
}

void KReportsView::slotNewFromList()
{
    Q_D(KReportsView);
    if (auto tocItem = dynamic_cast<TocItem*>(d->ui.m_tocTreeWidget->currentItem())) {
        slotItemDoubleClicked(tocItem, 0);
        slotDuplicate();
    }
}

void KReportsView::slotDeleteFromList()
{
    Q_D(KReportsView);
    if (auto tocItem = dynamic_cast<TocItem*>(d->ui.m_tocTreeWidget->currentItem())) {
        if (auto reportTocItem = dynamic_cast<TocItemReport*>(tocItem)) {
            MyMoneyReport& report = reportTocItem->getReport();

            // If this report does not have an ID, it's a default report and cannot be deleted
            if (! report.id().isEmpty() &&
                    KMessageBox::Continue == d->deleteReportDialog(report.name())) {
                // check if report's tab is open; start from 1 because 0 is toc tab
                for (int i = 1; i < d->ui.m_reportTabWidget->count(); ++i) {
                    auto tab = dynamic_cast<KReportTab*>(d->ui.m_reportTabWidget->widget(i));
                    if (tab && tab->report().id() == report.id()) {
                        slotClose(i); // if open, close it, so no crash when switching to it
                        break;
                    }
                }
                MyMoneyFileTransaction ft;
                MyMoneyFile::instance()->removeReport(report);
                ft.commit();
            }
        }
    }
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS
