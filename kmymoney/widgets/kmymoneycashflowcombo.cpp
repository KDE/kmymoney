/*
 * Copyright 2009-2010  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2011-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneycashflowcombo.h"
#include "kmymoneymvccombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "widgetenums.h"

using namespace eWidgets;
using namespace eMyMoney;

class KMyMoneyCashFlowComboPrivate : public KMyMoneyMVCComboPrivate
{
  Q_DISABLE_COPY(KMyMoneyCashFlowComboPrivate)

public:
  KMyMoneyCashFlowComboPrivate() :
    m_dir(eRegister::CashFlowDirection::Unknown)
  {
  }

  eRegister::CashFlowDirection   m_dir;
};

KMyMoneyCashFlowCombo::KMyMoneyCashFlowCombo(Account::Type accountType, QWidget* parent) :
    KMyMoneyMVCCombo(*new KMyMoneyCashFlowComboPrivate, false, parent)
{
  addItem(" ", QVariant((int)eRegister::CashFlowDirection::Unknown));
  if (accountType == Account::Type::Income || accountType == Account::Type::Expense) {
    // this is used for income/expense accounts to just show the reverse sense
    addItem(i18nc("Activity for income categories", "Received"), QVariant((int)eRegister::CashFlowDirection::Payment));
    addItem(i18nc("Activity for expense categories", "Paid"), QVariant((int)eRegister::CashFlowDirection::Deposit));
  } else {
    addItem(i18n("Pay to"), QVariant((int)eRegister::CashFlowDirection::Payment));
    addItem(i18n("From"), QVariant((int)eRegister::CashFlowDirection::Deposit));
  }

  connect(this, &KMyMoneyMVCCombo::itemSelected, this, &KMyMoneyCashFlowCombo::slotSetDirection);
}

KMyMoneyCashFlowCombo::~KMyMoneyCashFlowCombo()
{
}

void KMyMoneyCashFlowCombo::setDirection(eRegister::CashFlowDirection dir)
{
  Q_D(KMyMoneyCashFlowCombo);
  d->m_dir = dir;
  QString num;
  setSelectedItem(num.setNum((int)dir));
}

eRegister::CashFlowDirection KMyMoneyCashFlowCombo::direction() const
{
  Q_D(const KMyMoneyCashFlowCombo);
  return d->m_dir;
}

void KMyMoneyCashFlowCombo::slotSetDirection(const QString& id)
{
  Q_D(KMyMoneyCashFlowCombo);
  QString num;
  for (int i = (int)eRegister::CashFlowDirection::Deposit; i <= (int)eRegister::CashFlowDirection::Unknown; ++i) {
    num.setNum(i);
    if (num == id) {
      d->m_dir = static_cast<eRegister::CashFlowDirection>(i);
      break;
    }
  }
  emit directionSelected(d->m_dir);
  update();
}

void KMyMoneyCashFlowCombo::removeDontCare()
{
  removeItem(findData(QVariant((int)eRegister::CashFlowDirection::Unknown), Qt::UserRole, Qt::MatchExactly));
}
