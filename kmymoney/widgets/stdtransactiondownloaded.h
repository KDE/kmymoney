/*
    SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STDTRANSACTIONDOWNLOADED_H
#define STDTRANSACTIONDOWNLOADED_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "stdtransaction.h"
#include "investtransaction.h"

namespace KMyMoneyRegister
{

class StdTransactionDownloaded : public StdTransaction
{
public:
    explicit StdTransactionDownloaded(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransactionDownloaded() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class InvestTransactionDownloaded : public InvestTransaction
{
public:
    explicit InvestTransactionDownloaded(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~InvestTransactionDownloaded() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
};


} // namespace

#endif
