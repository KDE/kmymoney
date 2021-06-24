/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyaccount.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT AccountsModel : public MyMoneyModel<MyMoneyAccount>
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsModel)

public:
    enum Column {
        AccountName = 0,
        Type,
        HasOnlineMapping,
        CostCenter,
        Tax,
        Vat,
        Balance,
        PostedValue,
        TotalPostedValue,
        Number,
        Iban,
        BankCode,
        Bic,
        // insert new columns above this line
        MaxColumns,
    };

    enum ColorScheme {
        Positive,
        Negative,
    };

    explicit AccountsModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~AccountsModel();

    static const int ID_SIZE = 6;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

    void load(const QMap<QString, MyMoneyAccount>& list);

    void addItem(MyMoneyAccount& account);

    QList<MyMoneyAccount> itemList() const;
    QModelIndex indexById(const QString& id) const override;
    QModelIndexList indexListByName(const QString& name, const QModelIndex& parent = QModelIndex()) const override;


    /**
     * Returns the full account path for the given index @a idx.
     */
    QString indexToHierarchicalName(const QModelIndex& idx, bool includeStandardAccounts = false) const;

    /**
     * Convert the given @a accountId into the full hierarchical name
     * Include the standard base account if @a includeStandardAccounts is @c true
     *
     * Returns the full account path or empty if @a accountId is not known
     *
     * @sa indexToHierarchicalName()
     *
     * @note Previous name: accountToCategory()
     */
    QString accountIdToHierarchicalName(const QString& accountId, bool includeStandardAccounts = false) const;

    /**
     * Convert a given account @a name and @a type into the corresponding id. Valid types are
     * eMyMoney::Account::Type::(Income|Expense|Unknown). In case of Unknown, both Income and
     * Expense subtrees will be searched. The first matching entry will be used. If no account
     * is found, an empty string will be returned.
     */
    QString accountNameToId(const QString& name, eMyMoney::Account::Type type) const;

    QModelIndex favoriteIndex() const;
    QModelIndex assetIndex() const;
    QModelIndex liabilityIndex() const;
    QModelIndex incomeIndex() const;
    QModelIndex expenseIndex() const;
    QModelIndex equityIndex() const;

    void setColorScheme(ColorScheme scheme, const QColor& color);

    QModelIndexList accountsWithoutInstitutions() const;

    /**
     * Calculate the value in base currency for a given @a balance in
     * the currency of the account defined by @a accountId. The return
     * value returns a QPair where the first item is the value and
     * the second an indication if the value is exact (@c false) or
     * an approximation (@c true) in case not all price information
     * is available.
     */
    QPair<MyMoneyMoney, bool>  balanceToValue(const QString& accountId, MyMoneyMoney balance) const;

    void reparentAccount(const QString& accountId, const QString& newParentId);

    void removeItem(const QModelIndex& idx);

    void touchAccountById(const QString& id);

protected:
    void clearModelItems() override;
    void addFavorite(const QString& id);
    void removeFavorite(const QString& id);

    int processItems(Worker *worker) override;

    virtual void doAddItem(const MyMoneyAccount& item, const QModelIndex& parentIdx = QModelIndex()) override;
    virtual void doModifyItem(const MyMoneyAccount& before, const MyMoneyAccount& after) override;
    virtual void doRemoveItem(const MyMoneyAccount& before) override;

    MyMoneyModel<MyMoneyAccount>::Operation undoOperation(const MyMoneyAccount& before, const MyMoneyAccount& after) const override;
    void doReparentItem(const MyMoneyAccount& before, const MyMoneyAccount& after) override;

public Q_SLOTS:
    void setupAccountFractions();
    void updateAccountBalances(const QHash<QString, MyMoneyMoney>& balances);

Q_SIGNALS:
    void netWorthChanged(const MyMoneyMoney& amount, bool approximate);
    void profitLossChanged(const MyMoneyMoney& amount, bool approximate);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // ACCOUNTSMODEL_H

