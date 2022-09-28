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

protected:
    explicit LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent);

    /**
     * @note This does not call the base class implementation for speed purposes
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

};

#endif // LEDGERFILTERBASE_H

