/*
    SPDX-FileCopyrightText: 2009 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KBALANCEWARNING_H
#define KBALANCEWARNING_H
// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
class QString;
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

class KBalanceWarning : public QObject
{
    Q_OBJECT
public:
    explicit KBalanceWarning(QObject* parent);
    virtual ~KBalanceWarning();

public Q_SLOTS:
    void slotShowMessage(QWidget* parent, const MyMoneyAccount& account, const QString& msg);

private:
    class Private;
    Private* d;
};

#endif
