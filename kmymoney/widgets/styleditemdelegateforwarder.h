/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  explicit StyledItemDelegateForwarder(QObject* parent = 0);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

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
