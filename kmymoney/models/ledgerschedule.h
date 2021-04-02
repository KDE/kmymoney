/*
    SPDX-FileCopyrightText: 2014-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERSCHEDULE_H
#define LEDGERSCHEDULE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgertransaction.h"

class MyMoneySchedule;
class MyMoneyTransaction;

class LedgerSchedulePrivate;
class LedgerSchedule : public LedgerTransaction
{
public:
    explicit LedgerSchedule(const MyMoneySchedule& s, const MyMoneyTransaction& t, const MyMoneySplit& sp);
    LedgerSchedule(const LedgerSchedule & other);
    LedgerSchedule(LedgerSchedule && other);
    LedgerSchedule & operator=(LedgerSchedule other);
    friend void swap(LedgerSchedule& first, LedgerSchedule& second);
    ~LedgerSchedule() override;

    /// @copydoc LedgerItem::transactionSplitId()
    QString transactionSplitId() const override;

    QString scheduleId() const;

    /// @copydoc LedgerItem::isImported()
    bool isImported() const override;

private:
    LedgerSchedule();
    Q_DECLARE_PRIVATE(LedgerSchedule)
};

inline void swap(LedgerSchedule& first, LedgerSchedule& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline LedgerSchedule::LedgerSchedule(LedgerSchedule && other) : LedgerSchedule() // krazy:exclude=inline
{
    swap(*this, other);
}

inline LedgerSchedule & LedgerSchedule::operator=(LedgerSchedule other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

#endif // LEDGERSCHEDULE_H

