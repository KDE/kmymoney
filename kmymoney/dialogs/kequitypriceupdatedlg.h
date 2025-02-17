/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KEQUITYPRICEUPDATEDLG_H
#define KEQUITYPRICEUPDATEDLG_H

#include "kmm_extended_dialogs_export.h"

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
class KMM_EXTENDED_DIALOGS_EXPORT KEquityPriceUpdateDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KEquityPriceUpdateDlg)

public:
    explicit KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId = QString());
    ~KEquityPriceUpdateDlg();
    void storePrices();
    MyMoneyPrice price(const QString& id) const;

    void setSearchShortcut(const QKeySequence& shortcut);

protected:
    void keyPressEvent(QKeyEvent* ev) override;

private:
    KEquityPriceUpdateDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KEquityPriceUpdateDlg)

};

#endif // KEQUITYPRICEUPDATEDLG_H
