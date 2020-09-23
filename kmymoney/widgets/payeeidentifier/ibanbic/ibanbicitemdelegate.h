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

#ifndef IBANBICITEMDELEGATE_H
#define IBANBICITEMDELEGATE_H

#include "kmm_base_widgets_export.h"

#include <QStyledItemDelegate>

#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanbic/ibanbic.h"

class KMM_BASE_WIDGETS_EXPORT ibanBicItemDelegate : public QStyledItemDelegate
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
