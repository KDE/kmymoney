/***************************************************************************
                          viewinterface.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include "kmymoney.h"
#include "kmymoneyview.h"
#include "selectedtransaction.h"

KMyMoneyPlugin::KMMViewInterface::KMMViewInterface(KMyMoneyApp* app, KMyMoneyView* view, QObject* parent, const char* name) :
    ViewInterface(parent, name),
    m_view(view)
{
  connect(app, SIGNAL(accountSelected(MyMoneyAccount)), this, SIGNAL(accountSelected(MyMoneyAccount)));
  connect(app, SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)), this, SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)));
  connect(app, SIGNAL(accountReconciled(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)),
          this, SIGNAL(accountReconciled(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)));


  connect(app, SIGNAL(institutionSelected(MyMoneyInstitution)), this, SIGNAL(institutionSelected(MyMoneyInstitution)));

  connect(m_view, SIGNAL(viewStateChanged(bool)), this, SIGNAL(viewStateChanged(bool)));
  connect(m_view, SIGNAL(kmmFilePlugin(uint)), this, SIGNAL(kmmFilePlugin(uint)));
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
