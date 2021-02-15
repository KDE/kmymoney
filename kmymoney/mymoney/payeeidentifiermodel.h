/*
    SPDX-FileCopyrightText: 2015-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  QVariant data(const QModelIndex& index, int role) const final override;
  int columnCount(const QModelIndex& parent) const final override;
  int rowCount(const QModelIndex& parent) const final override;
  QModelIndex parent(const QModelIndex& child) const final override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const final override;
  Qt::ItemFlags flags(const QModelIndex &index) const final override;

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
