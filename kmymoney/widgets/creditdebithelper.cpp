/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
