/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONFORMITEMDELEGATE_H
#define TRANSACTIONFORMITEMDELEGATE_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyTransactionForm
{
class TransactionForm;
class KMM_OLDREGISTER_EXPORT TransactionFormItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(TransactionFormItemDelegate)

public:
    explicit TransactionFormItemDelegate(TransactionForm *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    TransactionForm *m_transactionForm;
};
} // namespace

#endif
