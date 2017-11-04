/***************************************************************************
                          ledgersplit.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
