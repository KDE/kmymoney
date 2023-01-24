/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERSORTPROXYMODEL_H
#define LEDGERSORTPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>
#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerviewsettings.h"
#include "mymoneyenums.h"

class LedgerSortProxyModelPrivate;
class KMM_MODELS_EXPORT LedgerSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LedgerSortProxyModel)
    Q_DISABLE_COPY(LedgerSortProxyModel)

public:
    virtual ~LedgerSortProxyModel();

    /**
     * Reimplemented to support KMyMoney specific sort options
     */
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /**
     * Sorts the model on the next return to the event loop
     */
    void sortOnIdle();

    /**
     * This method changes the global filter for
     * all instances of LedgerSortProxyModel and derived objects to
     * filter out any transaction with a post date prior to
     * @a date. If @a date is an invalid QDate, then the
     * filter is inactive.
     */
    void setHideTransactionsBefore(const QDate& date);

    /**
     * This method changes the global filter for
     * all instances of LedgerSortProxyModel and derived objects to
     * filter out any reconciled transaction.
     */
    void setHideReconciledTransactions(bool hide);

    /**
     * Reimplemented for internal reasons
     */
    void setSourceModel(QAbstractItemModel* sourceModel) override;

    virtual void setLedgerSortOrder(LedgerSortOrder sortOrder);

    virtual LedgerSortOrder ledgerSortOrder() const;

    /**
     * This method can be used to temporarily prevent
     * sorting of the model. Once turned back on and
     * sort() was called in the meantime it will be
     * performed upon the run of the next event loop.
     *
     * @sa doSort()
     */
    virtual void setSortingEnabled(bool enable);

    /**
     * This method is used to process postponed sorting
     */
    virtual void doSortOnIdle();

protected:
    explicit LedgerSortProxyModel(LedgerSortProxyModelPrivate* dd, QObject* parent);

    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    /**
     * @note This does not call the base class implementation for speed purposes
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    /**
     * This is a convenience method for sort()
     */
    virtual void doSort();

    /**
     * This is a debugging function for developers
     */
    void dumpSourceModel() const;

protected:
    LedgerSortProxyModelPrivate* d_ptr;
};

#endif // LEDGERSORTPROXYMODEL_H
