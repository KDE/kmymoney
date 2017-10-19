/***************************************************************************
                          kinstitutionsview.cpp
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

#include "kinstitutionsview.h"
#include "kinstitutionsview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

using namespace Icons;

KInstitutionsView::KInstitutionsView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KInstitutionsViewPrivate(this), parent)
{
}

KInstitutionsView::KInstitutionsView(KInstitutionsViewPrivate &dd, QWidget *parent)
    : KMyMoneyAccountsViewBase(dd, parent)
{
}

KInstitutionsView::~KInstitutionsView()
{
}

void KInstitutionsView::setDefaultFocus()
{
  Q_D(KInstitutionsView);
  QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
}

void KInstitutionsView::refresh()
{
  Q_D(KInstitutionsView);
  if (!isVisible()) {
    d->m_needsRefresh = true;
    return;
  }
  d->m_needsRefresh = false;

  d->m_proxyModel->invalidate();
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  d->m_proxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());
}

void KInstitutionsView::showEvent(QShowEvent * event)
{
  Q_D(KInstitutionsView);
  if (!d->m_proxyModel)
    d->init();

  emit aboutToShow(View::Institutions);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInstitutionsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  Q_D(KInstitutionsView);
  d->netBalProChanged(netWorth, d->ui->m_totalProfitsLabel, View::Institutions);
}
