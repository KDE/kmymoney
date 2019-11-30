/*
 * Copyright 2016-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractTableModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class SplitModelPrivate;
class MyMoneyTransaction;
class MyMoneySplit;
class KMM_MODELS_EXPORT SplitModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SplitModel(QObject* parent = nullptr);
  virtual ~SplitModel();
  void deepCopy(const SplitModel& right, bool revertSplitSign = false);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Adds a single split @a s of transaction @a t to the model
   */
  void addSplit(const MyMoneyTransaction& t, const MyMoneySplit& s);

  /**
   * Adds a single dummy split to the model which is used for
   * creation of new splits.
   */
  void addEmptySplitEntry();

  /**
   * Remove the single dummy split to the model which is used for
   * creation of new splits from the model.
   */
  void removeEmptySplitEntry();

  bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  /**
   * This method returns the string to be used for new split ids
   */
  static QString newSplitId();

  /**
   * This method compares the @a id against the one provided
   * by newSplitId() and returns true if it is identical.
   */
  static bool isNewSplitId(const QString& id);

  // void removeSplit(const LedgerTransaction& t);

private:
  Q_DISABLE_COPY(SplitModel)
  Q_DECLARE_PRIVATE(SplitModel)
  const QScopedPointer<SplitModelPrivate> d_ptr;

};
#endif // SPLITMODEL_H

