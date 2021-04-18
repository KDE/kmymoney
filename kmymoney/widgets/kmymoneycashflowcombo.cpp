/*
    SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
        addItem(i18nc("Payee", "Pay to"), QVariant((int)eRegister::CashFlowDirection::Payment));
        addItem(i18nc("Payer", "From"), QVariant((int)eRegister::CashFlowDirection::Deposit));
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

void KMyMoneyCashFlowCombo::reverseDirection()
{
    Q_D(KMyMoneyCashFlowCombo);
    switch (d->m_dir) {
    case eWidgets::eRegister::CashFlowDirection::Deposit:
        setDirection(eWidgets::eRegister::CashFlowDirection::Payment);
        break;
    case eWidgets::eRegister::CashFlowDirection::Payment:
        setDirection(eWidgets::eRegister::CashFlowDirection::Deposit);
        break;
    default:
        break;
    }
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
