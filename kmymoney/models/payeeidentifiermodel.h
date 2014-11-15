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
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneypayee.h"

/**
 * @warning You must set the source with setSource() before you call any other method.
 */
class payeeIdentifierModel : public QAbstractListModel
{
  Q_OBJECT

public:
  enum roles {
    payeeIdentifierType = Qt::UserRole, /**< type of payeeIdentifier */
    payeeIdentifier = Qt::UserRole+1 /**< actual payeeIdentifier */
  };

  payeeIdentifierModel( QObject* parent = 0 );

  virtual QVariant data(const QModelIndex& index, int role) const;

  /**
   * This model only supports to edit payeeIdentifier role with a QVariant of type
   * payeeIdentifier.
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual int rowCount(const QModelIndex& parent) const;

  virtual bool insertRows(int row, int count, const QModelIndex& parent);
  virtual bool removeRows(int row, int count, const QModelIndex& parent);

  void setSource( MyMoneyPayee payee );
  // void setSource( MyMoneyAccount account );

private:
  void setSource( MyMoneyPayeeIdentifierContainer* data );

  MyMoneyPayeeIdentifierContainer* m_data;

  /**
   * When data is read or written to MyMoneyFile the model has to call
   * modifyAccount or modifyPayee. This made this a bit complicated.
   * m_data always points to the current and correct object.
   *
   * When ever possible use m_data!
   */
  bool saveCurrentObject();
  /** @brief Store which data source is loaded */
  enum {NONE, PAYEE, ACCOUNT} m_loadedType;
  /** Contains the payee if this is the current type */
  MyMoneyPayee m_payee;
  /** Cointains the account if this is the current type */
  MyMoneyAccount m_account;
};

#endif // PAYEEIDENTIFIERMODEL_H
