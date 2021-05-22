/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECONCILIATIONDELEGATE_H
#define RECONCILIATIONDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

class QColor;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmstyleditemdelegate.h"
#include "mymoneyenums.h"

class MyMoneyMoney;

class ReconciliationDelegate : public KMMStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ReconciliationDelegate(QWidget* parent = 0);
    virtual ~ReconciliationDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

protected:
    bool eventFilter(QObject* o, QEvent* event) final override;

Q_SIGNALS:
    void sizeHintChanged(const QModelIndex&) const;

private:
    class Private;
    Private* const d;

    static QColor m_erroneousColor;
    static QColor m_importedColor;
    static QColor m_separatorColor;
};

#endif // RECONCILIATIONDELEGATE_H
