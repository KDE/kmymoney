/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AMOUNTEDITCURRENCYHELPER_H
#define AMOUNTEDITCURRENCYHELPER_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyAccountCombo;
class AmountEdit;
class CreditDebitHelper;
class AmountEditCurrencyHelperPrivate;

#ifdef Q_OS_WIN
#undef KMM_WIDGETS_EXPORT
#define KMM_WIDGETS_EXPORT
#endif



/**
 * This class takes care of the selection of visible columns of a tree view and
 * their sizes and stores the selection in the global application configuration.
 *
 * The @a parent tree view must have a model attached that allows to
 * extract the maximum number of columns. The header names found in
 * the model are displayed in a menu when the user clicks on the
 * header with the right mouse button.
 *
 * @author Thomas Baumgart
 */
class KMM_BASE_DIALOGS_EXPORT AmountEditCurrencyHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AmountEditCurrencyHelper)

public:
    /**
     * Creates a AmountEditCurrencyHelper object
     *
     * @param category pointer to KMyMoneyAccountCombo object
     * @param amount pointer to AmountEdit object
     */
    explicit AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, AmountEdit* amount, const QString& commodityId);

    /**
     * Creates a AmountEditCurrencyHelper object
     *
     * @param category pointer to KMyMoneyAccountCombo object
     * @param amount pointer to CreditDebitHelper object
     */
    explicit AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, CreditDebitHelper* amount, const QString& commodityId);

    ~AmountEditCurrencyHelper();

    void setCommodity(const QString& commodityId);

public Q_SLOTS:
    void categoryChanged(const QString& id);

private:
    AmountEditCurrencyHelperPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(AmountEditCurrencyHelper)
};

#endif
