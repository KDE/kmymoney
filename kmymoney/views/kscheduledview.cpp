/***************************************************************************
                          kscheduledview.cpp  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kscheduledview.h"

// ----------------------------------------------------------------------------
// QT Includes


#include <QComboBox>
#include <QLayout>
#include <QList>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyscheduleddatetbl.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"
#include "kscheduletreeitem.h"
#include "ktreewidgetfilterlinewidget.h"

#include "kmymoney.h"

KScheduledView::KScheduledView(QWidget *parent) :
    QWidget(parent),
    m_openBills(true),
    m_openDeposits(true),
    m_openTransfers(true),
    m_openLoans(true)
{
  setupUi(this);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KTreeWidgetFilterLineWidget(m_listTab, m_scheduleTree);
  m_filterBox->insertWidget(1, m_searchWidget);

  //enable custom context menu
  m_scheduleTree->setContextMenuPolicy(Qt::CustomContextMenu);
  m_scheduleTree->setSelectionMode(QAbstractItemView::SingleSelection);

  readConfig();

  connect(m_qbuttonNew, SIGNAL(clicked()), kmymoney->action("schedule_new"), SLOT(trigger()));

  // attach popup to 'Filter...' button
  m_kaccPopup = new KMenu(this);
  m_accountsCombo->setMenu(m_kaccPopup);
  connect(m_kaccPopup, SIGNAL(triggered(QAction*)), this, SLOT(slotAccountActivated()));

  m_qbuttonNew->setGuiItem(KMyMoneyUtils::scheduleNewGuiItem());
  m_accountsCombo->setGuiItem(KMyMoneyUtils::accountsFilterGuiItem());

  m_tabWidget->setTabIcon(m_tabWidget->indexOf(m_listTab), KIcon("view-calendar-list"));
  m_tabWidget->setTabIcon(m_tabWidget->indexOf(m_calendarTab), KIcon("view-calendar-timeline"));

  connect(m_scheduleTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotListViewContextMenu(QPoint)));
  connect(m_scheduleTree, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSetSelectedItem()));

  connect(m_scheduleTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
          this, SLOT(slotListItemExecuted(QTreeWidgetItem*,int)));
  connect(m_scheduleTree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          this, SLOT(slotListViewExpanded(QTreeWidgetItem*)));
  connect(m_scheduleTree, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
          this, SLOT(slotListViewCollapsed(QTreeWidgetItem*)));

  connect(m_calendar, SIGNAL(enterClicked(MyMoneySchedule,QDate)), this, SLOT(slotBriefEnterClicked(MyMoneySchedule,QDate)));
  connect(m_calendar, SIGNAL(skipClicked(MyMoneySchedule,QDate)), this, SLOT(slotBriefSkipClicked(MyMoneySchedule,QDate)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadView()));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

static bool accountNameLessThan(const MyMoneyAccount& acc1, const MyMoneyAccount& acc2)
{
  return acc1.name().toLower() < acc2.name().toLower();
}

void KScheduledView::refresh(bool full, const QString& schedId)
{
  m_scheduleTree->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  m_scheduleTree->clear();

  try {
    if (full) {
      try {
        m_kaccPopup->clear();

        MyMoneyFile* file = MyMoneyFile::instance();

        // extract a list of all accounts under the asset group
        // and sort them by name
        QList<MyMoneyAccount> list;
        QStringList accountList = file->asset().accountList();
        accountList.append(file->liability().accountList());
        file->accountList(list, accountList, true);
        qStableSort(list.begin(), list.end(), accountNameLessThan);

        QList<MyMoneyAccount>::ConstIterator it_a;
        for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
          if (!(*it_a).isClosed()) {
            QAction* act;
            act = m_kaccPopup->addAction((*it_a).name());
            act->setCheckable(true);
            act->setChecked(true);
          }
        }

      } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Unable to load accounts: "), e.what());
      }
    }

    // Refresh the calendar view first
    m_calendar->refresh();

    MyMoneyFile *file = MyMoneyFile::instance();
    QList<MyMoneySchedule> scheduledItems = file->scheduleList();

    if (scheduledItems.count() == 0)
      return;

    //disable sorting for performance
    m_scheduleTree->setSortingEnabled(false);

    KScheduleTreeItem *itemBills = new KScheduleTreeItem(m_scheduleTree);
    itemBills->setIcon(0, KIcon("view-expenses-categories"));
    itemBills->setText(0, i18n("Bills"));
    itemBills->setData(0, KScheduleTreeItem::OrderRole, QVariant("0"));
    itemBills->setFirstColumnSpanned(true);
    itemBills->setFlags(Qt::ItemIsEnabled);
    QFont bold = itemBills->font(0);
    bold.setBold(true);
    itemBills->setFont(0, bold);
    KScheduleTreeItem *itemDeposits = new KScheduleTreeItem(m_scheduleTree);
    itemDeposits->setIcon(0, KIcon("view-income-categories"));
    itemDeposits->setText(0, i18n("Deposits"));
    itemDeposits->setData(0, KScheduleTreeItem::OrderRole, QVariant("1"));
    itemDeposits->setFirstColumnSpanned(true);
    itemDeposits->setFlags(Qt::ItemIsEnabled);
    itemDeposits->setFont(0, bold);
    KScheduleTreeItem *itemLoans = new KScheduleTreeItem(m_scheduleTree);
    itemLoans->setIcon(0, KIcon("view-loan"));
    itemLoans->setText(0, i18n("Loans"));
    itemLoans->setData(0, KScheduleTreeItem::OrderRole, QVariant("2"));
    itemLoans->setFirstColumnSpanned(true);
    itemLoans->setFlags(Qt::ItemIsEnabled);
    itemLoans->setFont(0, bold);
    KScheduleTreeItem *itemTransfers = new KScheduleTreeItem(m_scheduleTree);
    itemTransfers->setIcon(0, KIcon("view-financial-transfer"));
    itemTransfers->setText(0, i18n("Transfers"));
    itemTransfers->setData(0, KScheduleTreeItem::OrderRole, QVariant("3"));
    itemTransfers->setFirstColumnSpanned(true);
    itemTransfers->setFlags(Qt::ItemIsEnabled);
    itemTransfers->setFont(0, bold);

    QList<MyMoneySchedule>::Iterator it;

    QTreeWidgetItem *openItem = 0;

    for (it = scheduledItems.begin(); it != scheduledItems.end(); ++it) {
      MyMoneySchedule schedData = (*it);
      QTreeWidgetItem* item = 0;

      bool bContinue = true;
      QStringList::iterator accIt;
      for (accIt = m_filterAccounts.begin(); accIt != m_filterAccounts.end(); ++accIt) {
        if (*accIt == schedData.account().id()) {
          bContinue = false; // Filter it out
          break;
        }
      }

      if (!bContinue)
        continue;

      QTreeWidgetItem* parent = 0;
      switch (schedData.type()) {
        case MyMoneySchedule::TYPE_ANY:
          // Should we display an error ?
          // We just sort it as bill and fall through here

        case MyMoneySchedule::TYPE_BILL:
          parent = itemBills;
          break;

        case MyMoneySchedule::TYPE_DEPOSIT:
          parent = itemDeposits;
          break;

        case MyMoneySchedule::TYPE_TRANSFER:
          parent = itemTransfers;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          parent = itemLoans;
          break;

      }
      if (parent) {
        if (!KMyMoneyGlobalSettings::hideFinishedSchedules() || !schedData.isFinished()) {
          item = addScheduleItem(parent, schedData);
          if (schedData.id() == schedId)
            openItem = item;
        }
      }
    }

    if (openItem) {
      m_scheduleTree->setCurrentItem(openItem);
    }
    // using a timeout is the only way, I got the 'ensureTransactionVisible'
    // working when coming from hidden form to visible form. I assume, this
    // has something to do with the delayed update of the display somehow.
    resize(width(), height() - 1);
    QTimer::singleShot(10, this, SLOT(slotTimerDone()));
    m_scheduleTree->update();

    // force repaint in case the filter is set
    m_searchWidget->searchLine()->updateSearch(QString());

    if (m_openBills)
      itemBills->setExpanded(true);

    if (m_openDeposits)
      itemDeposits->setExpanded(true);

    if (m_openTransfers)
      itemTransfers->setExpanded(true);

    if (m_openLoans)
      itemLoans->setExpanded(true);

  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, e.what());
  }

  for (int i = 0; i < m_scheduleTree->columnCount(); ++i) {
    m_scheduleTree->resizeColumnToContents(i);
  }

  //reenable sorting after loading items
  m_scheduleTree->setSortingEnabled(true);
}

QTreeWidgetItem* KScheduledView::addScheduleItem(QTreeWidgetItem* parent, MyMoneySchedule& schedule)
{
  KScheduleTreeItem* item = new KScheduleTreeItem(parent);
  item->setData(0, Qt::UserRole, QVariant::fromValue(schedule));
  item->setData(0, KScheduleTreeItem::OrderRole, schedule.name());
  if (!schedule.isFinished()) {
    if (schedule.isOverdue()) {
      item->setIcon(0, KIcon("view-calendar-upcoming-events"));
      QBrush brush = item->foreground(0);
      brush.setColor(Qt::red);
      for (int i = 0; i < m_scheduleTree->columnCount(); ++i) {
        item->setForeground(i, brush);
      }
    } else {
      item->setIcon(0, KIcon("view-calendar-day"));
    }
  } else {
    item->setIcon(0, KIcon("dialog-close"));
    QBrush brush = item->foreground(0);
    brush.setColor(Qt::darkGreen);
    for (int i = 0; i < m_scheduleTree->columnCount(); ++i) {
      item->setForeground(i, brush);
    }
  }

  try {
    MyMoneyTransaction transaction = schedule.transaction();
    MyMoneySplit s1 = (transaction.splits().size() < 1) ? MyMoneySplit() : transaction.splits()[0];
    MyMoneySplit s2 = (transaction.splits().size() < 2) ? MyMoneySplit() : transaction.splits()[1];
    QList<MyMoneySplit>::ConstIterator it_s;
    MyMoneySplit split;
    MyMoneyAccount acc;

    switch (schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        if (s1.value().isNegative())
          split = s2;
        else
          split = s1;
        break;

      case MyMoneySchedule::TYPE_LOANPAYMENT:
        for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
          acc = MyMoneyFile::instance()->account((*it_s).accountId());
          if (acc.accountGroup() == MyMoneyAccount::Asset
              || acc.accountGroup() == MyMoneyAccount::Liability) {
            if (acc.accountType() != MyMoneyAccount::Loan
                && acc.accountType() != MyMoneyAccount::AssetLoan) {
              split = *it_s;
              break;
            }
          }
        }
        if (it_s == transaction.splits().constEnd()) {
          qWarning("Split for payment account not found in %s:%d.", __FILE__, __LINE__);
        }
        break;

      default:
        if (!s1.value().isPositive())
          split = s1;
        else
          split = s2;
        break;
    }
    acc = MyMoneyFile::instance()->account(split.accountId());

    item->setText(0, schedule.name());
    MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());

    QString accName =  acc.name();
    if (!accName.isEmpty()) {
      item->setText(1, accName);
    } else {
      item->setText(1, "---");
    }
    item->setData(1, KScheduleTreeItem::OrderRole, QVariant(accName));

    QString payeeName;
    if (!s1.payeeId().isEmpty()) {
      payeeName = MyMoneyFile::instance()->payee(s1.payeeId()).name();
      item->setText(2, payeeName);
    } else {
      item->setText(2, "---");
    }
    item->setData(2, KScheduleTreeItem::OrderRole, QVariant(payeeName));

    MyMoneyMoney amount = split.shares().abs();
    item->setData(3, Qt::UserRole, QVariant::fromValue(amount));
    if (!accName.isEmpty()) {
      item->setText(3, QString("%1  ").arg(MyMoneyUtils::formatMoney(amount, acc, currency)));
    } else {
      //there are some cases where the schedule does not have an account
      //in those cases the account will not have a fraction
      //use base currency instead
      item->setText(3, QString("%1  ").arg(MyMoneyUtils::formatMoney(amount, MyMoneyFile::instance()->baseCurrency())));
    }
    item->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
    item->setData(3, KScheduleTreeItem::OrderRole, QVariant::fromValue(amount));

    // Do the real next payment like ms-money etc
    QDate nextDueDate;
    if (schedule.isFinished()) {
      item->setText(4, i18nc("Finished schedule", "Finished"));
    } else {
      nextDueDate = schedule.adjustedNextDueDate();
      item->setText(4, KGlobal::locale()->formatDate(schedule.adjustedNextDueDate(), KLocale::ShortDate));
    }
    item->setData(4, KScheduleTreeItem::OrderRole, QVariant(nextDueDate));
    item->setText(5, i18nc("Frequency of schedule", schedule.occurrenceToString().toLatin1()));
    item->setText(6, KMyMoneyUtils::paymentMethodToString(schedule.paymentType()));
  } catch (const MyMoneyException &e) {
    item->setText(0, "Error:");
    item->setText(1, e.what());
  }
  return item;
}

void KScheduledView::slotTimerDone()
{
  QTreeWidgetItem* item;

  item = m_scheduleTree->currentItem();
  if (item) {
    m_scheduleTree->scrollToItem(item);
  }

  // force a repaint of all items to update the branches
  /*for (item = m_scheduleTree->item(0); item != 0; item = m_scheduleTree->item(m_scheduleTree->row(item) + 1)) {
    m_scheduleTree->repaintItem(item);
  }
  resize(width(), height() + 1);*/
}

