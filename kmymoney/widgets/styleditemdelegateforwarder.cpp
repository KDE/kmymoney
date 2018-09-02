/*
 * Copyright 2014-2016  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
