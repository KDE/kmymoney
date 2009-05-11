/***************************************************************************
                          kinvestmentlistitem.h  -  description
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

#ifndef KINVESTMENTLISTITEM_H
#define KINVESTMENTLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneyobserver.h>

//indexes for the various columns on the summary view
#define COLUMN_NAME_INDEX       0
#define COLUMN_SYMBOL_INDEX     1
#define COLUMN_VALUE_INDEX      2
#define COLUMN_QUANTITY_INDEX   3
#define COLUMN_PRICE_INDEX      4
#define COLUMN_COSTBASIS_INDEX  5
#define COLUMN_RAWGAIN_INDEX    6
#define COLUMN_1WEEKGAIN_INDEX  7
#define COLUMN_4WEEKGAIN_INDEX  8
#define COLUMN_3MONGAIN_INDEX   9
#define COLUMN_YTDGAIN_INDEX    10

/**
  * @author Kevin Tambascio
  * @author Thomas Baumgart
  */
class KInvestmentListItem : public K3ListViewItem
{
public:
  KInvestmentListItem(K3ListView* parent, const MyMoneyAccount& security);
  ~KInvestmentListItem();

  QString securityId() const { return m_account.currencyId(); };
  const MyMoneyAccount& account(void) const { return m_account; };
  const MyMoneySecurity tradingCurrency(void) const { return m_tradingCurrency; };

 /**
   * Helper method to show the right order
   */
  int compare(Q3ListViewItem* i, int col, bool ascending) const;


protected:
  void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
  // FIXME PRICE
#if 0
  const QString calculate1WeekGain(const equity_price_history& history);
  const QString calculate4WeekGain(const equity_price_history& history);
  const QString calculate3MonthGain(const equity_price_history& history);
  const QString calculateYTDGain(const equity_price_history& history);
  const QString calculateGain(const equity_price_history& history, int dayDifference, int monthDifference, bool YTD, bool& bNegative);
#endif

private:
  K3ListView*        m_listView;
  MyMoneyAccount    m_account;
  MyMoneySecurity   m_tradingCurrency;
  bool bColumn5Negative, bColumn6Negative, bColumn7Negative, bColumn8Negative, bColumn9Negative;
};

#endif
