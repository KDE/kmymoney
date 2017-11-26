/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#ifndef PAYEEIDENTIFIERCONTAINERMODEL_H
#define PAYEEIDENTIFIERCONTAINERMODEL_H

#include <QModelIndex>
#include "mymoney/mymoneypayeeidentifiercontainer.h"
#include "payeeidentifier/payeeidentifier.h"

/**
 * @brief Model for MyMoneyPayeeIdentifierContainer
 *
 * Changes the user does have initernal effect only.
 */
class payeeIdentifierContainerModel : public QAbstractListModel
{
  Q_OBJECT

public:
  enum roles {
    payeeIdentifierType = Qt::UserRole, /**< type of payeeIdentifier */
    payeeIdentifier = Qt::UserRole + 1 /**< actual payeeIdentifier */
  };

  explicit payeeIdentifierContainerModel(QObject* parent = 0);

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

  /**
   * @brief Set source of data
   *
   * This makes the model editable.
   */
  void setSource(MyMoneyPayeeIdentifierContainer data);

  /** @brief Get stored data */
  QList< ::payeeIdentifier > identifiers() const;

public Q_SLOTS:
  /**
   * @brief Removes all data from the model
   *
   * The model is not editable afterwards.
   */
  void closeSource();

private:
  /** @internal
   * The use of a shared pointer makes this future prof. Because using identifier() causes
   * some unnecessary work.
   */
  QSharedPointer<MyMoneyPayeeIdentifierContainer> m_data;
};

#endif // PAYEEIDENTIFIERMODEL_H
