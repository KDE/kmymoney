/***************************************************************************
                          kcurrencycalculator.h  -  description
                             -------------------
    begin                : Thu Apr 8 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KCURRENCYCALCULATOR_H
#define KCURRENCYCALCULATOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/kcurrencycalculatordecl.h"
#include <kmymoney/mymoneyfile.h>

/**
  * @author Thomas Baumgart
  */

class KCurrencyCalculator : public KCurrencyCalculatorDecl
{
  Q_OBJECT

public:
  /**
    * @param from the @p from currency
    * @param to   the @p to currency
    * @param value the value to be converted
    * @param shares the number of foreign currency units
    * @param date the date when the conversion took place
    * @param resultFraction the smallest fraction of the result (default 100)
    * @param parent see QWidget constructor
    * @param name see QWidget constructor
    *
    * @note @p value must not be 0!
    */
  KCurrencyCalculator(const MyMoneySecurity& from, const MyMoneySecurity& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const signed64 resultFraction = 100, QWidget *parent=0, const char *name=0);
  ~KCurrencyCalculator();

  const MyMoneyMoney price(void) const;

  void setupPriceEditor(void);

  static bool setupSplitPrice(MyMoneyMoney& shares, const MyMoneyTransaction& t, const MyMoneySplit& s, const QMap<QString, MyMoneyMoney>& priceInfo, QWidget* parentWidget);

protected:
  void updateExample(const MyMoneyMoney& price);

protected slots:
  void slotSetToAmount(void);
  void slotSetExchangeRate(void);
  void slotUpdateResult(const QString& txt);
  void slotUpdateRate(const QString& txt);
  virtual void accept(void);

private:
  MyMoneySecurity     m_fromCurrency;
  MyMoneySecurity     m_toCurrency;
  MyMoneyMoney        m_result;
  MyMoneyMoney        m_value;
  signed64            m_resultFraction;
};

#endif
