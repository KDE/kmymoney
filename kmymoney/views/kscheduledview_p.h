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
#include <QAction>
#include <QScopedPointer>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConfig>
#include <KMessageBox>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kscheduledview.h"
#include "kmymoneyviewbase_p.h"
#include "kmymoneysettings.h"
#include "kenterscheduledlg.h"
#include "kbalancewarning.h"
#include "transactioneditor.h"
#include "kconfirmmanualenterdlg.h"
#include "kmymoneymvccombo.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
#include "mymoneyexception.h"
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
#include "schedulesmodel.h"
#include "scheduleproxymodel.h"

class KScheduledViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KScheduledView)

public:
  explicit KScheduledViewPrivate(KScheduledView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KScheduledView)
    , m_filterModel(nullptr)
    , m_needLoad(true)
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

    //enable custom context menu
    ui->m_scheduleTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_scheduleTree->setSelectionMode(QAbstractItemView::SingleSelection);

    m_filterModel = new ScheduleProxyModel(q);
    m_filterModel->setSourceModel(MyMoneyFile::instance()->schedulesModel());
    m_filterModel->setSortLocaleAware(true);
    m_filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterKeyColumn(-1);

    ui->m_scheduleTree->setModel(m_filterModel);

    readConfig();

    q->connect(ui->m_qbuttonNew, &QAbstractButton::clicked, pActions[eMenu::Action::NewSchedule], &QAction::trigger);
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_filterModel, &QSortFilterProxyModel::setFilterFixedString);

    KGuiItem::assign(ui->m_qbuttonNew, KMyMoneyUtils::scheduleNewGuiItem());

    q->connect(ui->m_scheduleTree->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KScheduledView::slotSetSelectedItem);

    q->connect(ui->m_scheduleTree, &QTreeView::expanded, q, &KScheduledView::slotListViewExpanded);
    q->connect(ui->m_scheduleTree, &QTreeView::collapsed, q, &KScheduledView::slotListViewCollapsed);

    // update settings
    settingsChanged();

    // Setup collapsed and expanded groups
    const auto model = ui->m_scheduleTree->model();
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
      const auto idx = model->index(row, 0);
      const auto groupType = static_cast<eMyMoney::Schedule::Type>(idx.data(eMyMoney::Model::ScheduleTypeRole).toInt());
      if (m_expandedGroups.contains(groupType)) {
        ui->m_scheduleTree->setExpanded(idx, m_expandedGroups[groupType]);
      }
    }
  }

  void settingsChanged()
  {
    if (m_filterModel) {
      m_filterModel->setHideFinishedSchedules(KMyMoneySettings::hideFinishedSchedules());
      m_filterModel->invalidate();
    }
  }

  void readConfig()
  {
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Last Use Settings");
    m_expandedGroups[eMyMoney::Schedule::Type::Bill] = grp.readEntry("KScheduleView_openBills", true);
    m_expandedGroups[eMyMoney::Schedule::Type::Deposit] = grp.readEntry("KScheduleView_openDeposits", true);
    m_expandedGroups[eMyMoney::Schedule::Type::Transfer] = grp.readEntry("KScheduleView_openTransfers", true);
    m_expandedGroups[eMyMoney::Schedule::Type::LoanPayment] = grp.readEntry("KScheduleView_openLoans", true);
    QByteArray columns;
    columns = grp.readEntry("KScheduleView_treeState", columns);
    ui->m_scheduleTree->header()->restoreState(columns);
    ui->m_scheduleTree->header()->setFont(KMyMoneySettings::listHeaderFontEx());
    int sortCol = grp.readEntry<int>("KScheduleView_sortColumn", SchedulesModel::Column::Name);
    auto sortOrder = static_cast<Qt::SortOrder>(grp.readEntry<int>("KScheduleView_sortOrder", Qt::AscendingOrder));
    m_filterModel->sort(sortCol, sortOrder);
  }

  void writeConfig()
  {
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Last Use Settings");
    grp.writeEntry("KScheduleView_openBills", m_expandedGroups[eMyMoney::Schedule::Type::Bill]);
    grp.writeEntry("KScheduleView_openDeposits", m_expandedGroups[eMyMoney::Schedule::Type::Deposit]);
    grp.writeEntry("KScheduleView_openTransfers", m_expandedGroups[eMyMoney::Schedule::Type::Transfer]);
    grp.writeEntry("KScheduleView_openLoans", m_expandedGroups[eMyMoney::Schedule::Type::LoanPayment]);
    QByteArray columns = ui->m_scheduleTree->header()->saveState();
    grp.writeEntry("KScheduleView_treeState", columns);
    grp.writeEntry("KScheduleView_sortColumn", m_filterModel->sortColumn());
    grp.writeEntry("KScheduleView_sortOrder", static_cast<int>(m_filterModel->sortOrder()));
    config->sync();
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
    const auto parentWidget = QApplication::activeWindow();

    if (!schedule.id().isEmpty()) {
      try {
        schedule = MyMoneyFile::instance()->schedule(schedule.id());
        if (!schedule.isFinished()) {
          if (schedule.occurrence() != eMyMoney::Schedule::Occurrence::Once) {
            QDate next = schedule.nextDueDate();
            if (!schedule.isFinished() && (KMessageBox::questionYesNo( parentWidget, i18n("<qt>Do you really want to skip the <b>%1</b> transaction scheduled for <b>%2</b>?</qt>", schedule.name(), QLocale().toString(next, QLocale::ShortFormat)))) == KMessageBox::Yes) {
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

  Ui::KScheduledView* ui;
  ScheduleProxyModel* m_filterModel;
  QHash<eMyMoney::Schedule::Type, bool> m_expandedGroups;

  /**
    * This member holds the initial load state of the view
    */
  bool m_needLoad;

  MyMoneySchedule m_currentSchedule;

  QScopedPointer<KBalanceWarning> m_balanceWarning;
};

#endif
