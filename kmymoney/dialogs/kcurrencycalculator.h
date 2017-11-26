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

#ifndef KCURRENCYCALCULATOR_H
#define KCURRENCYCALCULATOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneySecurity;

namespace Ui { class KCurrencyCalculator; }

typedef qint64 signed64;

/**
  * @author Thomas Baumgart
  */

class KCurrencyCalculatorPrivate;
class KCurrencyCalculator : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KCurrencyCalculator)

public:
  /**
    * @param from the @p from currency
    * @param to   the @p to currency
    * @param value the value to be converted
    * @param shares the number of foreign currency units
    * @param date the date when the conversion took place
    * @param resultFraction the smallest fraction of the result (default 100)
    * @param parent see QWidget constructor
    *
    * @note @p value must not be 0!
    */
  explicit KCurrencyCalculator(const MyMoneySecurity& from,
                               const MyMoneySecurity& to,
                               const MyMoneyMoney& value,
                               const MyMoneyMoney& shares,
                               const QDate& date,
                               const signed64 resultFraction = 100,
                               QWidget *parent = nullptr);
  ~KCurrencyCalculator();

  /**
   * This method returns the price determined by the method selected by the user
   * which is either
   *
   * a) based on the resulting amount or
   * b) based on direct price entry.
   *
   * In case a) the price is returned without precision loss as the devision
   * of the amount entered by the user and the @a value passed as argument.
   * In case b) it is returned with the selected global price precision.
   */
  MyMoneyMoney price() const;

  void setupPriceEditor();

  static bool setupSplitPrice(MyMoneyMoney& shares,
                              const MyMoneyTransaction& t,
                              const MyMoneySplit& s,
                              const QMap<QString,
                              MyMoneyMoney>& priceInfo,
                              QWidget* parentWidget);

protected Q_SLOTS:
  void slotSetToAmount();
  void slotSetExchangeRate();
  void slotUpdateResult(const QString& txt);
  void slotUpdateRate(const QString& txt);
  void accept() override;

private:
  KCurrencyCalculatorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KCurrencyCalculator)
};

#endif