void KScheduledView::slotReloadView()
{
  m_needReload = true;
  if (isVisible()) {
    m_qbuttonNew->setEnabled(true);
    m_tabWidget->setEnabled(true);

    refresh(true, m_selectedSchedule);

    m_needReload = false;
    QTimer::singleShot(50, this, SLOT(slotRearrange()));
  }
}

void KScheduledView::showEvent(QShowEvent* event)
{
  emit aboutToShow();

  if (m_needReload)
    slotReloadView();

  QWidget::showEvent(event);
}

void KScheduledView::slotRearrange()
{
  resizeEvent(0);
}

void KScheduledView::readConfig()
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  m_openBills = grp.readEntry("KScheduleView_openBills", true);
  m_openDeposits = grp.readEntry("KScheduleView_openDeposits", true);
  m_openTransfers = grp.readEntry("KScheduleView_openTransfers", true);
  m_openLoans = grp.readEntry("KScheduleView_openLoans", true);
  m_tabWidget->setCurrentIndex(grp.readEntry("KScheduleView_tab", 0));
  QByteArray columns;
  columns = grp.readEntry("KScheduleView_treeState", columns);
  m_scheduleTree->header()->restoreState(columns);
  m_scheduleTree->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());
}

void KScheduledView::writeConfig()
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KScheduleView_openBills", m_openBills);
  grp.writeEntry("KScheduleView_openDeposits", m_openDeposits);
  grp.writeEntry("KScheduleView_openTransfers", m_openTransfers);
  grp.writeEntry("KScheduleView_openLoans", m_openLoans);
  grp.writeEntry("KScheduleView_tab", m_tabWidget->currentIndex());
  QByteArray columns = m_scheduleTree->header()->saveState();
  grp.writeEntry("KScheduleView_treeState", columns);

  config->sync();
}

