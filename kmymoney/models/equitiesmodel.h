/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef EQUITIESMODEL_H
#define EQUITIESMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KExtraColumnsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

class EquitiesModelPrivate;
class KMM_MODELS_EXPORT EquitiesModel : public KExtraColumnsProxyModel
{
  Q_OBJECT
public:
  enum Column {
    Symbol = 0,
    Quantity,
    Price,
    Value,
  };

  EquitiesModel(QObject *parent = nullptr);
  ~EquitiesModel();

  QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role = Qt::DisplayRole) const override;

public Q_SLOTS:

private:
  void init();
  void load();

private:
  EquitiesModelPrivate* d_ptr;
  Q_DECLARE_PRIVATE(EquitiesModel);
};

#endif // EQUITIESMODEL_H
