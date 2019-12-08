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

#ifndef SPLITMODEL_H
#define SPLITMODEL_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QUndoStack;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneysplit.h"

class KMM_MYMONEY_EXPORT SplitModel : public MyMoneyModel<MyMoneySplit>
{
  Q_OBJECT

public:
  enum Column {
    Category = 0,
    Memo,
    Amount,
    // insert new columns above this line
    MaxColumns
  };

  explicit SplitModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~SplitModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

  void appendSplit(const MyMoneySplit& s);

  static QString newSplitId();
  static bool isNewSplitId(const QString& id);

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SPLITMODEL_H

