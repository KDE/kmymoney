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

#include "kscheduledview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QTimer>
#include <QPushButton>
#include <QMenu>
#include <QIcon>

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
#include "keditloanwizard.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyexception.h"
#include "kscheduletreeitem.h"
#include "keditscheduledlg.h"
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

using namespace Icons;

KScheduledView::KScheduledView(QWidget *parent) :
  KMyMoneyViewBase(*new KScheduledViewPrivate(this), parent)
{
  typedef void(KScheduledView::*KScheduledViewFunc)();
  const QHash<eMenu::Action, KScheduledViewFunc> actionConnections {
    {eMenu::Action::NewSchedule,        &KScheduledView::slotNewSchedule},
    {eMenu::Action::EditSchedule,       &KScheduledView::slotEditSchedule},
    {eMenu::Action::DeleteSchedule,     &KScheduledView::slotDeleteSchedule},
    {eMenu::Action::DuplicateSchedule,  &KScheduledView::slotDuplicateSchedule},
    {eMenu::Action::EnterSchedule,      &KScheduledView::slotEnterSchedule},
    {eMenu::Action::SkipSchedule,       &KScheduledView::slotSkipSchedule},
  };

  for (auto a = actionConnections.cbegin(); a != actionConnections.cend(); ++a)
    connect(pActions[a.key()], &QAction::triggered, this, a.value());

  Q_D(KScheduledView);
  d->m_balanceWarning.reset(new KBalanceWarning(this));
}

KScheduledView::~KScheduledView()
{
}

void KScheduledView::setDefaultFocus()
{
  Q_D(KScheduledView);
  QTimer::singleShot(0, d->m_searchWidget->searchLine(), SLOT(setFocus()));
}

void KScheduledView::slotTimerDone()
{
  Q_D(KScheduledView);
  QTreeWidgetItem* item;

  item = d->ui->m_scheduleTree->currentItem();
  if (item) {
    d->ui->m_scheduleTree->scrollToItem(item);
  }

  // force a repaint of all items to update the branches
  /*for (item = d->ui->m_scheduleTree->item(0); item != 0; item = d->ui->m_scheduleTree->item(d->ui->m_scheduleTree->row(item) + 1)) {
    d->ui->m_scheduleTree->repaintItem(item);
  }
  resize(width(), height() + 1);*/
}

void KScheduledView::refresh()
{
  Q_D(KScheduledView);
  if (isVisible()) {
    d->ui->m_qbuttonNew->setEnabled(true);

    d->refreshSchedule(true, d->m_currentSchedule.id());

    d->m_needsRefresh = false;
    QTimer::singleShot(50, this, SLOT(slotRearrange()));
  } else {
    d->m_needsRefresh = true;
  }
}

void KScheduledView::showEvent(QShowEvent* event)
{
  Q_D(KScheduledView);
  if (d->m_needLoad)
    d->init();

  emit aboutToShow(View::Schedules);

  if (d->m_needsRefresh)
    refresh();

  QWidget::showEvent(event);
}

void KScheduledView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KScheduledView);
  if (typeid(obj) != typeid(MyMoneySchedule) &&
      (obj.id().isEmpty() && d->m_currentSchedule.id().isEmpty())) // do not disable actions that were already disabled))
    return;

  const auto& sch = static_cast<const MyMoneySchedule&>(obj);

  const QVector<eMenu::Action> actionsToBeDisabled {
        eMenu::Action::EditSchedule, eMenu::Action::DuplicateSchedule, eMenu::Action::DeleteSchedule,
        eMenu::Action::EnterSchedule, eMenu::Action::SkipSchedule,
  };

  for (const auto& a : actionsToBeDisabled)
    pActions[a]->setEnabled(false);

  pActions[eMenu::Action::NewSchedule]->setEnabled(true);

  if (!sch.id().isEmpty()) {
    pActions[eMenu::Action::EditSchedule]->setEnabled(true);
    pActions[eMenu::Action::DuplicateSchedule]->setEnabled(true);
    pActions[eMenu::Action::DeleteSchedule]->setEnabled(!MyMoneyFile::instance()->isReferenced(sch));
    if (!sch.isFinished()) {
      pActions[eMenu::Action::EnterSchedule]->setEnabled(true);
      // a schedule with a single occurrence cannot be skipped
      if (sch.occurrence() != eMyMoney::Schedule::Occurrence::Once) {
        pActions[eMenu::Action::SkipSchedule]->setEnabled(true);
      }
    }
  }
  d->m_currentSchedule = sch;
}

eDialogs::ScheduleResultCode KScheduledView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
  Q_D(KScheduledView);
  return d->enterSchedule(schedule, autoEnter, extendedKeys);
}

void KScheduledView::slotRearrange()
{
  resizeEvent(0);
}

void KScheduledView::customContextMenuRequested(const QPoint)
{
  Q_D(KScheduledView);
  emit objectSelected(d->m_currentSchedule);
  emit contextMenuRequested(d->m_currentSchedule);
}

void KScheduledView::slotListItemExecuted(QTreeWidgetItem* item, int)
{
  Q_D(KScheduledView);
  if (!item)
    return;

  try {
    const auto sch = item->data(0, Qt::UserRole).value<MyMoneySchedule>();
    emit objectSelected(sch);
    emit openObjectRequested(sch);
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Error executing item"), e.what());
  }
}

