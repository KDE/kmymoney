/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SECURITIESMODEL_H
#define SECURITIESMODEL_H

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

namespace eMyMoney { namespace File { enum class Object; } }

class KMM_MODELS_EXPORT SecuritiesModel : public QStandardItemModel
{
  Q_OBJECT

public:
  enum Column { Security = 0, Symbol, Type, Market, Currency, Fraction };

  ~SecuritiesModel();

  auto getColumns();
  static QString getHeaderName(const Column column);

public Q_SLOTS:
  void slotObjectAdded(eMyMoney::File::Object objType, const QString &id);
  void slotObjectModified(eMyMoney::File::Object objType, const QString &id);
  void slotObjectRemoved(eMyMoney::File::Object objType, const QString& id);

private:
  SecuritiesModel(QObject *parent = nullptr);
  SecuritiesModel(const SecuritiesModel&);
  SecuritiesModel& operator=(SecuritiesModel&);
  friend class Models;  // only this class can create SecuritiesModel

  void init();
  void load();

protected:
  class Private;
  Private* const d;
};

class KMM_MODELS_EXPORT SecuritiesFilterProxyModel : public QSortFilterProxyModel
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

private:
  class Private;
  Private* const d;

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable) }
#endif

};

#undef QSortFilterProxyModel
#endif // SECURITIESMODEL_H
