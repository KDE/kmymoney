/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
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
