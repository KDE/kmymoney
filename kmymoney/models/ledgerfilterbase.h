/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERFILTERBASE_H
#define LEDGERFILTERBASE_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgersortproxymodel.h"

class LedgerFilterBasePrivate;
class KMM_MODELS_EXPORT LedgerFilterBase : public LedgerSortProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LedgerFilterBase)
    Q_DISABLE_COPY(LedgerFilterBase)

public:
    virtual ~LedgerFilterBase();

    void setAccountType(eMyMoney::Account::Type type);

    void setFilterFixedString(const QString& filter);
    void setFilterFixedStrings(const QStringList& filters);
    void appendFilterFixedString(const QString& filter);

    QStringList filterFixedStrings() const;

    /**
     * This method returns the headerData adjusted to the current
     * accountType
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * This method is used to update the balance data. All other
     * roles are forwarded to the source model. The balance can
     * be retrieved using the derived model.
     *
     * It also maintains the split height cache. Setting any @a index
     * with role JournalSplitMaxLinesCountRole to a @a value of -1
     * will clear the cache of all items.
     *
     * Call with any @a index and @a value of -1 and @a role
     * @c eMyMoney::Model::JournalSplitMaxLinesCountRole to reset
     * the split height cache
     *
     * @sa LedgerAccountFilter::data()
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * Overridden for internal reasons to implement the functionality
     * provided by setLedgerIsEditable()
     *
     * @sa setLedgerIsEditable()
     */
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void setShowEntryForNewTransaction(bool show);

    void setShowScheduledTransactions(bool show);

    /**
     * add @a model to the source models
     */
    void addSourceModel(QAbstractItemModel* model);

    /**
     * remove @a model from the source models
     */
    void removeSourceModel(QAbstractItemModel* model);

    /**
     * Use to control maintenance of balance information
     */
    void setMaintainBalances(bool maintainBalances);

    /**
     * This can be used to prevent editing items in
     * the views using this model. The default value
     * when this object is constructed for this setting
     * is @c true. If set to false, editing the transactions
     * shown is not possible anymore.
     */
    void setLedgerIsEditable(bool enableEdit);

protected:
    explicit LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent);

    /**
     * @note This does not call the base class implementation for speed purposes
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

};

#endif // LEDGERFILTERBASE_H

