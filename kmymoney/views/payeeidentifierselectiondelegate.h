/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
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
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
};

class payeeIdentifierTypeSelectionWidget : public QComboBox
{
  Q_OBJECT
public:
  explicit payeeIdentifierTypeSelectionWidget(QWidget* parent = 0);

Q_SIGNALS:
  void commitData(QWidget* editor);

private Q_SLOTS:
  void itemSelected(int index);
};

#endif // PAYEEIDENTIFIERSELECTIONDELEGATE_H
