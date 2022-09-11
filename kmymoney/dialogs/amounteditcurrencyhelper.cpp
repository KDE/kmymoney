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

#include "accountsmodel.h"
#include "amountedit.h"
#include "kmymoneyaccountcombo.h"
#include "multicurrencyedit.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "securitiesmodel.h"

class AmountEditCurrencyHelperPrivate
{
    Q_DECLARE_PUBLIC(AmountEditCurrencyHelper)

public:
    AmountEditCurrencyHelperPrivate(AmountEditCurrencyHelper* qq)
        : q_ptr(qq)
    {
    }

    AmountEditCurrencyHelper* q_ptr;
    MultiCurrencyEdit* amount;
    QString commodityId;

    void init(KMyMoneyAccountCombo* category, const QString& _commodityId)
    {
        Q_Q(AmountEditCurrencyHelper);
        commodityId = _commodityId;

        q->connect(category, &KMyMoneyAccountCombo::accountSelected, q, &AmountEditCurrencyHelper::categoryChanged);

        q->categoryChanged(category->getSelected());
    }
};

AmountEditCurrencyHelper::AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, MultiCurrencyEdit* amount, const QString& commodityId)
    : QObject(category)
    , d_ptr(new AmountEditCurrencyHelperPrivate(this))
{
    Q_D(AmountEditCurrencyHelper);
    d->amount = amount;
    connect(amount->widget(), &QObject::destroyed, this, &QObject::deleteLater);
    connect(this, &AmountEditCurrencyHelper::commodityChanged, this, [&](const MyMoneySecurity& commodity) {
        Q_D(AmountEditCurrencyHelper);
        d->amount->setSharesCommodity(commodity);
    });
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
    // update the widget
    if (!commodityId.isEmpty()) {
        const auto category = qobject_cast<KMyMoneyAccountCombo*>(parent());
        if (category) {
            categoryChanged(category->getSelected());
        }
    }
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
                    Q_EMIT commodityChanged(security);
                } else {
                    Q_EMIT commodityChanged(MyMoneyFile::instance()->security(d->commodityId));
                }
            }
        } catch (MyMoneyException&) {
        }
    }
}
