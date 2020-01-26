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

#ifndef SECURITIESFILTERPROXYMODEL_H
#define SECURITIESFILTERPROXYMODEL_H

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

/// @todo cleanup
/// @todo port to new model code

// #include <KItemModels/KRecursiveFilterProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "securitiesmodel.h"

/// @todo cleanup
// class MyMoneyObject;

// namespace eMyMoney { namespace File { enum class Object; } }
//
// class KMM_MODELS_EXPORT SecuritiesModel : public QStandardItemModel
// {
//   Q_OBJECT
//
// public:
//   enum Column { Security = 0, Symbol, Type, Market, Currency, Fraction };
//
//   ~SecuritiesModel();
//
//   auto getColumns();
//   static QString getHeaderName(const Column column);
//
// public Q_SLOTS:
//   void slotObjectAdded(eMyMoney::File::Object objType, const QString &id);
//   void slotObjectModified(eMyMoney::File::Object objType, const QString &id);
//   void slotObjectRemoved(eMyMoney::File::Object objType, const QString& id);
//
// private:
//   SecuritiesModel(QObject *parent = nullptr);
//   SecuritiesModel(const SecuritiesModel&);
//   SecuritiesModel& operator=(SecuritiesModel&);
//   friend class Models;  // only this class can create SecuritiesModel
//
//   void init();
//   void load();
//
// protected:
//   class Private;
//   Private* const d;
// };

/// @todo cleanup
class KMM_MODELS_EXPORT SecuritiesFilterProxyModel : public QSortFilterProxyModel //KRecursiveFilterProxyModel
{
  Q_OBJECT

public:
  SecuritiesFilterProxyModel(QObject *parent , SecuritiesModel *model, const QList<SecuritiesModel::Column> &columns = QList<SecuritiesModel::Column>());
  ~SecuritiesFilterProxyModel();

  QList<SecuritiesModel::Column> &getVisibleColumns();

Q_SIGNALS:
  void columnToggled(const SecuritiesModel::Column column, const bool show);

public Q_SLOTS:
  void slotColumnsMenu(const QPoint);

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  class Private;
  Private* const d;

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable) }
#endif

};

#undef QSortFilterProxyModel
#endif // SECURITIESFILTERPROXYMODEL_H
