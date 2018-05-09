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

#ifndef IBANBICITEMDELEGATE_H
#define IBANBICITEMDELEGATE_H

#include "payeeidentifier_iban_bic_widgets_export.h"

#include <QStyledItemDelegate>
#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanandbic/ibanbic.h"

class PAYEEIDENTIFIER_IBAN_BIC_WIDGETS_EXPORT ibanBicItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.payeeIdentifier.ibanbic.delegate" FILE "kmymoney-ibanbic-delegate.json")

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
