/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECIALDATESFILTER_H
#define SPECIALDATESFILTER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgersortproxymodel.h"

class SpecialLedgerItemFilterPrivate;
class KMM_MODELS_EXPORT SpecialLedgerItemFilter : public LedgerSortProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SpecialLedgerItemFilter)
    Q_DISABLE_COPY(SpecialLedgerItemFilter)

public:
    explicit SpecialLedgerItemFilter(QObject* parent);

    /**
     * Reimplemented to propagate sorting to sourceModel
     */
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setSourceModel(LedgerSortProxyModel* model);

    /**
     * This method returns the data from the base model and
     * intercepts the balance column and returns a fixed string
     * in case a balance cannot be provided (e.g. sorting is
     * not by date or some data is filtered)
     */
    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * Reimplemented to propagate setting of sort order to sourceModel
     */
    void setLedgerSortOrder(LedgerSortOrder sortOrder) override;

    LedgerSortOrder ledgerSortOrder() const override;

    /**
     * Reimplemented to propagate setting to sourceModel
     */
    void setSortingEnabled(bool enable) override;

public Q_SLOTS:
    void forceReload();

protected:
    /**
     * @note Does not call base class implementation
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    /**
     * Reimplemented to propagate setting to sourceModel
     */
    void doSortOnIdle() override;

private:
    // make sure that only LedgerSortProxyModel models can be used as sources
    void setSourceModel(QAbstractItemModel* model) override;
};

#endif // SPECIALDATESFILTER_H
