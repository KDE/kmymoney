/*
    SPDX-FileCopyrightText: 2010-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

/**
  * A proxy model to provide various sorting and filtering operations for @ref AccountsModel.
  *
  * Here is an example of how to use this class in combination with the @ref AccountsModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   AccountsFilterProxyModel *filterModel = new AccountsFilterProxyModel(widget);
  *   filterModel->addAccountGroup(eMyMoney::Account::Type::Asset);
  *   filterModel->addAccountGroup(eMyMoney::Account::Type::Liability);
  *   filterModel->setSourceModel(Models::instance()->accountsModel());
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
  * @endcode
  *
  * @see AccountsModel
  *
  * @author Cristian Onet 2010
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

    int visibleItems(bool includeBaseAccounts = false) const;

    void setSourceColumns(QList<eAccountsModel::Column> *columns);

protected:
    const QScopedPointer<AccountsProxyModelPrivate> d_ptr;
    AccountsProxyModel(AccountsProxyModelPrivate &dd, QObject *parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool acceptSourceItem(const QModelIndex &source) const;

    bool filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const;

    int visibleItems(const QModelIndex& index) const;

Q_SIGNALS:
    void unusedIncomeExpenseAccountHidden();

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
#endif
