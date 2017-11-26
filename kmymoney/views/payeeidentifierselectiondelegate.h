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

#ifndef PAYEEIDENTIFIERSELECTIONDELEGATE_H
#define PAYEEIDENTIFIERSELECTIONDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>

class payeeIdentifierSelectionDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit payeeIdentifierSelectionDelegate(QObject* parent = 0);
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class payeeIdentifierTypeSelectionWidget : public QComboBox
{
  Q_OBJECT
public:
  explicit payeeIdentifierTypeSelectionWidget(QWidget* parent = 0);

signals:
  void commitData(QWidget* editor);

private slots:
  void itemSelected(int index);
};

#endif // PAYEEIDENTIFIERSELECTIONDELEGATE_H
