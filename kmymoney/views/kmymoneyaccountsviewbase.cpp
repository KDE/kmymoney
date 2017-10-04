/***************************************************************************
                          kmymoneyaccountsviewbase.cpp
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
