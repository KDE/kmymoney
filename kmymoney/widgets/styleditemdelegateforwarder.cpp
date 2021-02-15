/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "styleditemdelegateforwarder.h"

StyledItemDelegateForwarder::StyledItemDelegateForwarder(QObject* parent)
    : QAbstractItemDelegate(parent)
{
}

void StyledItemDelegateForwarder::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  getItemDelegate(index)->paint(painter, option, index);
}

QSize StyledItemDelegateForwarder::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QAbstractItemDelegate* delegate = getItemDelegate(index);
  Q_CHECK_PTR(delegate);
  return delegate->sizeHint(option, index);
}

QWidget* StyledItemDelegateForwarder::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  return getItemDelegate(index)->createEditor(parent, option, index);
}

void StyledItemDelegateForwarder::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  getItemDelegate(index)->setEditorData(editor, index);
}

void StyledItemDelegateForwarder::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  getItemDelegate(index)->setModelData(editor, model, index);
}

void StyledItemDelegateForwarder::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  getItemDelegate(index)->updateEditorGeometry(editor, option, index);
}

void StyledItemDelegateForwarder::connectSignals(QAbstractItemDelegate* delegate, Qt::ConnectionType type) const
{
  connect(delegate, &QAbstractItemDelegate::commitData, this, &QAbstractItemDelegate::commitData, type);
  connect(delegate, &QAbstractItemDelegate::closeEditor, this, &QAbstractItemDelegate::closeEditor, type);
  connect(delegate, &QAbstractItemDelegate::sizeHintChanged, this, &QAbstractItemDelegate::sizeHintChanged, type);
}
