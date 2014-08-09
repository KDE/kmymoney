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

#include "payeeidentifierselectiondelegate.h"

#include <KLocalizedString>

#include "payeeidentifier/payeeidentifierloader.h"
#include "models/payeeidentifiermodel.h"


payeeIdentifierTypeSelectionWidget::payeeIdentifierTypeSelectionWidget(QWidget* parent)
  : QComboBox(parent)
{
  connect(this, SIGNAL(activated(int)), this, SLOT(itemSelected(int)));
}

void payeeIdentifierTypeSelectionWidget::itemSelected(int index)
{
  if (index != 0) {
    emit commitData(this);
    deleteLater(); // this closes the editor
  }
}

payeeIdentifierSelectionDelegate::payeeIdentifierSelectionDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* payeeIdentifierSelectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  payeeIdentifierTypeSelectionWidget* comboBox = new payeeIdentifierTypeSelectionWidget(parent);
  comboBox->setFrame(false);
  connect( comboBox, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));

  comboBox->addItem(i18n("Please select the account number type"));
  payeeIdentifierLoader *const loader = payeeIdentifierLoader::instance();
  QStringList pidids = loader->availableDelegates();
  Q_FOREACH(QString pidid, pidids) {
    comboBox->addItem(loader->translatedDelegateName(pidid), QVariant( pidid ));
  }
  return comboBox;
}

void payeeIdentifierSelectionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QComboBox *const comboBox = qobject_cast<QComboBox*>(editor);
  const QString selectedPidid = comboBox->model()->data(comboBox->model()->index(comboBox->currentIndex(), 0 ), Qt::UserRole).toString();
  payeeIdentifier::ptr ident = payeeIdentifierLoader::instance()->createPayeeIdentifier(selectedPidid);
  model->setData(index, QVariant::fromValue<payeeIdentifier::ptr>(ident), payeeIdentifierModel::payeeIdentifierPtr);
}

void payeeIdentifierSelectionDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
  editor->setGeometry(option.rect);
}

QSize payeeIdentifierSelectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
