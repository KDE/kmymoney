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

  if ( index.row() == rowCount(index.parent())-1 )
    return QVariant();

  const ::payeeIdentifier ident = m_data->payeeIdentifiers().at(index.row());

  if (role == payeeIdentifier) {
    return QVariant::fromValue< ::payeeIdentifier >(ident);
  } else if (ident.isNull()) {
    return QVariant();
  } else if (role == payeeIdentifierType) {
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
    if ( index.row() == rowCount(index.parent())-1 ) {
      // The new row will be the last but one
      beginInsertRows(index.parent(), index.row()-1, index.row()-1);
      m_data->addPayeeIdentifier(ident);
      endInsertRows();
    } else {
      m_data->modifyPayeeIdentifier(index.row(), ident);
    }
    const bool ret = saveCurrentObject();
    return ret;
  }
  return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags payeeIdentifierModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
  const QString type = data(index, payeeIdentifierType).toString();
  // type.isEmpty() means the type selection can be shown
  if ( type.isEmpty() || payeeIdentifierLoader::instance()->hasItemEditDelegate(type) )
    flags |= Qt::ItemIsEditable;
  return flags;
}

int payeeIdentifierModel::rowCount(const QModelIndex& parent) const
{
  Q_CHECK_PTR( m_data );
  // Always a row more which creates new entries
  return m_data->payeeIdentifiers().count()+1;
}

/** @brief unused at the moment */
bool payeeIdentifierModel::insertRows(int row, int count, const QModelIndex& parent)
{
  Q_CHECK_PTR( m_data );
  return false;
}

bool payeeIdentifierModel::removeRows(int row, int count, const QModelIndex& parent)
{
  Q_CHECK_PTR( m_data );
  if (count < 1 || row+count >= rowCount(parent))
    return false;

  beginRemoveRows(parent, row, row+count-1);
  for( unsigned int i = row; i < row+count; ++i) {
    m_data->removePayeeIdentifier(i);
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

#if 0
void payeeIdentifierModel::setSource(MyMoneyAccount account)
{
  m_account = account;
  m_loadedType = ACCOUNT;
  setSource(&m_account);
}
#endif

void payeeIdentifierModel::setSource(MyMoneyPayeeIdentifierContainer* data)
{
  if ( m_data != 0 ) {
    // Remove all rows
    const int oldLastRow = m_data->payeeIdentifiers().count()-1;
    beginRemoveRows(QModelIndex(), 0, oldLastRow);
    endRemoveRows();
  }

  // no need to delete data as it always points to 0, m_account or m_payee
  m_data = data;

  // Insert new rows
  const int newLastRow = m_data->payeeIdentifiers().count()-1;
  beginInsertRows(QModelIndex(), 0, newLastRow);
  endInsertRows();
}

bool payeeIdentifierModel::saveCurrentObject()
{
  Q_ASSERT( m_loadedType != NONE );
  try {
    MyMoneyFileTransaction transaction;

    if ( m_loadedType == PAYEE )
      MyMoneyFile::instance()->modifyPayee( m_payee );
    else if ( m_loadedType == ACCOUNT )
      MyMoneyFile::instance()->modifyAccount( m_account );
    else
      return false;

    transaction.commit();
  } catch ( MyMoneyException& e ) {
    qWarning() << e.what() << e.file() << e.line();
    return false;
  }
  return true;
}
