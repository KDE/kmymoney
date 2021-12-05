/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amounteditcurrencyhelper.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountcombo.h"
#include "amountedit.h"
#include "creditdebithelper.h"
#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "securitiesmodel.h"
#include "mymoneyexception.h"

class AmountEditCurrencyHelperPrivate
{
    Q_DECLARE_PUBLIC(AmountEditCurrencyHelper)

public:
    AmountEditCurrencyHelperPrivate(AmountEditCurrencyHelper* qq)
        : q_ptr(qq)
    {
    }

    AmountEditCurrencyHelper*       q_ptr;
    QString                         commodityId;

    void init(KMyMoneyAccountCombo* category, const QString& _commodityId)
    {
        Q_Q(AmountEditCurrencyHelper);
        commodityId = _commodityId;

        q->connect(category, &KMyMoneyAccountCombo::accountSelected, q, &AmountEditCurrencyHelper::categoryChanged);

        q->categoryChanged(category->getSelected());
    }
};


AmountEditCurrencyHelper::AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, AmountEdit* amount, const QString& commodityId)
    : QObject(category)
    , d_ptr(new AmountEditCurrencyHelperPrivate(this))
{
    Q_D(AmountEditCurrencyHelper);
    connect(amount, &QObject::destroyed, this, &QObject::deleteLater);
    connect(this, &AmountEditCurrencyHelper::currencySymbolChanged, amount, &AmountEdit::setCurrencySymbol);
    d->init(category, commodityId);
}

AmountEditCurrencyHelper::AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, CreditDebitHelper* amount, const QString& commodityId)
    : QObject(category)
    , d_ptr(new AmountEditCurrencyHelperPrivate(this))
{
    Q_D(AmountEditCurrencyHelper);
    connect(amount, &QObject::destroyed, this, &QObject::deleteLater);
    connect(this, &AmountEditCurrencyHelper::currencySymbolChanged, amount, &CreditDebitHelper::setCurrencySymbol);
    d->init(category, commodityId);
}

AmountEditCurrencyHelper::~AmountEditCurrencyHelper()
{
    Q_D(AmountEditCurrencyHelper);
    delete d;
}

void AmountEditCurrencyHelper::setCommodity(const QString& commodityId)
{
    Q_D(AmountEditCurrencyHelper);
    d->commodityId = commodityId;
}

void AmountEditCurrencyHelper::categoryChanged(const QString& id)
{
    Q_D(AmountEditCurrencyHelper);
    QString currencySymbol;
    QString currencyName;

    if (!id.isEmpty()) {
        try {
            const auto category = MyMoneyFile::instance()->account(id);
            const auto security = MyMoneyFile::instance()->security(category.currencyId());
            if (security.id() != d->commodityId) {
                if (category.isIncomeExpense()) {
                    currencySymbol = security.tradingSymbol();
                    currencyName = security.name();
                } else {
                    auto commodity = MyMoneyFile::instance()->security(d->commodityId);
                    currencySymbol = commodity.tradingSymbol();
                    currencyName = commodity.name();
                }
            }
        } catch (MyMoneyException& e) {
        }
    }

    emit currencySymbolChanged(currencySymbol, currencyName);
}
