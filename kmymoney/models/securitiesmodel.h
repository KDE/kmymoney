/***************************************************************************
                          securitiesmodel.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SECURITIESMODEL_H
#define SECURITIESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KItemModels/KRecursiveFilterProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObject;

namespace eMyMoney { namespace File { enum class Object; } }

class SecuritiesModel : public QStandardItemModel
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

class SecuritiesFilterProxyModel : public KRecursiveFilterProxyModel
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
};

#endif // SECURITIESMODEL_H
