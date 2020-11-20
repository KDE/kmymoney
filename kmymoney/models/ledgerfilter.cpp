/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#include "ledgerfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class LedgerFilterPrivate
{
public:
  LedgerFilterPrivate()
    : state(-1)
  {
  }

  int           state;
};


LedgerFilter::LedgerFilter(QObject* parent)
  : QSortFilterProxyModel(parent)
  , d_ptr(new LedgerFilterPrivate)
{
}

bool LedgerFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const LedgerFilter);

  if (d->state != -1) {
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    switch(d->state) {
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void LedgerFilter::setStateFilter(int state)
{
  Q_D(LedgerFilter);
  d->state = state;
  invalidateFilter();
}
