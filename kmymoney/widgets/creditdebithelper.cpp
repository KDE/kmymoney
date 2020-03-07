/*
 * Copyright 2016-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "creditdebithelper.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "amountedit.h"

class CreditDebitHelperPrivate
{
  Q_DISABLE_COPY(CreditDebitHelperPrivate)
  Q_DECLARE_PUBLIC(CreditDebitHelper)

public:
  explicit CreditDebitHelperPrivate(CreditDebitHelper *qq) :
    q_ptr(qq)
  {
  }

  void widgetChanged(AmountEdit* src, AmountEdit* dst)
  {
    // make sure the objects exist
    if(!src || !dst) {
      return;
    }

    // in case both are filled with text, the src wins
    if(!src->text().isEmpty() && !dst->text().isEmpty()) {
      dst->clear();
    }

    // in case the source is negative, we negate the value
    // and load it into destination.
    if(src->value().isNegative()) {
      dst->setValue(-(src->value()));
      src->clear();
    }
    Q_Q(CreditDebitHelper);
    emit q->valueChanged();
  }

  CreditDebitHelper    *q_ptr;
  QPointer<AmountEdit>  m_credit;
  QPointer<AmountEdit>  m_debit;
};

CreditDebitHelper::CreditDebitHelper(QObject* parent, AmountEdit* credit, AmountEdit* debit) :
  QObject(parent),
  d_ptr(new CreditDebitHelperPrivate(this))
{
  Q_D(CreditDebitHelper);
  d->m_credit = credit;
  d->m_debit = debit;
  connect(d->m_credit.data(), &AmountEdit::valueChanged, this, &CreditDebitHelper::creditChanged);
  connect(d->m_debit.data(), &AmountEdit::valueChanged, this, &CreditDebitHelper::debitChanged);
}

CreditDebitHelper::~CreditDebitHelper()
{
  Q_D(CreditDebitHelper);
  delete d;
}

void CreditDebitHelper::creditChanged()
{
  Q_D(CreditDebitHelper);
  d->widgetChanged(d->m_credit, d->m_debit);
}

void CreditDebitHelper::debitChanged()
{
  Q_D(CreditDebitHelper);
  d->widgetChanged(d->m_debit, d->m_credit);
}

bool CreditDebitHelper::haveValue() const
{
  Q_D(const CreditDebitHelper);
  return (!d->m_credit->text().isEmpty()) || (!d->m_debit->text().isEmpty());
}

MyMoneyMoney CreditDebitHelper::value() const
{
  Q_D(const CreditDebitHelper);
  MyMoneyMoney value;
  if(d->m_credit && d->m_debit) {
    if(!d->m_credit->text().isEmpty()) {
      value = -d->m_credit->value();
    } else {
      value = d->m_debit->value();
    }
  } else {
    qWarning() << "CreditDebitHelper::value() called with no objects attached. Zero returned.";
  }
  return value;
}

void CreditDebitHelper::setValue(const MyMoneyMoney& value)
{
  Q_D(CreditDebitHelper);
  if(d->m_credit && d->m_debit) {
    if(value.isNegative()) {
      d->m_credit->setValue(-value);
      d->m_debit->clear();
    } else {
      d->m_debit->setValue(value);
      d->m_credit->clear();
    }
  } else {
    qWarning() << "CreditDebitHelper::setValue() called with no objects attached. Skipped.";
  }
}

void CreditDebitHelper::showCurrencySymbol(const QString& symbol)
{
  Q_D(CreditDebitHelper);
  if(d->m_credit && d->m_debit) {
    d->m_credit->showCurrencySymbol(symbol);
    d->m_debit->showCurrencySymbol(symbol);
  }
}
