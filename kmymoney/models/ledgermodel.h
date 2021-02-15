/*
 * SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LEDGERMODEL_H
#define LEDGERMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractTableModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObject;
class MyMoneySchedule;
class MyMoneySplit;
class MyMoneyTransaction;
class LedgerTransaction;

namespace eMyMoney { namespace File { enum class Object; } }

class LedgerModelPrivate;
class KMM_MODELS_EXPORT LedgerModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit LedgerModel(QObject* parent = nullptr);
  virtual ~LedgerModel();

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  /**
   * clears all objects currently in the model
   */
  void unload();

  /**
   * Adds the transaction items in the @a list to the model
   */
  void addTransactions(const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list);

  /**
   * Adds a single transaction @a t to the model
   */
  void addTransaction(const LedgerTransaction& t);

  /**
   * Adds a single split based on its transactionSplitId
   */
  void addTransaction(const QString& transactionSplitId);

  /**
   * Adds the schedule items in the  @a list to the model
   */
  void addSchedules(const QList< MyMoneySchedule >& list, int previewPeriod);

  /**
   * Loads the model with data from the engine
   */
  void load();

  /**
   * This method extracts the transaction id from a combined
   * transactionSplitId and returns it. In case the @a transactionSplitId does
   * not resembles a transactionSplitId an empty string is returned.
   */
  QString transactionIdFromTransactionSplitId(const QString& transactionSplitId) const;

public Q_SLOTS:
  void slotAddTransaction   (eMyMoney::File::Object objType, const QString& id);
  void slotModifyTransaction(eMyMoney::File::Object objType, const QString& id);
  void slotRemoveTransaction(eMyMoney::File::Object objType, const QString& id);
  void slotAddSchedule      (eMyMoney::File::Object objType, const QString& id);
  void slotModifySchedule   (eMyMoney::File::Object objType, const QString& id);
  void slotRemoveSchedule   (eMyMoney::File::Object objType, const QString& id);

private:
  Q_DISABLE_COPY(LedgerModel)
  Q_DECLARE_PRIVATE(LedgerModel)
  const QScopedPointer<LedgerModelPrivate> d_ptr;
};

#endif // LEDGERMODEL_H

