/***************************************************************************
                          kequitypriceupdatedlg.h  -  description
                             -------------------
    begin                : Tuesday June 22nd, 2004
    copyright            : (C) 2000-2004 by Kevin Tambascio
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

#ifndef KEQUITYPRICEUPDATEDLG_H
#define KEQUITYPRICEUPDATEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;
class MyMoneyStatement;
class MyMoneyPrice;

typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

/**
  * @author Kevin Tambascio & Ace Jones
  */

class KEquityPriceUpdateDlgPrivate;
class KEquityPriceUpdateDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KEquityPriceUpdateDlg)

public:
  explicit KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId);
  explicit KEquityPriceUpdateDlg(QWidget *parent);
  ~KEquityPriceUpdateDlg();
  void storePrices();
  MyMoneyPrice price(const QString& id) const;

protected Q_SLOTS:
  void slotConfigureClicked();
  void slotUpdateSelectedClicked();
  void slotUpdateAllClicked();
  void slotUpdateSelection();
  void slotDateChanged();

  void logStatusMessage(const QString&);
  void logErrorMessage(const QString&);
  void slotReceivedCSVQuote(const QString& _kmmID, const QString& _webID, MyMoneyStatement& st);
  void slotReceivedQuote(const QString& _kmmID, const QString& _webID, const QDate&, const double&);
  void slotQuoteFailed(const QString& _kmmID, const QString& _webID);

protected:
  void addInvestment(const MyMoneySecurity& inv);
  void finishUpdate();

private:
  KEquityPriceUpdateDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KEquityPriceUpdateDlg)

};

#endif // KEQUITYPRICEUPDATEDLG_H
