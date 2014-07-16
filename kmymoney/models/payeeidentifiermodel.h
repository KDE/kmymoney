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

#ifndef PAYEEIDENTIFIERMODEL_H
#define PAYEEIDENTIFIERMODEL_H

#include <QtCore/QModelIndex>
#include "mymoneypayee.h"

class payeeIdentifierModel : public QAbstractListModel
{
  Q_OBJECT

public:
  enum roles {
    payeeIdentifierType = Qt::UserRole, /**< type of payeeIdentifier */
    payeeIdentifierPtr = Qt::UserRole+1 /**< Pointer to acual payeeIdentifier */
  };

  payeeIdentifierModel( QObject* parent = 0 );

  /**
   * Returend QVariants for payeeIdentifierPtr are of type payeeIdentifier::constPtr
   */
  virtual QVariant data(const QModelIndex& index, int role) const;

  /**
   * This model only supports to edit payeeIdentifierPtr role with a QVariant of type
   * payeeIdentifier::ptr. I take ownership of the pointer, modifing it later leads to
   * undefined behaviour.
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  
  virtual int rowCount(const QModelIndex& parent) const;

  virtual bool insertRows(int row, int count, const QModelIndex& parent);
  virtual bool removeRows(int row, int count, const QModelIndex& parent);

  void setPayee( MyMoneyPayee payee );
  void setPayee( QString payeeId );

private:
  MyMoneyPayee m_payee;
};

#endif // PAYEEIDENTIFIERMODEL_H
