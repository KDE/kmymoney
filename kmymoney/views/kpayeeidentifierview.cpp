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

#include "kpayeeidentifierview.h"
#include "ui_kpayeeidentifierview.h"

#include <QStyledItemDelegate>
#include <QDebug>
#include <QTimer>

#include "payeeidentifier/payeeidentifierloader.h"
#include "payeeidentifiermodel.h"

payeeIdentifierDelegate::payeeIdentifierDelegate(QObject* parent)
  : StyledItemDelegateForwarder(parent)
{

}

QAbstractItemDelegate* payeeIdentifierDelegate::getItemDelegate(const QModelIndex& index) const
{
  static QAbstractItemDelegate* defaultDelegate = 0;
  const QString type = index.model()->data(index, payeeIdentifierModel::payeeIdentifierType).toString();

  if ( type.isNull() ) {
    QAbstractItemDelegate* delegate = payeeIdentifierLoader::instance()->createItemDelegate("org.kmymoney.payeeIdentifier.empty", this->parent());
    connectSignals(delegate);
    return delegate;
  }

  // Use this->parent() as parent because "this" is const
  QAbstractItemDelegate* delegate = payeeIdentifierLoader::instance()->createItemDelegate(type, this->parent());

  if (delegate == 0) {
    if (defaultDelegate == 0)
      defaultDelegate = new QStyledItemDelegate( this->parent() );
    delegate = defaultDelegate;
  }
  connectSignals(delegate, Qt::UniqueConnection);
  return delegate;
}

KPayeeIdentifierView::KPayeeIdentifierView(QWidget* parent)
  : QWidget(parent)
{
  ui = new Ui::KPayeeIdentifierView;
  ui->setupUi(this);
  ui->view->setItemDelegate( new payeeIdentifierDelegate(ui->view) );

}

void KPayeeIdentifierView::setPayee(MyMoneyPayee payee)
{
  if ( ui->view->model() == 0 ) {
    payeeIdentifierModel* model = new payeeIdentifierModel( ui->view );
    ui->view->setModel( model );
  }

  Q_CHECK_PTR( qobject_cast<payeeIdentifierModel*>(ui->view->model()) );  // this should never fail but may help during debugging
  static_cast<payeeIdentifierModel*>(ui->view->model())->setPayee(payee);
}

/**
 * @param index not used at the moment, a new item is always inserted at the end
 */
void KPayeeIdentifierView::addEntry(const QModelIndex& index)
{
  Q_UNUSED( index );
  if ( ui->view->model()->insertRow(ui->view->model()->rowCount()) ) {
    QModelIndex index = ui->view->model()->index(ui->view->model()->rowCount()-1, 0);
    ui->view->setCurrentIndex( index );
    ui->view->openPersistentEditor(index);
  }
}

void KPayeeIdentifierView::removeSelected()
{
  QModelIndexList selectedRows = ui->view->selectionModel()->selectedRows();
  QAbstractItemModel *const model = ui->view->model();
  Q_CHECK_PTR( model );

  Q_FOREACH( QModelIndex row, selectedRows ) {
    model->removeRow(row.row());
  }
}
