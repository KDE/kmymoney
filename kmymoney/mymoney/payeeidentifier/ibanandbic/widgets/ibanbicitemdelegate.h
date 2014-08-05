/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include <QStyledItemDelegate>
#include "payeeidentifier/ibanandbic/ibanbic.h"

#include "../ibanbicmacros.h"

class IBAN_BIC_IDENTIFIER_EXPORT ibanBicItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  ibanBicItemDelegate(QObject* parent);
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

signals:
  void sizeHintChanged( const QModelIndex& ) const;

private:
  inline payeeIdentifiers::ibanBic::constPtr ibanBicByIndex( const QModelIndex& index ) const;

};

#endif // IBANBICITEMDELEGATE_H
