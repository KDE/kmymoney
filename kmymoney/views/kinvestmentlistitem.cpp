/***************************************************************************
                          kinvestmentlistitem.cpp  -  description
                             -------------------
    begin                : Wed Feb 6 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kinvestmentlistitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyglobalsettings.h>
#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include <config-kmymoney.h>

KInvestmentListItem::KInvestmentListItem(K3ListView* parent, const MyMoneyAccount& account)
    : K3ListViewItem(parent)
{
  bColumn5Negative = false;
  bColumn6Negative = false;
  bColumn7Negative = false;
  bColumn8Negative = false;
  bColumn9Negative = false;

  m_account = account;
  m_listView = parent;

  MyMoneySecurity security;
  MyMoneyFile* file = MyMoneyFile::instance();

  security = file->security(m_account.currencyId());
  m_tradingCurrency = file->security(security.tradingCurrency());

  int prec = MyMoneyMoney::denomToPrec(m_tradingCurrency.smallestAccountFraction());

  QList<MyMoneyTransaction> transactionList;
  // FIXME PRICE
  // equity_price_history history = equity.priceHistory();

  //column 0 (COLUMN_NAME_INDEX) is the name of the stock
  setText(COLUMN_NAME_INDEX, m_account.name());

  //column 1 (COLUMN_SYMBOL_INDEX) is the ticker symbol
  setText(COLUMN_SYMBOL_INDEX, security.tradingSymbol());

  //column 2 is the net value (price * quantity owned)
  MyMoneyPrice price = file->price(m_account.currencyId(), m_tradingCurrency.id());
  if (price.isValid()) {
    setText(COLUMN_VALUE_INDEX, (file->balance(m_account.id()) * price.rate(m_tradingCurrency.id())).formatMoney(m_tradingCurrency.tradingSymbol(), prec));
  } else {
    setText(COLUMN_VALUE_INDEX, "---");
  }

  //column 3 (COLUMN_QUANTITY_INDEX) is the quantity of shares owned
  prec = MyMoneyMoney::denomToPrec(security.smallestAccountFraction());
  setText(COLUMN_QUANTITY_INDEX, file->balance(m_account.id()).formatMoney("", prec));

  //column 4 is the current price
  // Get the price precision from the configuration
  prec = KMyMoneyGlobalSettings::pricePrecision();

  // prec = MyMoneyMoney::denomToPrec(m_tradingCurrency.smallestAccountFraction());
  if (price.isValid()) {
    setText(COLUMN_PRICE_INDEX, price.rate(m_tradingCurrency.id()).formatMoney(m_tradingCurrency.tradingSymbol(), prec));
  } else {
    setText(COLUMN_PRICE_INDEX, "---");
  }
}

KInvestmentListItem::~KInvestmentListItem()
{
}

int KInvestmentListItem::compare(Q3ListViewItem* i, int col, bool ascending) const
{
  KInvestmentListItem* item = dynamic_cast<KInvestmentListItem*>(i);
  // do special sorting only for numeric columns
  // in all other cases use the standard sorting
  if (item) {
    switch (col) {
      case COLUMN_VALUE_INDEX:
      case COLUMN_QUANTITY_INDEX:
      case COLUMN_PRICE_INDEX: {
          bool inv1 = text(col) == "---";
          bool inv2 = item->text(col) == "---";
          if (!inv1 && !inv2) {
            MyMoneyMoney result = MyMoneyMoney(text(col)) - MyMoneyMoney(item->text(col));
            if (result.isNegative())
              return -1;
            if (result.isZero())
              return 0;
            return 1;
          } else if (inv1 && inv2) {
            return 0;
          } else if (inv1) {
            return -1;
          }
          return 1;
        }
        break;

      default:
        break;
    }
  }

  // do standard sorting here
  return K3ListViewItem::compare(i, col, ascending);
}

void KInvestmentListItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
  bool bPaintRed = false;
  if ((column == COLUMN_RAWGAIN_INDEX && bColumn5Negative) ||
      (column == COLUMN_1WEEKGAIN_INDEX && bColumn6Negative) ||
      (column == COLUMN_4WEEKGAIN_INDEX && bColumn7Negative) ||
      (column == COLUMN_3MONGAIN_INDEX && bColumn8Negative) ||
      (column == COLUMN_YTDGAIN_INDEX && bColumn9Negative)) {
    bPaintRed = true;
  }

  p->save();

  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

#ifndef KMM_DESIGNER
  QFont font = KMyMoneyGlobalSettings::listCellFont();
  // strike out closed accounts
  if (m_account.isClosed())
    font.setStrikeOut(true);

  p->setFont(font);
#endif

  if (bPaintRed) {
    QColorGroup _cg(cg2);
    QColor c = _cg.text();
    _cg.setColor(QColorGroup::Text, Qt::red);
    Q3ListViewItem::paintCell(p, _cg, column, width, align);
    _cg.setColor(QColorGroup::Text, c);
  } else {
    Q3ListViewItem::paintCell(p, cg2, column, width, align);
  }

  p->restore();
}
