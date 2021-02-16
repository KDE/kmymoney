/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERSPLIT_H
#define LEDGERSPLIT_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgertransaction.h"

class MyMoneySplit;
class MyMoneyTransaction;

class LedgerSplitPrivate;
class LedgerSplit : public LedgerTransaction
{
public:
  explicit LedgerSplit(const MyMoneyTransaction& t, const MyMoneySplit& s);
  LedgerSplit(const LedgerSplit & other);
  LedgerSplit(LedgerSplit && other);
  LedgerSplit & operator=(LedgerSplit other);
  friend void swap(LedgerSplit& first, LedgerSplit& second);

  ~LedgerSplit() override;

  /// @copydoc LedgerItem::memo()
  QString memo() const override;

private:
  LedgerSplit();
  Q_DECLARE_PRIVATE(LedgerSplit)
};

inline void swap(LedgerSplit& first, LedgerSplit& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline LedgerSplit::LedgerSplit(LedgerSplit && other) : LedgerSplit() // krazy:exclude=inline
{
  swap(*this, other);
}

inline LedgerSplit & LedgerSplit::operator=(LedgerSplit other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

#endif // LEDGERSPLIT_H
