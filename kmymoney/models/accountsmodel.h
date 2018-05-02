/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017, 2018  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * A model for the accounts.
  * This model loads all the accounts from the @ref MyMoneyFile.
  * It also computes various data like account balances needed
  * in different views. This object should be kept sychronized
  * with the data in the @ref MyMoneyFile (this is accomplished
  * by the @ref Models object).
  *
  * @see MyMoneyAccount
  * @see MyMoneyFile
  *
  * @author Cristian Onet 2010
  * @author Łukasz Wojniłowicz 2017, 2018
  *
  */
class MyMoneyObject;
class MyMoneyMoney;
class MyMoneyAccount;

namespace eMyMoney { namespace File { enum class Object; } }
namespace eAccountsModel { enum class Column; }
namespace eView { enum class Intent; }

class AccountsModelPrivate;
class KMM_MODELS_EXPORT AccountsModel : public QStandardItemModel
{
  Q_OBJECT
  Q_DISABLE_COPY(AccountsModel)

public:
  /**
    * The account id used by this model for the 'Favorites' top level item. This can be used to identify that item on the @ref AccountIdRole.
    */
  static const QString favoritesAccountId;

  explicit AccountsModel(QObject *parent = nullptr);
  virtual ~AccountsModel() override;

  /**
    * This method must be used to perform the initial load of the model.
    */
  void load();

  /**
    * Compute the value of the given account using the provided balance.
    * The value is defined as the balance of the account converted to the base currency.
    *
    * @param account The account for which the value is being computed.
    * @param balance The balance which should be used.
    *
    * @todo Make this a static or a global function since the object's state has nothing to do with this computation
    */
  MyMoneyMoney accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance);

  /**
   * This method returns the QModelIndex of the account specified by its @a id. If the
   * account was not found, an invalid QModelIndex is returned.
   */
  QModelIndex accountById(const QString& id) const;

  QList<eAccountsModel::Column> *getColumns();

  void setColumnVisibility(const eAccountsModel::Column column, const bool show);
  static QString getHeaderName(const eAccountsModel::Column column);

public Q_SLOTS:
  void slotReconcileAccount(const MyMoneyAccount &account, const QDate &reconciliationDate, const MyMoneyMoney &endingBalance);
  void slotObjectAdded(eMyMoney::File::Object objType, const QString &id);
  void slotObjectModified(eMyMoney::File::Object objType, const QString &id);
  void slotObjectRemoved(eMyMoney::File::Object objType, const QString& id);
  void slotBalanceOrValueChanged(const MyMoneyAccount &account);

Q_SIGNALS:
  /**
    * Emit this signal when the net worth based on the value of the loaded accounts is changed.
    */
  void netWorthChanged(const QVariantList&, eView::Intent);

  /**
    * Emit this signal when the profit based on the value of the loaded accounts is changed.
    */
  void profitChanged(const QVariantList&, eView::Intent);

protected:
  AccountsModelPrivate * const d_ptr;
  AccountsModel(AccountsModelPrivate &dd, QObject *parent);

private:
  Q_DECLARE_PRIVATE(AccountsModel)

  void checkNetWorth();
  void checkProfit();

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;
};

/**
  * A model for the accounts grouped by institutions. It extends the functionality already present
  * in @ref AccountsModel to enable the grouping of the accounts by institutions.
  *
  * @author Cristian Onet 2011
  * @author Łukasz Wojniłowicz 2017, 2018
  *
  */
class InstitutionsModelPrivate;
class KMM_MODELS_EXPORT InstitutionsModel : public AccountsModel
{
  Q_OBJECT
  Q_DISABLE_COPY(InstitutionsModel)

public:
  explicit InstitutionsModel(QObject *parent = nullptr);
  ~InstitutionsModel() override;

  /**
    * This method must be used to perform the initial load of the model.
    */
  void load();

public Q_SLOTS:
  void slotObjectAdded(eMyMoney::File::Object objType, const QString &id);
  void slotObjectModified(eMyMoney::File::Object objType, const QString &id);
  void slotObjectRemoved(eMyMoney::File::Object objType, const QString& id);

private:
  Q_DECLARE_PRIVATE(InstitutionsModel)

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;
};

#endif
