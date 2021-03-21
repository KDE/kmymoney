/*
    SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scheduledtransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyleOptionViewItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionScheduled::StdTransactionScheduled(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
    StdTransaction(parent, transaction, split, uniqueId)
{
    // setup initial size
    setNumRowsRegister(numRowsRegister(KMyMoneySettings::showRegisterDetailed()));
}

StdTransactionScheduled::~StdTransactionScheduled()
{
}

const char* StdTransactionScheduled::className()
{
    return "StdTransactionScheduled";
}

bool StdTransactionScheduled::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
    option.palette.setCurrentColorGroup(QPalette::Disabled);
    return rc;
}

bool StdTransactionScheduled::isSelectable() const
{
    return true;
}

bool StdTransactionScheduled::canHaveFocus() const
{
    return true;
}

bool StdTransactionScheduled::isScheduled() const
{
    return true;
}

int StdTransactionScheduled::sortSamePostDate() const
{
    return 4;
}


