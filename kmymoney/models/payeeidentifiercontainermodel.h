/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#include "kmm_models_export.h"

#include <QAbstractListModel>
#include <QSharedPointer>

#include "mymoney/payeeidentifiermodel.h"
#include "mymoney/mymoneypayeeidentifiercontainer.h"
#include "payeeidentifier/payeeidentifier.h"

/**
 * @brief Model for MyMoneyPayeeIdentifierContainer
 *
 * Changes the user does have internal effect only.
 *
 * @see payeeIdentifierModel
 */
class MyMoneyPayeeIdentifierContainer;
class payeeIdentifier;
class KMM_MODELS_EXPORT payeeIdentifierContainerModel : public QAbstractListModel
{
  Q_OBJECT

public:
  /**
   * @brief Roles for this model
   *
   * They are equal to payeeIdentifierModel::roles
   */
  enum roles {
    payeeIdentifierType = payeeIdentifierModel::payeeIdentifierType, /**< type of payeeIdentifier */
    payeeIdentifier = payeeIdentifierModel::payeeIdentifier /**< actual payeeIdentifier */
  };

  explicit payeeIdentifierContainerModel(QObject* parent = 0);

  QVariant data(const QModelIndex& index, int role) const final override;

  /**
   * This model only supports to edit payeeIdentifier role with a QVariant of type
   * payeeIdentifier.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  Qt::ItemFlags flags(const QModelIndex& index) const final override;

  int rowCount(const QModelIndex& parent) const final override;

  bool insertRows(int row, int count, const QModelIndex& parent) final override;
  bool removeRows(int row, int count, const QModelIndex& parent) final override;

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

#endif // PAYEEIDENTIFIERCONTAINERMODEL_H
