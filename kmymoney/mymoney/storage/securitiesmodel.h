/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SECURITIESMODEL_H
#define SECURITIESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneysecurity.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SecuritiesModel : public MyMoneyModel<MyMoneySecurity>
{
  Q_OBJECT

public:
  enum Column {
    Security = 0,
    Symbol,
    Type,
    Market,
    Currency,
    Fraction,
    // insert new columns above this line
    MaxColumns
  };


  explicit SecuritiesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~SecuritiesModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  void addCurrency(const MyMoneySecurity& currency);
  void loadCurrencies(const QMap<QString, MyMoneySecurity>& list);

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SECURITIESMODEL_H

