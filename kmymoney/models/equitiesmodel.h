/*
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EQUITIESMODEL_H
#define EQUITIESMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>

#include <QSortFilterProxyModel>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif
// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObject;
class MyMoneyAccount;

namespace eMyMoney { namespace File { enum class Object; } }

class KMM_MODELS_EXPORT EquitiesModel : public QStandardItemModel
{
  Q_OBJECT

public:
  enum Column { Equity = 0, Symbol, Value, Quantity, Price };
  enum Role { InvestmentID = Qt::UserRole, EquityID = Qt::UserRole, SecurityID = Qt::UserRole + 1 };

  ~EquitiesModel();

  auto getColumns();
  static QString getHeaderName(const Column column);

public Q_SLOTS:
  void slotObjectAdded(eMyMoney::File::Object objType, const QString &id);
  void slotObjectModified(eMyMoney::File::Object objType, const QString &id);
  void slotObjectRemoved(eMyMoney::File::Object objType, const QString& id);
  void slotBalanceOrValueChanged(const MyMoneyAccount &account);

private:
  EquitiesModel(QObject *parent = nullptr);
  EquitiesModel(const EquitiesModel&);
  EquitiesModel& operator=(EquitiesModel&);
  friend class Models;  // only this class can create EquitiesModel

  void init();
  void load();

protected:
  class Private;
  Private* const d;
};

class KMM_MODELS_EXPORT EquitiesFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  EquitiesFilterProxyModel(QObject *parent , EquitiesModel *model, const QList<EquitiesModel::Column> &columns = QList<EquitiesModel::Column>());
  ~EquitiesFilterProxyModel();

  QList<EquitiesModel::Column> &getVisibleColumns();
  void setHideClosedAccounts(const bool hideClosedAccounts);
  void setHideZeroBalanceAccounts(const bool hideZeroBalanceAccounts);

Q_SIGNALS:
  void columnToggled(const EquitiesModel::Column column, const bool show);

public Q_SLOTS:
  void slotColumnsMenu(const QPoint);

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
  class Private;
  Private* const d;
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable) }
#endif

};

#undef QSortFilterProxyModel
#endif // EQUITIESMODEL_H