void KScheduledView::slotAccountActivated()
{
  Q_D(KScheduledView);
  d->m_filterAccounts.clear();

  try {

    int accountCount = 0;
    MyMoneyFile* file = MyMoneyFile::instance();

    // extract a list of all accounts under the asset and liability groups
    // and sort them by name
    QList<MyMoneyAccount> list;
    QStringList accountList = file->asset().accountList();
    accountList.append(file->liability().accountList());
    file->accountList(list, accountList, true);
    qStableSort(list.begin(), list.end(), d->accountNameLessThan);

    QList<MyMoneyAccount>::ConstIterator it_a;
    for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
      if (!(*it_a).isClosed()) {
        if (!d->m_kaccPopup->actions().value(accountCount)->isChecked()) {
          d->m_filterAccounts.append((*it_a).id());
        }
        ++accountCount;
      }
    }

    d->refreshSchedule(false, d->m_currentSchedule.id());
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedError(this, i18n("Unable to filter account"), e.what());
  }
}

void KScheduledView::slotListViewExpanded(QTreeWidgetItem* item)
{
  Q_D(KScheduledView);
  if (item) {
    if (item->text(0) == i18n("Bills"))
      d->m_openBills = true;
    else if (item->text(0) == i18n("Deposits"))
      d->m_openDeposits = true;
    else if (item->text(0) == i18n("Transfers"))
      d->m_openTransfers = true;
    else if (item->text(0) == i18n("Loans"))
      d->m_openLoans = true;
  }
}

void KScheduledView::slotListViewCollapsed(QTreeWidgetItem* item)
{
  Q_D(KScheduledView);
  if (item) {
    if (item->text(0) == i18n("Bills"))
      d->m_openBills = false;
    else if (item->text(0) == i18n("Deposits"))
      d->m_openDeposits = false;
    else if (item->text(0) == i18n("Transfers"))
      d->m_openTransfers = false;
    else if (item->text(0) == i18n("Loans"))
      d->m_openLoans = false;
  }
}

void KScheduledView::slotSelectSchedule(const QString& schedule)
{
  Q_D(KScheduledView);
  d->refreshSchedule(true, schedule);
}

void KScheduledView::slotShowScheduleMenu(const MyMoneySchedule& sch)
{
  Q_UNUSED(sch)
  pMenus[eMenu::Menu::Schedule]->exec(QCursor::pos());
}

void KScheduledView::slotSetSelectedItem()
{
  Q_D(KScheduledView);
  MyMoneySchedule sch;
  const auto item = d->ui->m_scheduleTree->currentItem();
  if (item) {
    try {
      sch = item->data(0, Qt::UserRole).value<MyMoneySchedule>();
    } catch (const MyMoneyException &e) {
      qDebug("KScheduledView::slotSetSelectedItem: %s", qPrintable(e.what()));
    }
  }

  emit objectSelected(sch);
}

void KScheduledView::slotNewSchedule()
{
  KEditScheduleDlg::newSchedule(MyMoneyTransaction(), eMyMoney::Schedule::Occurrence::Monthly);
}

void KScheduledView::slotEditSchedule()
{
  Q_D(KScheduledView);
  KEditScheduleDlg::editSchedule(d->m_currentSchedule);
}

void KScheduledView::slotDeleteSchedule()
{
  Q_D(KScheduledView);
  if (!d->m_currentSchedule.id().isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneySchedule sched = MyMoneyFile::instance()->schedule(d->m_currentSchedule.id());
      QString msg = i18n("<p>Are you sure you want to delete the scheduled transaction <b>%1</b>?</p>", d->m_currentSchedule.name());
      if (sched.type() == eMyMoney::Schedule::Type::LoanPayment) {
        msg += QString(" ");
        msg += i18n("In case of loan payments it is currently not possible to recreate the scheduled transaction.");
      }
      if (KMessageBox::questionYesNo(this, msg) == KMessageBox::No)
        return;

      MyMoneyFile::instance()->removeSchedule(sched);
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to remove scheduled transaction '%1'", d->m_currentSchedule.name()), e.what());
    }
  }
}

void KScheduledView::slotDuplicateSchedule()
{
  Q_D(KScheduledView);
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[eMenu::Action::DuplicateSchedule]->isEnabled()) {
    MyMoneySchedule sch = d->m_currentSchedule;
    sch.clearId();
    sch.setLastPayment(QDate());
    sch.setName(i18nc("Copy of scheduled transaction name", "Copy of %1", sch.name()));
    // make sure that we set a valid next due date if the original next due date is invalid
    if (!sch.nextDueDate().isValid())
      sch.setNextDueDate(QDate::currentDate());

    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addSchedule(sch);
      ft.commit();

      // select the new schedule in the view
      if (!d->m_currentSchedule.id().isEmpty())
        emit objectSelected(sch);

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to duplicate scheduled transaction: '%1'", d->m_currentSchedule.name()), e.what());
    }
  }
}

void KScheduledView::slotEnterSchedule()
{
  Q_D(KScheduledView);
  if (!d->m_currentSchedule.id().isEmpty()) {
    try {
      auto schedule = MyMoneyFile::instance()->schedule(d->m_currentSchedule.id());
      d->enterSchedule(schedule);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unknown scheduled transaction '%1'", d->m_currentSchedule.name()), e.what());
    }
  }
}

void KScheduledView::slotSkipSchedule()
{
  Q_D(KScheduledView);
  if (!d->m_currentSchedule.id().isEmpty()) {
    try {
      auto schedule = MyMoneyFile::instance()->schedule(d->m_currentSchedule.id());
      d->skipSchedule(schedule);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unknown scheduled transaction '%1'", d->m_currentSchedule.name()), e.what());
    }
  }
}
