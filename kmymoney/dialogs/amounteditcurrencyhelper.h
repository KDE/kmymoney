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
class MultiCurrencyEdit;
class MyMoneySecurity;
class AmountEditCurrencyHelperPrivate;

#ifdef Q_OS_WIN
#undef KMM_WIDGETS_EXPORT
#define KMM_WIDGETS_EXPORT
#endif

/**
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
    explicit AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, MultiCurrencyEdit* amount, const QString& commodityId);

    ~AmountEditCurrencyHelper();

    void setCommodity(const QString& commodityId);

public Q_SLOTS:
    void categoryChanged(const QString& id);

Q_SIGNALS:
    /**
     * This signal is emitted when the @a commodity to be
     * used for the shares has changed.
     */
    void commodityChanged(const MyMoneySecurity& commodity);

private:
    AmountEditCurrencyHelperPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(AmountEditCurrencyHelper)
};

#endif
