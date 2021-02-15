/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECIALDATESFILTER_H
#define SPECIALDATESFILTER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class SpecialDatesFilterPrivate;
class KMM_MODELS_EXPORT SpecialDatesFilter : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(SpecialDatesFilter)
  Q_DISABLE_COPY(SpecialDatesFilter)

public:
  explicit SpecialDatesFilter(const QAbstractItemModel* specialDatesModel, QObject* parent);

public Q_SLOTS:
  void forceReload();

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  SpecialDatesFilterPrivate*  d_ptr;
};

#endif // SPECIALDATESFILTER_H

