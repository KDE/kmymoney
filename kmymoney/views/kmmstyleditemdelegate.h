/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMSTYLEDITEMDELEGATE_H
#define KMMSTYLEDITEMDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class KMMStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit KMMStyledItemDelegate(QWidget* parent = 0);
    virtual ~KMMStyledItemDelegate();

    /**
     * Make the editorEvent publically available
     */
    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    /**
     * Make the eventFilter publically available
     */
    bool eventFilter ( QObject * watched, QEvent * event ) override;
};

#endif // KMMSTYLEDITEMDELEGATE_H

