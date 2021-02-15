/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LEDGERFILTERBASE_H
#define LEDGERFILTERBASE_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class LedgerFilterBasePrivate;
class KMM_MODELS_EXPORT LedgerFilterBase : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(LedgerFilterBase)
  Q_DISABLE_COPY(LedgerFilterBase)

public:
  enum GroupSortOrder {
    DateGrouping = 0,
    PayeeGrouping,
    JournalEntry,
    OnlineBalance,
  };

  virtual ~LedgerFilterBase();

  void setAccountType(eMyMoney::Account::Type type);

  void setFilterFixedString(const QString& filter);
  void setFilterFixedStrings(const QStringList& filters);

  QStringList filterFixedStrings() const;

  /**
   * This method returns the headerData adjusted to the current
   * accountType
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * This method returns the data adjusted to the current accountType
   */
  QVariant data(const QModelIndex& idx, int role) const override;

  void setShowEntryForNewTransaction(bool show);

  void setShowScheduledTransactions(bool show);

  /**
   * add @a model to the source models
   */
  void addSourceModel(QAbstractItemModel* model);

  /**
   * remove @a model from the source models
   */
  void removeSourceModel(QAbstractItemModel* model);

protected:
  LedgerFilterBasePrivate*  d_ptr;
  explicit LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent);

  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

  /**
   * @note This does not call the base class implementation for speed purposes
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

};

#endif // LEDGERFILTERBASE_H

