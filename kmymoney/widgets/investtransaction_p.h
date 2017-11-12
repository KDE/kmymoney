/***************************************************************************
                          investtransaction_p.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef INVESTTRANSACTION_P_H
#define INVESTTRANSACTION_P_H

#include "investtransaction.h"
#include "transaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPainter>
#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "transaction.h"

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneysecurity.h"
#include "register.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "transactionform.h"
#include "kmymoneylineedit.h"
#include "kmymoneyedit.h"
#include "transactioneditor.h"
#include "investtransactioneditor.h"
#include "kmymoneyutils.h"
#include "kmymoneymvccombo.h"
#ifndef KMM_DESIGNER
#include "stdtransactioneditor.h"
#endif

#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

namespace KMyMoneyRegister
{
  class InvestTransactionPrivate : public TransactionPrivate
  {
  public:
    QList<MyMoneySplit>       m_feeSplits;
    QList<MyMoneySplit>       m_interestSplits;
    MyMoneySplit              m_assetAccountSplit;
    MyMoneySecurity           m_security;
    MyMoneySecurity           m_currency;
    eMyMoney::Split::InvestmentTransactionType    m_transactionType;
    QString                   m_feeCategory;
    QString                   m_interestCategory;
    MyMoneyMoney              m_feeAmount;
    MyMoneyMoney              m_interestAmount;
    MyMoneyMoney              m_totalAmount;
  };
}

#endif
