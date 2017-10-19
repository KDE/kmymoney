/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kaccountsview.h"
#include "kaccountsview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

using namespace Icons;

KAccountsView::KAccountsView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KAccountsViewPrivate(this), parent)
{
}

KAccountsView::KAccountsView(KAccountsViewPrivate &dd, QWidget *parent)
    : KMyMoneyAccountsViewBase(dd, parent)
{
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::setDefaultFocus()
{
  Q_D(KAccountsView);
  QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
}

void KAccountsView::refresh()
{
  Q_D(KAccountsView);
  if (!isVisible()) {
    d->m_needsRefresh = true;
    return;
  }
  d->m_needsRefresh = false;
  // TODO: check why the invalidate is needed here
  d->m_proxyModel->invalidate();
  d->m_proxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView()) {
    d->m_proxyModel->addAccountGroup(QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Income, MyMoneyAccount::Expense});
  } else {
    d->m_proxyModel->removeAccountType(MyMoneyAccount::Income);
    d->m_proxyModel->removeAccountType(MyMoneyAccount::Expense);
  }

  // reinitialize the default state of the hidden categories label
  d->m_haveUnusedCategories = false;
  d->ui->m_hiddenCategories->hide();  // hides label
  d->m_proxyModel->setHideUnusedIncomeExpenseAccounts(KMyMoneyGlobalSettings::hideUnusedCategory());
}

void KAccountsView::showEvent(QShowEvent * event)
{
  Q_D(KAccountsView);
  if (!d->m_proxyModel)
    d->init();

  emit aboutToShow(View::Accounts);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KAccountsView::slotUnusedIncomeExpenseAccountHidden()
{
  Q_D(KAccountsView);
  d->m_haveUnusedCategories = true;
  d->ui->m_hiddenCategories->setVisible(d->m_haveUnusedCategories);
}

void KAccountsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  Q_D(KAccountsView);
  d->netBalProChanged(netWorth, d->ui->m_totalProfitsLabel, View::Accounts);
}
