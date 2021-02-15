/*
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneyaccountsviewbase.h"
#include "kmymoneyaccountsviewbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


KMyMoneyAccountsViewBase::KMyMoneyAccountsViewBase(QWidget* parent) :
    KMyMoneyViewBase(*new KMyMoneyAccountsViewBasePrivate, parent)
{
}

KMyMoneyAccountsViewBase::KMyMoneyAccountsViewBase(KMyMoneyAccountsViewBasePrivate &dd, QWidget *parent)
    : KMyMoneyViewBase(dd, parent)
{
}

KMyMoneyAccountsViewBase::~KMyMoneyAccountsViewBase()
{
}

AccountsViewProxyModel *KMyMoneyAccountsViewBase::getProxyModel()
{
  Q_D(KMyMoneyAccountsViewBase);
  return d->m_proxyModel;
}

KMyMoneyAccountTreeView *KMyMoneyAccountsViewBase::getTreeView()
{
  Q_D(KMyMoneyAccountsViewBase);
  return *d->m_accountTree;
}
