/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactionformitemdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transactionform.h"

using namespace KMyMoneyTransactionForm;

TransactionFormItemDelegate::TransactionFormItemDelegate(TransactionForm *parent) : QStyledItemDelegate(parent), m_transactionForm(parent)
{
}

void TransactionFormItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  m_transactionForm->paintCell(painter, option, index);
}
