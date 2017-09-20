/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STYLEDITEMDELEGATEFORWARDER_H
#define STYLEDITEMDELEGATEFORWARDER_H

#include <QAbstractItemDelegate>

#include "kmm_widgets_export.h"

/**
 * @brief Helper to use multiple item delegates in a view
 *
 * This class allows to select the used item delegate based on the QModelIndex.
 *
 */
class KMM_WIDGETS_EXPORT StyledItemDelegateForwarder : public QAbstractItemDelegate
{
  Q_OBJECT

public:
  StyledItemDelegateForwarder(QObject* parent = 0);
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * @brief Return delegate for a given index
   *
   * If an method of this class is called, it uses this function to receive
   * the correct delegate where the call is forwarded to.
   *
   * @return You must return a valid item delegate.
   * @see connectSignals()
   */
  virtual QAbstractItemDelegate* getItemDelegate(const QModelIndex& index) const = 0;

protected:
  /**
   * @brief Connects all signals accordingly
   *
   * Call this function if you create a new delegate in getItemDelegate().
   */
  void connectSignals(QAbstractItemDelegate* delegate, Qt::ConnectionType type = Qt::AutoConnection) const;

};

#endif // STYLEDITEMDELEGATEFORWARDER_H
