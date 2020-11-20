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

#ifndef LEDGERFILTER_H
#define LEDGERFILTER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class LedgerFilterPrivate;
class KMM_MODELS_EXPORT LedgerFilter : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(LedgerFilter)
  Q_DISABLE_COPY(LedgerFilter)

public:
  explicit LedgerFilter(QObject* parent);

  void setStateFilter(int state);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  LedgerFilterPrivate*  d_ptr;
};

#endif // LEDGERFILTER_H

