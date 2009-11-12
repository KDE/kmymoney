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

KMyMoneyPlugin::KMMViewInterface::KMMViewInterface(KMyMoney2App* app, KMyMoneyView* view, QObject* parent, const char* name) :
  ViewInterface(parent, name),
  m_app(app),
  m_view(view)
{
  connect(app, SIGNAL(accountSelected(const MyMoneyAccount&)), this, SIGNAL(accountSelected(const MyMoneyAccount&)));
  connect(app, SIGNAL(transactionsSelected(const KMyMoneyRegister::SelectedTransactions&)), this, SIGNAL(transactionsSelected(const KMyMoneyRegister::SelectedTransactions&)));
  connect(app, SIGNAL(accountReconciled(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&, const MyMoneyMoney&, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >&)),
          this, SIGNAL(accountReconciled(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&, const MyMoneyMoney&, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >&)));


  connect(app, SIGNAL(institutionSelected(const MyMoneyInstitution&)), this, SIGNAL(institutionSelected(const MyMoneyInstitution&)));

  connect(m_view, SIGNAL(viewStateChanged(bool)), this, SIGNAL(viewStateChanged(bool)));
  connect(m_view, SIGNAL(kmmFilePlugin(unsigned int)), this, SIGNAL(kmmFilePlugin(unsigned int)));
}

KMyMoneyViewBase* KMyMoneyPlugin::KMMViewInterface::addPage(const QString& item, const QString& icon)
{
  return m_view->addBasePage(item, icon);
}

void KMyMoneyPlugin::KMMViewInterface::addWidget(KMyMoneyViewBase* view, QWidget* w)
{
  if(view && w)
    view->addWidget(w);
}


#include "kmmviewinterface.moc"
