/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    Payment,
    Deposit,
    // insert new columns above this line
    MaxColumns,
  };

  explicit SplitModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  SplitModel(QObject* parent, QUndoStack* undoStack, const SplitModel& right);
  virtual ~SplitModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

  bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;

  void appendSplit(const MyMoneySplit& s);
  void appendEmptySplit();
  void removeEmptySplit();

  // Reimplemented for internal reasons
  void doRemoveItem(const MyMoneySplit& before) override;
  void doAddItem(const MyMoneySplit& item, const QModelIndex& parentIdx = QModelIndex()) override;

  static QString newSplitId();
  static bool isNewSplitId(const QString& id);

  SplitModel& operator= (const SplitModel& right);

  MyMoneyMoney valueSum() const;

Q_SIGNALS:
  void itemCountChanged(int cnt);

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SPLITMODEL_H

