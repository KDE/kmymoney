/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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


#include "delegateproxy.h"

// ----------------------------------------------------------------------------
// Qt Includes

#include <QDebug>
#include <QHash>
#include <QSortFilterProxyModel>
class QAbstractItemDelegate;
class QAbstractItemModel;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymodelbase.h>

class DelegateProxyPrivate
{
  Q_DISABLE_COPY(DelegateProxyPrivate)
  Q_DECLARE_PUBLIC(DelegateProxy)

public:
  explicit DelegateProxyPrivate(DelegateProxy *qq)
    : q_ptr(qq)
  {
  }

  const QStyledItemDelegate* findDelegate(const QModelIndex& idx) const
  {
    // create an index for column 0
    const QModelIndex baseIdx = idx.model()->index(idx.row(), 0, idx.parent());
    const QAbstractItemModel* model = MyMoneyModelBase::baseModel(baseIdx);
    return mapping.value(model, nullptr);
  }

  DelegateProxy      *q_ptr;
  QHash<const QAbstractItemModel*, const QStyledItemDelegate*>  mapping;
};


DelegateProxy::DelegateProxy(QObject* parent)
: QStyledItemDelegate(parent)
  , d_ptr(new DelegateProxyPrivate(this))
{
}

void DelegateProxy::addDelegate(const QAbstractItemModel* model, QStyledItemDelegate* delegate)
{
  Q_D(DelegateProxy);
  if (model == nullptr) {
    return;
  }

  // in case we have a mapping for the model, we remove it
  if (d->mapping.contains(model)) {
    d->mapping.remove(model);
  }
  if (delegate) {
    d->mapping[model] = delegate;
  }
}

const QStyledItemDelegate * DelegateProxy::delegate(const QAbstractItemModel* model) const
{
  if (model != nullptr) {
    Q_D(const DelegateProxy);
    return d->mapping.value(model, nullptr);
  }
  return nullptr;
}

void DelegateProxy::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    delegate->paint(painter, option, index);
  }
}

QSize DelegateProxy::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    return delegate->sizeHint(option, index);
  }
  return QSize();
}

QWidget* DelegateProxy::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    return delegate->createEditor(parent, option, index);
  }
  return nullptr;
}

void DelegateProxy::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    delegate->updateEditorGeometry(editor, option, index);
  }
}

void DelegateProxy::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    delegate->setEditorData(editWidget, index);
  }
}

void DelegateProxy::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_D(const DelegateProxy);
  const auto delegate = d->findDelegate(index);
  if (delegate) {
    delegate->setModelData(editor, model, index);
  }
}
