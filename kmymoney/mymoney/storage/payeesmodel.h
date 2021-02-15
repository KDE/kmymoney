/*
 * SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAYEESMODEL_H
#define PAYEESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneypayee.h"

class QUndoStack;
class PayeesModelEmptyPayee;
/**
  */
class KMM_MYMONEY_EXPORT PayeesModel : public MyMoneyModel<MyMoneyPayee>
{
  Q_OBJECT

public:
  class Column {
  public:
    enum {
      Name
    } Columns;
  };

  explicit PayeesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  ~PayeesModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  PayeesModelEmptyPayee* emptyPayee();

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

class KMM_MYMONEY_EXPORT PayeesModelEmptyPayee : public PayeesModel
{
  Q_OBJECT

public:
  explicit PayeesModelEmptyPayee(QObject* parent = nullptr);
  virtual ~PayeesModelEmptyPayee();

  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const final override;

protected:
  void addItem(MyMoneyPayee& p) { Q_UNUSED(p); }
  void removeTransaction(const MyMoneyPayee& p) { Q_UNUSED(p); }
  void modifyTransaction(const MyMoneyPayee& newPayee) { Q_UNUSED(newPayee); }

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override
  {
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
  }

  void load(const QMap<QString, MyMoneyPayee>& list) { Q_UNUSED(list); };
};

#endif // PAYEESMODEL_H

