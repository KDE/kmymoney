/*
 * SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
  explicit KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId = QString());
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
