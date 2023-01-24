/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SECURITYACCOUNTNAMEDELEGATE_H
#define SECURITYACCOUNTNAMEDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

class QColor;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmstyleditemdelegate.h"
#include "mymoneyenums.h"

class LedgerView;
class MyMoneyMoney;

class SecurityAccountNameDelegate : public KMMStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SecurityAccountNameDelegate(LedgerView* parent = 0);
    virtual ~SecurityAccountNameDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

protected:
    bool eventFilter(QObject* o, QEvent* event) final override;

Q_SIGNALS:
    void sizeHintChanged(const QModelIndex&) const;

private:
    class Private;
    Private* const d;
};

#endif // SECURITYACCOUNTNAMEDELEGATE_H
