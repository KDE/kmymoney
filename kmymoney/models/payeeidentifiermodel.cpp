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
#include <payeeidentifier/ibanandbic/ibanbic.h>

#include <QDebug>

payeeIdentifierModel::payeeIdentifierModel( QObject* parent )
  : QAbstractListModel(parent),
  m_payee( MyMoneyPayee() )
{
}

QVariant payeeIdentifierModel::data(const QModelIndex& index, int role) const
{
  payeeIdentifier::constPtr ident = m_payee.payeeIdentifiers().values().at(index.row());

  if ( role == payeeIdentifierPtr) {
    return QVariant::fromValue<payeeIdentifier::constPtr>(ident);
  } else if ( ident.isNull() ) {
    return QVariant();
  } else if ( role == payeeIdentifierType || role == Qt::DisplayRole) {
    return ident->payeeIdentifierId();
  }
  return QVariant();
}

bool payeeIdentifierModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if ( role == payeeIdentifierPtr ) {
    payeeIdentifier::ptr ident = value.value<payeeIdentifier::ptr>();
    if ( !ident.isNull() ) {
      m_payee.modifyPayeeIdentifier(index.row(), ident);
      try {
        MyMoneyFileTransaction transaction;
        MyMoneyFile *const file = MyMoneyFile::instance();
        file->modifyPayee(m_payee);
        transaction.commit();
        m_payee = file->payee(m_payee.id());
      } catch ( MyMoneyException& e ) {
        qDebug( "payeeIdentifierModel could not edit payee" );
        return false;
      }
      return true;
    }
    return false;
  }
  return QAbstractItemModel::setData(index, value, role);
}

/** @todo make items editable if a plugin exists only */
Qt::ItemFlags payeeIdentifierModel::flags(const QModelIndex& index) const
{
  return (QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);
}

int payeeIdentifierModel::rowCount(const QModelIndex& parent) const
{
  return m_payee.payeeIdentifiers().count();
}

bool payeeIdentifierModel::insertRows(int row, int count, const QModelIndex& parent)
{
  return false;
}

bool payeeIdentifierModel::removeRows(int row, int count, const QModelIndex& parent)
{
  return false;
}

void payeeIdentifierModel::setPayee(MyMoneyPayee payee)
{
  m_payee = payee;
  /** @todo emit signals after change of payee */
}

void payeeIdentifierModel::setPayee(QString payeeId)
{
  try {
    setPayee( MyMoneyFile::instance()->payee( payeeId ) );
  } catch ( MyMoneyException& ) {
    qWarning("Tried to load non-existent payee into payeeIdentifierModel");
  }
}

#include "../models/payeeidentifiermodel.moc"