void KScheduledView::slotListViewContextMenu(const QPoint& pos)
{
  QTreeWidgetItem* item = m_scheduleTree->itemAt(pos);
  if (item) {
    try {
      MyMoneySchedule schedule = item->data(0, Qt::UserRole).value<MyMoneySchedule>();
      emit scheduleSelected(schedule);
      m_selectedSchedule = schedule.id();
      emit openContextMenu();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Error activating context menu"), e.what());
    }
  } else {
    emit openContextMenu();
  }
}

void KScheduledView::slotListItemExecuted(QTreeWidgetItem* item, int)
{
  if (!item)
    return;

  try {
    MyMoneySchedule schedule = item->data(0, Qt::UserRole).value<MyMoneySchedule>();
    m_selectedSchedule = schedule.id();
    emit editSchedule();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Error executing item"), e.what());
  }
}

void KScheduledView::slotAccountActivated()
{
  m_filterAccounts.clear();

  try {

    int accountCount = 0;
    MyMoneyFile* file = MyMoneyFile::instance();

    // extract a list of all accounts under the asset and liability groups
    // and sort them by name
    QList<MyMoneyAccount> list;
    QStringList accountList = file->asset().accountList();
    accountList.append(file->liability().accountList());
    file->accountList(list, accountList, true);
    qStableSort(list.begin(), list.end(), accountNameLessThan);

    QList<MyMoneyAccount>::ConstIterator it_a;
    for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
      if (!(*it_a).isClosed()) {
        if (!m_kaccPopup->actions().value(accountCount)->isChecked()) {
          m_filterAccounts.append((*it_a).id());
        }
        ++accountCount;
      }
    }

    m_calendar->setFilterAccounts(m_filterAccounts);

    refresh(false, m_selectedSchedule);
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedError(this, i18n("Unable to filter account"), e.what());
  }
}

