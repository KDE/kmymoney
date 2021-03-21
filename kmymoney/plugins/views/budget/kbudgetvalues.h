/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KBUDGETVALUES_H
#define KBUDGETVALUES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybudget.h"

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */

class KBudgetValuesPrivate;
class KBudgetValues : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KBudgetValues)

public:
    explicit KBudgetValues(QWidget* parent = nullptr);
    ~KBudgetValues();

    void setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount);
    void budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount);
    void clear();

Q_SIGNALS:
    void valuesChanged();

protected Q_SLOTS:
    void slotChangePeriod(int id);

    /**
     * This slot clears the value in the value widgets of the selected budget type.
     * Values of the other types are unaffected.
     */
    void slotClearAllValues();

    /**
     * Helper slot used to postpone sending the valuesChanged() signal.
     */
    void slotNeedUpdate();

    void slotUpdateClearButton();

protected:
    bool eventFilter(QObject* o, QEvent* e) override;

private:
    KBudgetValuesPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KBudgetValues)
};

#endif
