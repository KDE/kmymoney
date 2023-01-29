/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERFILTER_H
#define LEDGERFILTER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QComboBox;
class QLineEdit;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgersortproxymodel.h"

class LedgerFilterPrivate;
class KMM_MODELS_EXPORT LedgerFilter : public LedgerSortProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LedgerFilter)
    Q_DISABLE_COPY(LedgerFilter)

public:
    enum class State {
        Any,
        Imported,
        Matched,
        Erroneous,
        NotMarked,
        NotReconciled,
        Cleared,
        Scheduled,
    };

    explicit LedgerFilter(QObject* parent);

    void setComboBox(QComboBox* filterBox);
    void setLineEdit(QLineEdit* lineEdit);

    /**
     * Clear all filter settings. This will cause the
     * pattern of the filterFixedString to be cleared,
     * the stateFilter to be set to State::Any and the
     * endDate to be set to QDate().
     */
    void clearFilter();

    void setStateFilter(LedgerFilter::State state);
    void setFilterFixedString(const QString &pattern);

    /**
     * Set the end date of the state filter for transactions.
     * Transactions with a postdate after @a date will not be filtered.
     * When the object is created, the setting is QDate() which
     * causes the filter to be applied to all transactions.
     */
    void setEndDate(const QDate& endDate);

    /**
     * This method returns information about an active
     * filter (@c true or @c false) when @a role is
     * @c ActiveFilterRole. Otherwise it returns the
     * return value of the base class implementation.
     */
    QVariant data(const QModelIndex& index, int role) const override;

protected:
    /**
     * @note Does not call base class implementation
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    // prohibit usage of these methods as we internally rely on setFilterFixedString
    void setFilterRegularExpression(const QString& pattern);
    void setFilterRegExp(const QString &pattern);
};

Q_DECLARE_METATYPE(LedgerFilter::State)

#endif // LEDGERFILTER_H

