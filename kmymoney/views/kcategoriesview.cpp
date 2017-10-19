/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
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

#include "kcategoriesview.h"
#include "kcategoriesview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

using namespace Icons;

KCategoriesView::KCategoriesView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KCategoriesViewPrivate(this), parent)
{
}

KCategoriesView::KCategoriesView(KCategoriesViewPrivate &dd, QWidget *parent)
    : KMyMoneyAccountsViewBase(dd, parent)
{
}

KCategoriesView::~KCategoriesView()
{
}

void KCategoriesView::setDefaultFocus()
{
  Q_D(KCategoriesView);
  QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
}

void KCategoriesView::refresh()
{
  Q_D(KCategoriesView);
  if (!isVisible()) {
    d->m_needsRefresh = true;
    return;
  }
  d->m_needsRefresh = false;

  d->m_proxyModel->invalidate();
  d->m_proxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());

  // reinitialize the default state of the hidden categories label
  d->m_haveUnusedCategories = false;
  d->ui->m_hiddenCategories->hide();
  d->m_proxyModel->setHideUnusedIncomeExpenseAccounts(KMyMoneyGlobalSettings::hideUnusedCategory());
}

void KCategoriesView::showEvent(QShowEvent * event)
{
  Q_D(KCategoriesView);
  if (!d->m_proxyModel)
    d->init();

  emit aboutToShow(View::Categories);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KCategoriesView::slotUnusedIncomeExpenseAccountHidden()
{
  Q_D(KCategoriesView);
  d->m_haveUnusedCategories = true;
  d->ui->m_hiddenCategories->setVisible(d->m_haveUnusedCategories);
}

void KCategoriesView::slotProfitChanged(const MyMoneyMoney &profit)
{
  Q_D(KCategoriesView);
  d->netBalProChanged(profit, d->ui->m_totalProfitsLabel, View::Categories);
}
