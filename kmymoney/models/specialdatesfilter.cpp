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

#include "specialdatesfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "specialdatesmodel.h"
#include "mymoneyfile.h"

using namespace eMyMoney;

class SpecialDatesFilterPrivate
{
public:
  SpecialDatesFilterPrivate(QAbstractItemModel* model)
    : specialDatesModel(model)
  {
  }

  inline bool isSpecialDatesModel(const QAbstractItemModel* model) const
  {
    return (model == specialDatesModel);
  }

  QAbstractItemModel*   specialDatesModel;
};


SpecialDatesFilter::SpecialDatesFilter(const QAbstractItemModel* datesModel, QObject* parent)
  : QSortFilterProxyModel(parent)
  , d_ptr(new SpecialDatesFilterPrivate(const_cast<QAbstractItemModel*>(datesModel)))
{
  Q_D(SpecialDatesFilter);
}

bool SpecialDatesFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const SpecialDatesFilter);

  QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
  const auto baseModel = MyMoneyModelBase::baseModel(idx);
  if (d->isSpecialDatesModel(baseModel)) {
    // make sure we don't show trailing special date entries
    const auto rows = sourceModel()->rowCount(source_parent);
    int row = source_row + 1;
    bool visible = false;
    QModelIndex testIdx;
    for (; !visible && row < rows; ++row) {
      testIdx = sourceModel()->index(row, 0, source_parent);
      if(testIdx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        // the empty id is the entry for the new transaction entry
        // we're done scanning
        break;
      }
      const auto testModel = MyMoneyModelBase::baseModel(testIdx);
      if (!d->isSpecialDatesModel(testModel)) {
        // we did not hit a special date entry
        // now we need to check for a real transaction or the online balance one
        if (!testIdx.data(eMyMoney::Model::JournalTransactionIdRole).toString().isEmpty()) {
          visible = true;
        }
        break;
      }
    }

    // in case this is not a trailing date entry, we need to check
    // if it is the last of a row of date entries.
    if (visible && ((source_row + 1) < rows)) {
      // check if the next is also a date entry
      testIdx = sourceModel()->index(source_row+1, 0, source_parent);
      const auto testModel = MyMoneyModelBase::baseModel(testIdx);
      if (d->isSpecialDatesModel(testModel)) {
        visible = false;
      }
    }
    return visible;
  }
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
