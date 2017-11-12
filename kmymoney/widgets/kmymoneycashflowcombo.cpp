/***************************************************************************
                          kmymoneycashflowcombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
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

#include "kmymoneycashflowcombo.h"
#include "kmymoneymvccombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "registeritem.h"

#include "mymoneyenums.h"

using namespace eMyMoney;

class KMyMoneyCashFlowComboPrivate : public KMyMoneyMVCComboPrivate
{
  Q_DISABLE_COPY(KMyMoneyCashFlowComboPrivate)

public:
  KMyMoneyCashFlowComboPrivate() :
    m_dir(KMyMoneyRegister::Unknown)
  {
  }

  KMyMoneyRegister::CashFlowDirection   m_dir;
};

KMyMoneyCashFlowCombo::KMyMoneyCashFlowCombo(Account accountType, QWidget* parent) :
    KMyMoneyMVCCombo(*new KMyMoneyCashFlowComboPrivate, false, parent)
{
  addItem(" ", QVariant(KMyMoneyRegister::Unknown));
  if (accountType == Account::Income || accountType == Account::Expense) {
    // this is used for income/expense accounts to just show the reverse sense
    addItem(i18nc("Activity for income categories", "Received"), QVariant(KMyMoneyRegister::Payment));
    addItem(i18nc("Activity for expense categories", "Paid"), QVariant(KMyMoneyRegister::Deposit));
  } else {
    addItem(i18n("Pay to"), QVariant(KMyMoneyRegister::Payment));
    addItem(i18n("From"), QVariant(KMyMoneyRegister::Deposit));
  }

  connect(this, SIGNAL(itemSelected(QString)), this, SLOT(slotSetDirection(QString)));
}

KMyMoneyCashFlowCombo::~KMyMoneyCashFlowCombo()
{
}

void KMyMoneyCashFlowCombo::setDirection(KMyMoneyRegister::CashFlowDirection dir)
{
  Q_D(KMyMoneyCashFlowCombo);
  d->m_dir = dir;
  QString num;
  setSelectedItem(num.setNum(dir));
}

KMyMoneyRegister::CashFlowDirection KMyMoneyCashFlowCombo::direction() const
{
  Q_D(const KMyMoneyCashFlowCombo);
  return d->m_dir;
}

void KMyMoneyCashFlowCombo::slotSetDirection(const QString& id)
{
  Q_D(KMyMoneyCashFlowCombo);
  QString num;
  for (int i = KMyMoneyRegister::Deposit; i <= KMyMoneyRegister::Unknown; ++i) {
    num.setNum(i);
    if (num == id) {
      d->m_dir = static_cast<KMyMoneyRegister::CashFlowDirection>(i);
      break;
    }
  }
  emit directionSelected(d->m_dir);
  update();
}

void KMyMoneyCashFlowCombo::removeDontCare()
{
  removeItem(findData(QVariant(KMyMoneyRegister::Unknown), Qt::UserRole, Qt::MatchExactly));
}