void KScheduledView::slotListViewExpanded(QTreeWidgetItem* item)
{
  if (item) {
    if (item->text(0) == i18n("Bills"))
      m_openBills = true;
    else if (item->text(0) == i18n("Deposits"))
      m_openDeposits = true;
    else if (item->text(0) == i18n("Transfers"))
      m_openTransfers = true;
    else if (item->text(0) == i18n("Loans"))
      m_openLoans = true;
  }
}

void KScheduledView::slotListViewCollapsed(QTreeWidgetItem* item)
{
  if (item) {
    if (item->text(0) == i18n("Bills"))
      m_openBills = false;
    else if (item->text(0) == i18n("Deposits"))
      m_openDeposits = false;
    else if (item->text(0) == i18n("Transfers"))
      m_openTransfers = false;
    else if (item->text(0) == i18n("Loans"))
      m_openLoans = false;
  }
}

void KScheduledView::slotSelectSchedule(const QString& schedule)
{
  refresh(true, schedule);
}

void KScheduledView::slotBriefEnterClicked(const MyMoneySchedule& schedule, const QDate& date)
{
  Q_UNUSED(date);

  emit scheduleSelected(schedule);
  emit enterSchedule();
}

void KScheduledView::slotBriefSkipClicked(const MyMoneySchedule& schedule, const QDate& date)
{
  Q_UNUSED(date);

  emit scheduleSelected(schedule);
  emit skipSchedule();
}

void KScheduledView::slotSetSelectedItem()
{
  emit scheduleSelected(MyMoneySchedule());
  QTreeWidgetItem* item = m_scheduleTree->currentItem();
  if (item) {
    try {
      MyMoneySchedule schedule = item->data(0, Qt::UserRole).value<MyMoneySchedule>();
      emit scheduleSelected(schedule);
      m_selectedSchedule = schedule.id();
    } catch (const MyMoneyException &e) {
      qDebug("KScheduledView::slotSetSelectedItem: %s", qPrintable(e.what()));
    }
  }
}
