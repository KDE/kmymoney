/***************************************************************************
                          khomeview.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
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
#ifndef KHOMEVIEW_H
#define KHOMEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <khtml_part.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyschedule.h"
#include "mymoneyaccount.h"
#include "kmymoneyview.h"

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  *
  * @short A view containing the home page for kmymoney.
**/
class KHomeView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  /**
    * Definition of bitmap used as argument for showAccounts().
    */
  enum paymentTypeE {
    Preferred = 1,          ///< show preferred accounts
    Payment = 2             ///< show payment accounts
  };

  explicit KHomeView(QWidget *parent = 0, const char *name = 0);
  ~KHomeView();

protected:
  void showPayments();
  void showPaymentEntry(const MyMoneySchedule&, int cnt = 1);
  void showAccounts(paymentTypeE type, const QString& hdr);
  void showAccountEntry(const MyMoneyAccount&);
  void showFavoriteReports();
  void showForecast();
  void showNetWorthGraph();
  void showSummary();
  void showAssetsLiabilities();
  void showIncomeExpenseSummary();
  void showSchedulesSummary();
  void showBudget();
  void showCashFlowSummary();

  const QString link(const QString& view, const QString& query, const QString& title = QString()) const;
  const QString linkend() const;
  void loadView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
    */
  void showEvent(QShowEvent* event);

public slots:

  void slotOpenUrl(const KUrl &url, const KParts::OpenUrlArguments &args, const KParts::BrowserArguments &browArgs);
  void slotLoadView();

  /**
    * Print the current view
    */
  void slotPrintView();

  /**
    * Opens the print preview for the current view
    */
  void slotPrintPreviewView();

  /**
    * Generates a print for a given printer
    */
  void slotPaintRequested(QPrinter *printer);

  void slotZoomView(int);

signals:
  void ledgerSelected(const QString& id, const QString& transaction);
  void scheduleSelected(const QString& id);
  void reportSelected(const QString& id);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  /**
    * Print an account and its balance and limit
    */
  void showAccountEntry(const MyMoneyAccount& acc, const MyMoneyMoney& value, const MyMoneyMoney& valueToMinBal, const bool showMinBal);

  /**
    * @param acc the investment account
    * @return the balance in the currency of the investment account
    */
  MyMoneyMoney investmentBalance(const MyMoneyAccount& acc);

  /**
   * Print text in the color set for negative numbers, if @p amount is negative
   * abd @p isNegative is true
   */
  QString showColoredAmount(const QString& amount, bool isNegative);

  /**
   * Run the forecast
   */
  void doForecast();

  /**
   * Calculate the forecast balance after a payment has been made
   */
  MyMoneyMoney forecastPaymentBalance(const MyMoneyAccount& acc, const MyMoneyMoney& payment, QDate& paymentDate);
};

#endif
