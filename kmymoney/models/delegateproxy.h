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


#ifndef DELEGATEPROXY_H
#define DELEGATEPROXY_H

// ----------------------------------------------------------------------------
// Qt Includes

#include <QObject>
#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_models_export.h"

class DelegateProxyPrivate;

/**
 * Delegate Proxy object
 *
 * The delegate proxy object allows to assign a specific delegate in a view
 * depending on the model where the item is located. This becomes handy in case
 * the items shown are combined using e.g. a KConcatenateRowsProxyModel and
 * each base model should have its own delegate.
 */
class KMM_MODELS_EXPORT DelegateProxy : public QStyledItemDelegate
{
  Q_OBJECT
  Q_DISABLE_COPY(DelegateProxy)
public:
  /**
   * This method creates a new DelegateProxy as child of @a parent.
   */
  explicit DelegateProxy(QObject* parent = nullptr);

  /**
   * This method adds @a delegate as the delegate to be used when the item
   * to be shown/edited is located in @a model. The delegate and model are
   * still owned by the caller. They are not destroyed when the DelegateProxy
   * is destroyed.
   */
  void addDelegate(const QAbstractItemModel* model, QStyledItemDelegate* delegate);

  /**
   * Retrieve the delegate for objects stored in @a model. In case no
   * assignment exists, @c nullptr is returned.
   */
  const QStyledItemDelegate* delegate(const QAbstractItemModel* model) const;

  /**
   * This method returns a list of pointers to all registered delegates
   *
   * @return QList<QStyledItemDelegate> of all delegates
   */
  QList<QStyledItemDelegate*> delegateList() const;


  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
  void setEditorData(QWidget* editWidget, const QModelIndex& index) const final override;

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;


private:
  DelegateProxyPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(DelegateProxy);
};

#endif // DELEGATEPROXY_H
