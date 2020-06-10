/*
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
