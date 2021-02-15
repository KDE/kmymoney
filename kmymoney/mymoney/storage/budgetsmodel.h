/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BUDGETSMODEL_H
#define BUDGETSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneybudget.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT BudgetsModel : public MyMoneyModel<MyMoneyBudget>
{
  Q_OBJECT

public:
  enum Columns {
      Name,
      Year
  };

  explicit BudgetsModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  ~BudgetsModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // BUDGETSMODEL_H

