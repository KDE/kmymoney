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

  // Use this->parent() as parent because "this" is const
  QAbstractItemDelegate* delegate = payeeIdentifierLoader::instance()->createItemDelegate(index.model()->data(index, payeeIdentifierModel::payeeIdentifierType).toString(), this->parent());

  if (delegate == 0) {
    if (defaultDelegate == 0)
      defaultDelegate = new QStyledItemDelegate( this->parent() );
    delegate = defaultDelegate;
  }

  connect( delegate, SIGNAL(sizeHintChanged(QModelIndex)), this, SLOT(sizeHintChangedSlot(QModelIndex)), Qt::UniqueConnection);
  return delegate;
}

void payeeIdentifierDelegate::sizeHintChangedSlot(const QModelIndex& index)
{
  //qDebug() << "sizeHintChanged for" << index.row() << index.column();
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
