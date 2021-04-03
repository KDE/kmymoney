/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NATIONALACCOUNTDELEGATE_H
#define NATIONALACCOUNTDELEGATE_H

#include "kmm_base_widgets_export.h"

#include <QStyledItemDelegate>

#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "payeeidentifier/payeeidentifiertyped.h"

class KMM_BASE_WIDGETS_EXPORT nationalAccountDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit nationalAccountDelegate(QObject* parent, const QVariantList& options = QVariantList());

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const final override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

Q_SIGNALS:
    void sizeHintChanged(const QModelIndex&) const;

private:
    inline payeeIdentifierTyped<payeeIdentifiers::nationalAccount> identByIndex(const QModelIndex& index) const;

};

#endif // NATIONALACCOUNTDELEGATE_H
