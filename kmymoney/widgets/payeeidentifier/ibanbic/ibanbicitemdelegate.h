/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IBANBICITEMDELEGATE_H
#define IBANBICITEMDELEGATE_H

#include "kmm_widgets_export.h"

#include <QStyledItemDelegate>

#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanbic/ibanbic.h"

class KMM_WIDGETS_EXPORT ibanBicItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ibanBicItemDelegate(QObject* parent = nullptr, const QVariantList& args = QVariantList());
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

Q_SIGNALS:
    void sizeHintChanged(const QModelIndex&) const;

private:
    inline payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBicByIndex(const QModelIndex& index) const;

};

#endif // IBANBICITEMDELEGATE_H
