/***************************************************************************
                          viewinterface.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include "kmmviewinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyview.h"
#include "selectedtransactions.h"

KMyMoneyPlugin::KMMViewInterface::KMMViewInterface(KMyMoneyView* view, QObject* parent, const char* name) :
    ViewInterface(parent, name),
    m_view(view)
{
  connect(m_view, &KMyMoneyView::accountSelected, this, &ViewInterface::accountSelected);
  connect(m_view, &KMyMoneyView::transactionsSelected, this, &ViewInterface::transactionsSelected);
  connect(m_view, &KMyMoneyView::accountReconciled,
          this, &ViewInterface::accountReconciled);

//  connect(app, &KMyMoney::institutionSelected, this, &ViewInterface::institutionSelected);

  connect(m_view, &KMyMoneyView::viewStateChanged, this, &ViewInterface::viewStateChanged);
}

void KMyMoneyPlugin::KMMViewInterface::slotRefreshViews()
{
  m_view->slotRefreshViews();
}

void KMyMoneyPlugin::KMMViewInterface::addView(KMyMoneyViewBase* view, const QString& name, View idView)
{
  m_view->addView(view, name, idView);
}

void KMyMoneyPlugin::KMMViewInterface::removeView(View idView)
{
  m_view->removeView(idView);
}

//KMyMoneyViewBase* KMyMoneyPlugin::KMMViewInterface::addPage(const QString& item, const QString& icon)
//{
//  return m_view->addBasePage(item, icon);
//}

//void KMyMoneyPlugin::KMMViewInterface::addWidget(KMyMoneyViewBase* view, QWidget* w)
//{
//  if (view && w)
//    view->addWidget(w);
//}
