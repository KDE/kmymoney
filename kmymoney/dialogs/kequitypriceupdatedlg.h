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

#include "webpricequote.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "kequitypriceupdateconfdlg.h"
#include "ui_kequitypriceupdatedlgdecl.h"

/**
  * @author Kevin Tambascio & Ace Jones
  */
class MyMoneyStatement;
class KEquityPriceUpdateDlgDecl  : public QDialog, public Ui::KEquityPriceUpdateDlgDecl
{
public:
  KEquityPriceUpdateDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KEquityPriceUpdateDlg : public KEquityPriceUpdateDlgDecl
{
  Q_OBJECT
public:
  explicit KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId = QString());
  ~KEquityPriceUpdateDlg();
  void storePrices();
  MyMoneyPrice price(const QString& id) const;

protected slots:
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
  void addPricePair(const MyMoneySecurityPair& pair, bool dontCheckExistance = false);
  void addInvestment(const MyMoneySecurity& inv);
  void finishUpdate();

private:
  bool m_fUpdateAll;
  updatingPricePolicyE m_updatingPricePolicy;
  WebPriceQuote m_webQuote;
};

#endif // KEQUITYPRICEUPDATEDLG_H
