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
#include "payeeidentifierselectiondelegate.h"

payeeIdentifierDelegate::payeeIdentifierDelegate(QObject* parent)
  : StyledItemDelegateForwarder(parent)
{

}

QAbstractItemDelegate* payeeIdentifierDelegate::getItemDelegate(const QModelIndex& index) const
{
  static QAbstractItemDelegate* defaultDelegate = 0;
  const QString type = (index.isValid()) ? index.model()->data(index, payeeIdentifierModel::payeeIdentifierType).toString() : QString();

  if ( type.isEmpty() ) {
    QAbstractItemDelegate* delegate = new payeeIdentifierSelectionDelegate(this->parent());
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
  static_cast<payeeIdentifierModel*>(ui->view->model())->setSource(payee);

  // Open persistent editor for last row
  ui->view->openPersistentEditor(ui->view->model()->index( ui->view->model()->rowCount(QModelIndex())-1, 0 ));
}

/**
 * @brief Helper to sort QModelIndexList in decreasing order.
 */
inline bool QModelIndexRowComparison(const QModelIndex& first, const QModelIndex& second)
{
  return (first.row() > second.row());
}

/**
 * @bug If the last row is removed the type selection editor (which is always behind that last row) closes.
 * Maybe that is a Qt bug?!
 */
void KPayeeIdentifierView::removeSelected()
{
  QModelIndexList selectedRows = ui->view->selectionModel()->selectedRows();
  // To keep the items valid during remove the data must be removed from highest row
  // to the lowes. Unfortunately QList has no reverse iterator.
  std::sort(selectedRows.begin(), selectedRows.end(), QModelIndexRowComparison);

  QAbstractItemModel* model = ui->view->model();
  Q_CHECK_PTR( model );

  QModelIndexList::const_iterator end = selectedRows.constEnd();
  for(QModelIndexList::const_iterator iter = selectedRows.constBegin(); iter != end; ++iter)
    model->removeRow(iter->row(), iter->parent());
}
