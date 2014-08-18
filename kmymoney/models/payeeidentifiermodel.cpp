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

#include "payeeidentifiermodel.h"
#include "mymoney/mymoneyfile.h"
#include "payeeidentifier/payeeidentifierloader.h"
#include "payeeidentifier/payeeidentifier.h"

#include <QDebug>

#include <KLocalizedString>

payeeIdentifierModel::payeeIdentifierModel( QObject* parent )
  : QAbstractListModel(parent),
  m_data(0),
  m_loadedType(NONE)
{
}

QVariant payeeIdentifierModel::data(const QModelIndex& index, int role) const
{
  Q_CHECK_PTR( m_data );
  const ::payeeIdentifier ident = m_data->payeeIdentifiers().at(index.row());

  if ( role == payeeIdentifier) {
    return QVariant::fromValue< ::payeeIdentifier >(ident);
  } else if ( ident.isNull() ) {
    return QVariant();
  } else if ( role == payeeIdentifierType) {
    return ident.iid();
  } else if (role == Qt::DisplayRole) {
    // The custom delegates won't ask for this role
    return QVariant::fromValue( i18n("The plugin to show this information could not be found.") );
  }
  return QVariant();
}

/** @todo implement dataChanged signal */
bool payeeIdentifierModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  Q_CHECK_PTR( m_data );

  if ( role == payeeIdentifier ) {
    ::payeeIdentifier ident = value.value< ::payeeIdentifier >();
    m_data->modifyPayeeIdentifier(ident);
    const bool ret = saveCurrentObject();
    return ret;
  }
  return QAbstractItemModel::setData(index, value, role);
}

/** @todo make items editable if a plugin exists only */
Qt::ItemFlags payeeIdentifierModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
  const QString type = data(index, payeeIdentifierType).toString();
  // type.isNull() means the type selection can be shown
  if ( type.isNull() || payeeIdentifierLoader::instance()->hasItemEditDelegate(type) )
    flags |= Qt::ItemIsEditable;
  return flags;
}

int payeeIdentifierModel::rowCount(const QModelIndex& parent) const
{
  Q_CHECK_PTR( m_data );
  return m_data->payeeIdentifiers().count();
}

/** @todo implement dataChanged signal */
bool payeeIdentifierModel::insertRows(int row, int count, const QModelIndex& parent)
{
  Q_CHECK_PTR( m_data );
//  if ( row != m_payee.payeeIdentifiers().count()-1 || count != 1 || parent.isValid())
//    return false; // cannot add rows in the middle at the moment and only a single row

  beginInsertRows(parent, row+1, row+1);
  ::payeeIdentifier ident;
  m_data->addPayeeIdentifier( ident );
  const bool ret = saveCurrentObject();
  endInsertRows();
  return ret;
}

/** @todo implement dataChanged signal */
bool payeeIdentifierModel::removeRows(int row, int count, const QModelIndex& parent)
{
  Q_CHECK_PTR( m_data );
  if (count < 1)
    return false;

  beginRemoveRows(parent, row, row+count-1);
  for( int i = row; i < row+count; ++i) {
    m_data->removePayeeIdentifier(data(index(i, 0), payeeIdentifier).value< ::payeeIdentifier >());
  }
  const bool ret = saveCurrentObject();
  endRemoveRows();
  return ret;
}

void payeeIdentifierModel::setSource(MyMoneyPayee payee)
{
  m_payee = payee;
  m_loadedType = PAYEE;
  setSource( &m_payee );
}

void payeeIdentifierModel::setSource(MyMoneyAccount account)
{
  m_account = account;
  m_loadedType = ACCOUNT;
  setSource(&m_account);
}

void payeeIdentifierModel::setSource(MyMoneyPayeeIdentifierContainer* data)
{
  if ( m_data != 0 ) {
    // Remove all rows
    const int oldLastRow = m_data->payeeIdentifiers().count()-1;
    beginRemoveRows(QModelIndex(), 0, oldLastRow);
    emit dataChanged(index(0, 0), index(oldLastRow, 0));
    endRemoveRows();
  }

  // no need to delete data as it always points to 0, m_account or m_payee
  m_data = data;

  // Insert new rows
  const int newLastRow = m_data->payeeIdentifiers().count()-1;
  beginInsertRows(QModelIndex(), 0, newLastRow);
  emit dataChanged(index(0, 0), index(newLastRow, 0));
  endInsertRows();
}

bool payeeIdentifierModel::saveCurrentObject()
{
  Q_ASSERT( m_loadedType != NONE );
  try {
    MyMoneyFileTransaction transaction;

    if ( m_loadedType == PAYEE ) {
      MyMoneyFile::instance()->modifyPayee( m_payee );
    } else if ( m_loadedType == ACCOUNT ) {
      MyMoneyFile::instance()->modifyAccount( m_account );
    }

    transaction.commit();
  } catch ( MyMoneyException& ) {
    return false;
  }
  return true;
}
