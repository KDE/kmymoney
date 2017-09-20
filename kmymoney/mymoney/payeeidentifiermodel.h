/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015  Christian David <christian-david@web.de>
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

#include "kmm_mymoney_export.h"

#include <QAbstractItemModel>
#include <QStringList>

#include "mymoney/mymoneypayee.h"

/**
 * @brief Read-only model for stored payeeIdentifiers
 *
 * @note You must set an filter
 *
 * @internal if needed this can be extended to an read/write model
 */
class KMM_MYMONEY_EXPORT payeeIdentifierModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum roles {
    payeeName = Qt::UserRole, /**< MyMoneyPayee::name() */
    isPayeeIdentifier = Qt::UserRole + 1, /**< refers index to payeeIdentifier (true) or MyMoneyPayee (false) */
    payeeIdentifierType = Qt::UserRole + 2, /**< type of payeeIdentifier */
    payeeIdentifier = Qt::UserRole + 3, /**< actual payeeIdentifier */
    payeeIdentifierUserRole = Qt::UserRole + 4 /**< role to start with for inheriting models */
  };

  explicit payeeIdentifierModel(QObject* parent = 0);
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual int columnCount(const QModelIndex& parent) const;
  virtual int rowCount(const QModelIndex& parent) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  /**
   * @brief Set which payeeIdentifier types to show
   *
   * @param filter list of payeeIdentifier types. An empty list leads to an empty model.
   */
  void setTypeFilter(QStringList filter);

  /** convenience overload for setTypeFilter(QStringList) */
  void setTypeFilter(QString type);

  void loadData();

private:
  typedef QPair<QString, int> identId_t;
  inline MyMoneyPayee payeeByIndex(const QModelIndex& index) const;
  QStringList m_payeeIdentifierIds;
  QStringList m_typeFilter;
};

#endif // PAYEEIDENTIFIERMODEL_H
