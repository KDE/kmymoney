/*
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2010-2016 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyperiodcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

KMyMoneyPeriodCombo::KMyMoneyPeriodCombo(QWidget* parent) :
    KMyMoneyGeneralCombo(parent)
{
    insertItem(i18n("All dates"), (int)TransactionFilter::Date::All);
    insertItem(i18n("As of today"), (int)TransactionFilter::Date::AsOfToday);
    insertItem(i18n("Today"), (int)TransactionFilter::Date::Today);
    insertItem(i18n("Current month"), (int)TransactionFilter::Date::CurrentMonth);
    insertItem(i18n("Current quarter"), (int)TransactionFilter::Date::CurrentQuarter);
    insertItem(i18n("Current year"), (int)TransactionFilter::Date::CurrentYear);
    insertItem(i18n("Current fiscal year"), (int)TransactionFilter::Date::CurrentFiscalYear);
    insertItem(i18n("Month to date"), (int)TransactionFilter::Date::MonthToDate);
    insertItem(i18n("Year to date"), (int)TransactionFilter::Date::YearToDate);
    insertItem(i18n("Year to month"), (int)TransactionFilter::Date::YearToMonth);
    insertItem(i18n("Last month"), (int)TransactionFilter::Date::LastMonth);
    insertItem(i18n("Last year"), (int)TransactionFilter::Date::LastYear);
    insertItem(i18n("Last fiscal year"), (int)TransactionFilter::Date::LastFiscalYear);
    insertItem(i18n("Last 7 days"), (int)TransactionFilter::Date::Last7Days);
    insertItem(i18n("Last 30 days"), (int)TransactionFilter::Date::Last30Days);
    insertItem(i18n("Last 3 months"), (int)TransactionFilter::Date::Last3Months);
    insertItem(i18n("Last quarter"), (int)TransactionFilter::Date::LastQuarter);
    insertItem(i18n("Last 6 months"), (int)TransactionFilter::Date::Last6Months);
    insertItem(i18n("Last 11 months"), (int)TransactionFilter::Date::Last11Months);
    insertItem(i18n("Last 12 months"), (int)TransactionFilter::Date::Last12Months);
    insertItem(i18n("Next 7 days"), (int)TransactionFilter::Date::Next7Days);
    insertItem(i18n("Next 30 days"), (int)TransactionFilter::Date::Next30Days);
    insertItem(i18n("Next 3 months"), (int)TransactionFilter::Date::Next3Months);
    insertItem(i18n("Next quarter"), (int)TransactionFilter::Date::NextQuarter);
    insertItem(i18n("Next 6 months"), (int)TransactionFilter::Date::Next6Months);
    insertItem(i18n("Next 12 months"), (int)TransactionFilter::Date::Next12Months);
    insertItem(i18n("Next 18 months"), (int)TransactionFilter::Date::Next18Months);
    insertItem(i18n("Last 3 months to next 3 months"), (int)TransactionFilter::Date::Last3ToNext3Months);
    insertItem(i18n("User defined"), (int)TransactionFilter::Date::UserDefined);
}

KMyMoneyPeriodCombo::~KMyMoneyPeriodCombo()
{
}

void KMyMoneyPeriodCombo::setCurrentItem(TransactionFilter::Date id)
{
    if (id >= TransactionFilter::Date::LastDateItem)
        id = TransactionFilter::Date::UserDefined;

    KMyMoneyGeneralCombo::setCurrentItem((int)id);
}

TransactionFilter::Date KMyMoneyPeriodCombo::currentItem() const
{
    return static_cast<TransactionFilter::Date>(KMyMoneyGeneralCombo::currentItem());
}

QDate KMyMoneyPeriodCombo::start(TransactionFilter::Date id)
{
    QDate start, end;
    MyMoneyTransactionFilter::translateDateRange(id, start, end);
    return start;
}

QDate KMyMoneyPeriodCombo::end(TransactionFilter::Date id)
{
    QDate start, end;
    MyMoneyTransactionFilter::translateDateRange(id, start, end);
    return end;
}

#if 0
void KMyMoneyPeriodCombo::dates(QDate& start, QDate& end, MyMoneyTransactionFilter::dateOptionE id)
{
}
#endif
