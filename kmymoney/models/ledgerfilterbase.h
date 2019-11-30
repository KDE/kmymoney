/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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
   * Currently filtering only on account id provided by @sa setAccount()
   *
   * @note This does not call the base class implementation for speed purposes
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

};

#endif // LEDGERFILTERBASE_H

