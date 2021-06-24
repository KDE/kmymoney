/*
    SPDX-FileCopyrightText: 2010-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSPROXYMODEL_H
#define ACCOUNTSPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QtGlobal> // for QT_VERSION macro
#include <QSortFilterProxyModel>

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

/**
  * A proxy model to provide various sorting and filtering operations for @ref AccountsModel.
  *
  * Here is an example of how to use this class in combination with the @ref AccountsModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   AccountsProxyModel *filterModel = new AccountsProxyModel(widget);
  *   filterModel->addAccountGroup(AccountsProxyModel::assetLiabilityEquity());
  *   filterModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
  * @endcode
  *
  * @see AccountsModel
  *
  * @author Cristian Onet 2010
  * @author Thomas Baumgart 2019
  *
  */

namespace eMyMoney {
namespace Account {
enum class Type;
}
}
namespace eAccountsModel {
enum class Column;
}

class AccountsProxyModelPrivate;
class KMM_MODELS_EXPORT AccountsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsProxyModel)

public:
    explicit AccountsProxyModel(QObject *parent = nullptr);
    virtual ~AccountsProxyModel();

    void addAccountType(eMyMoney::Account::Type type);
    void addAccountGroup(const QVector<eMyMoney::Account::Type> &groups);
    void removeAccountType(eMyMoney::Account::Type type);

    void clear();

    void setHideClosedAccounts(bool hideClosedAccounts);
    bool hideClosedAccounts() const;

    void setHideEquityAccounts(bool hideEquityAccounts);
    bool hideEquityAccounts() const;

    void setHideUnusedIncomeExpenseAccounts(bool hideUnusedIncomeExpenseAccounts);
    bool hideUnusedIncomeExpenseAccounts() const;

    void setHideFavoriteAccounts(bool hideFavoriteAccounts);
    bool hideFavoriteAccounts() const;

    void setHideAllEntries(bool hideAllEntries);
    bool hideAllEntries() const;

    int visibleItems(bool includeBaseAccounts = false) const;

    void setNotSelectable(const QString& accountId);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * This is a convenience method which returns a prefilled vector
     * to be used with accAccountGroup() for asset, liability and equity
     * accounts.
     */
    static QVector<eMyMoney::Account::Type> assetLiabilityEquity();

    /**
     * This is a convenience method which returns a prefilled vector
     * to be used with accAccountGroup() for income and expense
     * accounts.
     */
    static QVector<eMyMoney::Account::Type> incomeExpense();

    /**
     * This is a convenience method which returns a prefilled vector
     * to be used with accAccountGroup() for asset, liability, equity, income and expense
     * accounts.
     */
    static QVector<eMyMoney::Account::Type> assetLiabilityEquityIncomeExpense();

protected:
    const QScopedPointer<AccountsProxyModelPrivate> d_ptr;
    AccountsProxyModel(AccountsProxyModelPrivate &dd, QObject *parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool acceptSourceItem(const QModelIndex &source) const;

    bool filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const;

    int visibleItems(const QModelIndex& index) const;

Q_SIGNALS:
    void unusedIncomeExpenseAccountHidden() const;

private:
    Q_DECLARE_PRIVATE(AccountsProxyModel)

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
    // provide the interface for backward compatbility
    void setRecursiveFilteringEnabled(bool enable) {
        Q_UNUSED(enable)
    }
#endif

};

#undef QSortFilterProxyModel

/**
 * A proxy model used to filter all the data from the core accounts model leaving
 * only the name of the accounts so this model can be used in the account
 * completion combo.
 *
 * It shows only the first column (account name) and makes top level items non-selectable.
 *
 * @see AccountsModel
 * @see AccountsProxyModel
 *
 * @author Cristian Onet 2010
 * @author Christian David
 */

template <class baseProxyModel>
class AccountNamesFilterProxyModelTpl : public baseProxyModel
{
public:
    explicit AccountNamesFilterProxyModelTpl(QObject *parent = 0)
        : baseProxyModel(parent)
    {}

    /**
     * Top items are not selectable because they are not real accounts but are only used for grouping.
     */
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override
    {
        if (!idx.parent().isValid())
            return baseProxyModel::flags(idx) & ~Qt::ItemIsSelectable;
        return baseProxyModel::flags(idx);
    }

    /**
     * The Qt::EditRole returns the full account name including parent account name
     */
    virtual QVariant data(const QModelIndex& idx, int role) const override
    {
        if (role == Qt::EditRole) {
            return baseProxyModel::data(idx, eMyMoney::Model::AccountFullNameRole);
        }
        return baseProxyModel::data(idx, role);
    }

protected:
    /**
     * Filter all but the first column.
     */
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override
    {
        Q_UNUSED(source_parent)
        if (source_column == 0)
            return true;
        return false;
    }
};

/**
 * @brief "typedef" for AccountNamesFilterProxyModelTpl<AccountsFilterProxyModel>
 *
 * To create valid Qt moc data this class inherits the template and uses Q_OBJECT.
 * Simply using a typedef like
 *
 * @code
 * typedef AccountNamesFilterProxyModelTpl<AccountsFilterProxyModel> AccountNamesFilterProxyModel;
 * @endcode
 *
 * does not work, because at some point the code needs to use qobject_cast<> to promote a
 * returned QSortFilterProxyModel pointer to an AccountNamesFilterProxyModel which is
 * only possible with Q_OBJECT being in place.
 */
class KMM_MODELS_EXPORT AccountNamesFilterProxyModel : public AccountNamesFilterProxyModelTpl<AccountsProxyModel>
{
    Q_OBJECT
public:
    explicit AccountNamesFilterProxyModel(QObject* parent = 0)
        : AccountNamesFilterProxyModelTpl< AccountsProxyModel >(parent) {}
};


#endif
