/***************************************************************************
                          ledgerschedule.h
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

