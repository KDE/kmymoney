/***************************************************************************
                          kscheduledview_p.h  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSCHEDULEDVIEW_P_H
#define KSCHEDULEDVIEW_P_H

#include "kscheduledview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QTimer>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QScopedPointer>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConfig>
#include <KMessageBox>
#include <KSharedConfig>
#include <KTreeWidgetSearchLine>
#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kscheduledview.h"
#include "kmymoneyviewbase_p.h"
#include "kenterscheduledlg.h"
#include "kbalancewarning.h"
#include "transactioneditor.h"
#include "kconfirmmanualenterdlg.h"
#include "kmymoneymvccombo.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
#include "mymoneyexception.h"
#include "kscheduletreeitem.h"
#include "ktreewidgetfilterlinewidget.h"
#include "icons/icons.h"
#include "mymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneyschedule.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"
#include "menuenums.h"
#include "dialogenums.h"

using namespace Icons;

class KScheduledViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KScheduledView)

public:
  explicit KScheduledViewPrivate(KScheduledView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KScheduledView)
    , m_kaccPopup(nullptr)
    , m_openBills(true)
    , m_openDeposits(true)
    , m_openTransfers(true)
    , m_openLoans(true)
    , m_needLoad(true)
    , m_searchWidget(nullptr)
    , m_balanceWarning(nullptr)
  {
  }

  ~KScheduledViewPrivate()
  {
    if(!m_needLoad)
      writeConfig();
    delete ui;
  }

  void init()
  {
    Q_Q(KScheduledView);
    m_needLoad = false;
    ui->setupUi(q);

    // create the searchline widget
    // and insert it into the existing layout
    m_searchWidget = new KTreeWidgetFilterLineWidget(q, ui->m_scheduleTree);
    ui->vboxLayout->insertWidget(1, m_searchWidget);

    //enable custom context menu
    ui->m_scheduleTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_scheduleTree->setSelectionMode(QAbstractItemView::SingleSelection);

    readConfig();

    q->connect(ui->m_qbuttonNew, &QAbstractButton::clicked, pActions[eMenu::Action::NewSchedule], &QAction::trigger);

    // attach popup to 'Filter...' button
    m_kaccPopup = new QMenu(q);
    ui->m_accountsCombo->setMenu(m_kaccPopup);
    q->connect(m_kaccPopup, &QMenu::triggered, q, &KScheduledView::slotAccountActivated);

    KGuiItem::assign(ui->m_qbuttonNew, KMyMoneyUtils::scheduleNewGuiItem());
    KGuiItem::assign(ui->m_accountsCombo, KMyMoneyUtils::accountsFilterGuiItem());

    q->connect(ui->m_scheduleTree, &QWidget::customContextMenuRequested, q, &KScheduledView::customContextMenuRequested);
    q->connect(ui->m_scheduleTree, &QTreeWidget::itemSelectionChanged,
            q, &KScheduledView::slotSetSelectedItem);

    q->connect(ui->m_scheduleTree, &QTreeWidget::itemDoubleClicked,
            q, &KScheduledView::slotListItemExecuted);
    q->connect(ui->m_scheduleTree, &QTreeWidget::itemExpanded,
            q, &KScheduledView::slotListViewExpanded);
    q->connect(ui->m_scheduleTree, &QTreeWidget::itemCollapsed,
            q, &KScheduledView::slotListViewCollapsed);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KScheduledView::refresh);
  }

  static bool accountNameLessThan(const MyMoneyAccount& acc1, const MyMoneyAccount& acc2)
  {
    return acc1.name().toLower() < acc2.name().toLower();
  }

  void refreshSchedule(bool full, const QString& schedId)
  {
    Q_Q(KScheduledView);
    ui->m_scheduleTree->header()->setFont(KMyMoneySettings::listHeaderFontEx());

    ui->m_scheduleTree->clear();

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
          KMessageBox::detailedError(q, i18n("Unable to load accounts: "), e.what());
        }
      }

      MyMoneyFile *file = MyMoneyFile::instance();
      QList<MyMoneySchedule> scheduledItems = file->scheduleList();

      if (scheduledItems.count() == 0)
        return;

      //disable sorting for performance
      ui->m_scheduleTree->setSortingEnabled(false);

      KScheduleTreeItem *itemBills = new KScheduleTreeItem(ui->m_scheduleTree);
      itemBills->setIcon(0, Icons::get(Icon::Expense));
      itemBills->setText(0, i18n("Bills"));
      itemBills->setData(0, KScheduleTreeItem::OrderRole, QVariant("0"));
      itemBills->setFirstColumnSpanned(true);
      itemBills->setFlags(Qt::ItemIsEnabled);
      QFont bold = itemBills->font(0);
      bold.setBold(true);
      itemBills->setFont(0, bold);
      KScheduleTreeItem *itemDeposits = new KScheduleTreeItem(ui->m_scheduleTree);
      itemDeposits->setIcon(0, Icons::get(Icon::Income));
      itemDeposits->setText(0, i18n("Deposits"));
      itemDeposits->setData(0, KScheduleTreeItem::OrderRole, QVariant("1"));
      itemDeposits->setFirstColumnSpanned(true);
      itemDeposits->setFlags(Qt::ItemIsEnabled);
      itemDeposits->setFont(0, bold);
      KScheduleTreeItem *itemLoans = new KScheduleTreeItem(ui->m_scheduleTree);
      itemLoans->setIcon(0, Icons::get(Icon::Loan));
      itemLoans->setText(0, i18n("Loans"));
      itemLoans->setData(0, KScheduleTreeItem::OrderRole, QVariant("2"));
      itemLoans->setFirstColumnSpanned(true);
      itemLoans->setFlags(Qt::ItemIsEnabled);
      itemLoans->setFont(0, bold);
      KScheduleTreeItem *itemTransfers = new KScheduleTreeItem(ui->m_scheduleTree);
      itemTransfers->setIcon(0, Icons::get(Icon::Transaction));
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
          case eMyMoney::Schedule::Type::Any:
            // Should we display an error ?
            // We just sort it as bill and fall through here

          case eMyMoney::Schedule::Type::Bill:
            parent = itemBills;
            break;

          case eMyMoney::Schedule::Type::Deposit:
            parent = itemDeposits;
            break;

          case eMyMoney::Schedule::Type::Transfer:
            parent = itemTransfers;
            break;

          case eMyMoney::Schedule::Type::LoanPayment:
            parent = itemLoans;
            break;

        }
        if (parent) {
          if (!KMyMoneySettings::hideFinishedSchedules() || !schedData.isFinished()) {
            item = addScheduleItem(parent, schedData);
            if (schedData.id() == schedId)
              openItem = item;
          }
        }
      }

      if (openItem) {
        ui->m_scheduleTree->setCurrentItem(openItem);
      }
      // using a timeout is the only way, I got the 'ensureTransactionVisible'
      // working when coming from hidden form to visible form. I assume, this
      // has something to do with the delayed update of the display somehow.
      q->resize(q->width(), q->height() - 1);
      QTimer::singleShot(10, q, SLOT(slotTimerDone()));
      ui->m_scheduleTree->update();

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
      KMessageBox::error(q, e.what());
    }

    for (int i = 0; i < ui->m_scheduleTree->columnCount(); ++i) {
      ui->m_scheduleTree->resizeColumnToContents(i);
    }

    //reenable sorting after loading items
    ui->m_scheduleTree->setSortingEnabled(true);
  }

  void readConfig()
  {
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Last Use Settings");
    m_openBills = grp.readEntry("KScheduleView_openBills", true);
    m_openDeposits = grp.readEntry("KScheduleView_openDeposits", true);
    m_openTransfers = grp.readEntry("KScheduleView_openTransfers", true);
    m_openLoans = grp.readEntry("KScheduleView_openLoans", true);
    QByteArray columns;
    columns = grp.readEntry("KScheduleView_treeState", columns);
    ui->m_scheduleTree->header()->restoreState(columns);
    ui->m_scheduleTree->header()->setFont(KMyMoneySettings::listHeaderFontEx());
  }

  void writeConfig()
  {
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Last Use Settings");
    grp.writeEntry("KScheduleView_openBills", m_openBills);
    grp.writeEntry("KScheduleView_openDeposits", m_openDeposits);
    grp.writeEntry("KScheduleView_openTransfers", m_openTransfers);
    grp.writeEntry("KScheduleView_openLoans", m_openLoans);
    QByteArray columns = ui->m_scheduleTree->header()->saveState();
    grp.writeEntry("KScheduleView_treeState", columns);

    config->sync();
  }

  QTreeWidgetItem* addScheduleItem(QTreeWidgetItem* parent, MyMoneySchedule& schedule)
  {
    KScheduleTreeItem* item = new KScheduleTreeItem(parent);
    item->setData(0, Qt::UserRole, QVariant::fromValue(schedule));
    item->setData(0, KScheduleTreeItem::OrderRole, schedule.name());
    if (!schedule.isFinished()) {
      if (schedule.isOverdue()) {
        item->setIcon(0, Icons::get(Icon::UpcomingEvents));
        QBrush brush = item->foreground(0);
        brush.setColor(Qt::red);
        for (int i = 0; i < ui->m_scheduleTree->columnCount(); ++i) {
          item->setForeground(i, brush);
        }
      } else {
        item->setIcon(0, Icons::get(Icon::CalendarDay));
      }
    } else {
      item->setIcon(0, Icons::get(Icon::DialogClose));
      QBrush brush = item->foreground(0);
      brush.setColor(Qt::darkGreen);
      for (int i = 0; i < ui->m_scheduleTree->columnCount(); ++i) {
        item->setForeground(i, brush);
      }
    }

    try {
      MyMoneyTransaction transaction = schedule.transaction();
      MyMoneySplit s1 = (transaction.splits().size() < 1) ? MyMoneySplit() : transaction.splits()[0];
      MyMoneySplit s2 = (transaction.splits().size() < 2) ? MyMoneySplit() : transaction.splits()[1];
      MyMoneySplit split;
      MyMoneyAccount acc;

      switch (schedule.type()) {
      case eMyMoney::Schedule::Type::Deposit:
        if (s1.value().isNegative())
          split = s2;
        else
          split = s1;
        break;

      case eMyMoney::Schedule::Type::LoanPayment:
      {
        auto found = false;
        foreach (const auto it_split, transaction.splits()) {
          acc = MyMoneyFile::instance()->account(it_split.accountId());
          if (acc.accountGroup() == eMyMoney::Account::Type::Asset
              || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
            if (acc.accountType() != eMyMoney::Account::Type::Loan
                && acc.accountType() != eMyMoney::Account::Type::AssetLoan) {
              split = it_split;
              found = true;
              break;
            }
          }
        }
        if (!found) {
          qWarning("Split for payment account not found in %s:%d.", __FILE__, __LINE__);
        }
        break;
      }
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
        item->setText(4, QLocale().toString(schedule.adjustedNextDueDate(), QLocale::ShortFormat));
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

  /**
    * This method allows to enter the next scheduled transaction of
    * the given schedule @a s. In case @a extendedKeys is @a true,
    * the given schedule can also be skipped or ignored.
    * If @a autoEnter is @a true and the schedule does not contain
    * an estimated value, the schedule is entered as is without further
    * interaction with the user. In all other cases, the user will
    * be presented a dialog and allowed to adjust the values for this
    * instance of the schedule.
    *
    * The transaction will be created and entered into the ledger
    * and the schedule updated.
    */
  eDialogs::ScheduleResultCode enterSchedule(MyMoneySchedule& schedule, bool autoEnter = false, bool extendedKeys = false)
  {
    Q_Q(KScheduledView);
    auto rc = eDialogs::ScheduleResultCode::Cancel;
    if (!schedule.id().isEmpty()) {
      try {
        schedule = MyMoneyFile::instance()->schedule(schedule.id());
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(q, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
        return rc;
      }

      QWidget* parent = QApplication::activeWindow();
      QPointer<KEnterScheduleDlg> dlg = new KEnterScheduleDlg(parent, schedule);
      qDebug() << "parent widget" << (void*) parent;

      try {
        QDate origDueDate = schedule.nextDueDate();

        dlg->showExtendedKeys(extendedKeys);

        QPointer<TransactionEditor> transactionEditor = dlg->startEdit();
        if (transactionEditor) {
          KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
          MyMoneyTransaction torig, taccepted;
          transactionEditor->createTransaction(torig, dlg->transaction(),
              schedule.transaction().splits().isEmpty() ? MyMoneySplit() : schedule.transaction().splits().front(), true);
          // force actions to be available no matter what (will be updated according to the state during
          // slotTransactionsEnter or slotTransactionsCancel)
          pActions[eMenu::Action::CancelTransaction]->setEnabled(true);
          pActions[eMenu::Action::EnterTransaction]->setEnabled(true);

          KConfirmManualEnterDlg::Action action = KConfirmManualEnterDlg::ModifyOnce;
          if (!autoEnter || !schedule.isFixed()) {
            for (; dlg != 0;) {
              rc = eDialogs::ScheduleResultCode::Cancel;
              if (dlg->exec() == QDialog::Accepted && dlg != 0) {
                rc = dlg->resultCode();
                if (rc == eDialogs::ScheduleResultCode::Enter) {
                  transactionEditor->createTransaction(taccepted, torig, torig.splits().isEmpty() ? MyMoneySplit() : torig.splits().front(), true);
                  // make sure to suppress comparison of some data: postDate
                  torig.setPostDate(taccepted.postDate());
                  if (torig != taccepted) {
                    QPointer<KConfirmManualEnterDlg> cdlg =
                      new KConfirmManualEnterDlg(schedule, q);
                    cdlg->loadTransactions(torig, taccepted);
                    if (cdlg->exec() == QDialog::Accepted) {
                      action = cdlg->action();
                      delete cdlg;
                      break;
                    }
                    delete cdlg;
                    // the user has chosen 'cancel' during confirmation,
                    // we go back to the editor
                    continue;
                  }
                } else if (rc == eDialogs::ScheduleResultCode::Skip) {
                  slotTransactionsCancel(transactionEditor, schedule);
                  skipSchedule(schedule);
                } else {
                  slotTransactionsCancel(transactionEditor, schedule);
                }
              } else {
                if (autoEnter) {
                  if (KMessageBox::warningYesNo(q, i18n("Are you sure you wish to stop this scheduled transaction from being entered into the register?\n\nKMyMoney will prompt you again next time it starts unless you manually enter it later.")) == KMessageBox::No) {
                    // the user has chosen 'No' for the above question,
                    // we go back to the editor
                    continue;
                  }
                }
                slotTransactionsCancel(transactionEditor, schedule);
              }
              break;
            }
          }

          // if we still have the editor around here, the user did not cancel
          if ((transactionEditor != 0) && (dlg != 0)) {
            MyMoneyFileTransaction ft;
            try {
              MyMoneyTransaction t;
              // add the new transaction
              switch (action) {
                case KConfirmManualEnterDlg::UseOriginal:
                  // setup widgets with original transaction data
                  transactionEditor->setTransaction(dlg->transaction(), dlg->transaction().splits().isEmpty() ? MyMoneySplit() : dlg->transaction().splits().front());
                  // and create a transaction based on that data
                  taccepted = MyMoneyTransaction();
                  transactionEditor->createTransaction(taccepted, dlg->transaction(),
                      dlg->transaction().splits().isEmpty() ? MyMoneySplit() : dlg->transaction().splits().front(), true);
                  break;

                case KConfirmManualEnterDlg::ModifyAlways:
                  torig = taccepted;
                  torig.setPostDate(origDueDate);
                  schedule.setTransaction(torig);
                  break;

                case KConfirmManualEnterDlg::ModifyOnce:
                  break;
              }

              QString newId;
              q->connect(transactionEditor, SIGNAL(balanceWarning(QWidget*,MyMoneyAccount,QString)), m_balanceWarning.data(), SLOT(slotShowMessage(QWidget*,MyMoneyAccount,QString)));
              if (transactionEditor->enterTransactions(newId, false)) {
                if (!newId.isEmpty()) {
                  t = MyMoneyFile::instance()->transaction(newId);
                  schedule.setLastPayment(t.postDate());
                }
                // in case the next due date is invalid, the schedule is finished
                // we mark it as such by setting the next due date to one day past the end
                QDate nextDueDate = schedule.nextPayment(origDueDate);
                if (!nextDueDate.isValid()) {
                  schedule.setNextDueDate(schedule.endDate().addDays(1));
                } else {
                  schedule.setNextDueDate(nextDueDate);
                }
                MyMoneyFile::instance()->modifySchedule(schedule);
                rc = eDialogs::ScheduleResultCode::Enter;

                // delete the editor before we emit the dataChanged() signal from the
                // engine. Calling this twice in a row does not hurt.
                delete transactionEditor;
                ft.commit();
              }
            } catch (const MyMoneyException &e) {
              KMessageBox::detailedSorry(q, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
            }
            delete transactionEditor;
          }
        }
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(q, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
      }
      delete dlg;
    }
    return rc;
  }

  void slotTransactionsCancel(TransactionEditor* editor, const MyMoneySchedule& schedule)
  {
    Q_Q(KScheduledView);
    // since we jump here via code, we have to make sure to react only
    // if the action is enabled
    if (pActions[eMenu::Action::CancelTransaction]->isEnabled()) {
      // make sure, we block the enter function
      pActions[eMenu::Action::EnterTransaction]->setEnabled(false);
      // qDebug("KMyMoneyApp::slotTransactionsCancel");
      delete editor;
      emit q->selectByObject(schedule, eView::Intent::None);
    }
  }

  /**
    * This method allows to skip the next scheduled transaction of
    * the given schedule @a s.
    *
    */
  void skipSchedule(MyMoneySchedule& schedule)
  {
    Q_Q(KScheduledView);
    if (!schedule.id().isEmpty()) {
      try {
        schedule = MyMoneyFile::instance()->schedule(schedule.id());
        if (!schedule.isFinished()) {
          if (schedule.occurrence() != eMyMoney::Schedule::Occurrence::Once) {
            QDate next = schedule.nextDueDate();
            if (!schedule.isFinished() && (KMessageBox::questionYesNo(q, i18n("<qt>Do you really want to skip the <b>%1</b> transaction scheduled for <b>%2</b>?</qt>", schedule.name(), QLocale().toString(next, QLocale::ShortFormat)))) == KMessageBox::Yes) {
              MyMoneyFileTransaction ft;
              schedule.setLastPayment(next);
              schedule.setNextDueDate(schedule.nextPayment(next));
              MyMoneyFile::instance()->modifySchedule(schedule);
              ft.commit();
            }
          }
        }
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(q, i18n("<qt>Unable to skip scheduled transaction <b>%1</b>.</qt>", schedule.name()), e.what());
      }
    }
  }

  Ui::KScheduledView  *ui;
  /// The selected schedule id in the list view.
  QMenu *m_kaccPopup;
  QStringList m_filterAccounts;
  bool m_openBills;
  bool m_openDeposits;
  bool m_openTransfers;
  bool m_openLoans;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  /**
   * Search widget for the list
   */
  KTreeWidgetSearchLineWidget*  m_searchWidget;
  MyMoneySchedule m_currentSchedule;

  QScopedPointer<KBalanceWarning> m_balanceWarning;
};

#endif
