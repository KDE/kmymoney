/*
    SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STDTRANSACTIONMATCHED_H
#define STDTRANSACTIONMATCHED_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "stdtransaction.h"

class MyMoneySplit;
class MyMoneyTransaction;

namespace KMyMoneyRegister
{

class Register;
class StdTransactionMatched : public StdTransaction
{
    static const int m_additionalRows = 3;

public:
    explicit StdTransactionMatched(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransactionMatched() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;

    void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister(bool)
    */
    int numRowsRegister(bool expanded) const override;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
    int numRowsRegister() const override;
};

} // namespace

#endif
