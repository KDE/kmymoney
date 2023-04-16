/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XMLSTORAGEHELPER_H
#define XMLSTORAGEHELPER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QXmlStreamReader>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class MyMoneyReport;
class MyMoneyBudget;
class MyMoneyMoney;

class QDomDocument;
class QDomElement;

namespace Element {

enum class Report {
    Payee,
    Tag,
    Account,
    Text,
    Type,
    State,
    Number,
    Amount,
    Dates,
    Category,
    AccountGroup,
    Validity,
    // insert new entries above this line
    LastElement,
};

enum class Budget {
    Budget = 0,
    Account,
    Period,
    // insert new entries above this line
    LastElement,
};
}

namespace Attribute {

enum class Report {
    ID = 0,
    Group,
    Type,
    Name,
    Comment,
    ConvertCurrency,
    Favorite,
    SkipZero,
    DateLock,
    DataLock,
    MovingAverageDays,
    IncludesActuals,
    IncludesForecast,
    IncludesPrice,
    IncludesAveragePrice,
    IncludesMovingAverage,
    IncludesSchedules,
    IncludesTransfers,
    IncludesUnused,
    MixedTime,
    Investments,
    Budget,
    ShowRowTotals,
    ShowColumnTotals,
    Detail,
    ColumnsAreDays,
    ChartType,
    ChartCHGridLines,
    ChartSVGridLines,
    ChartDataLabels,
    ChartByDefault,
    LogYAxis,
    ChartLineWidth,
    ColumnType,
    RowType,
    DataRangeStart,
    DataRangeEnd,
    DataMajorTick,
    DataMinorTick,
    YLabelsPrecision,
    QueryColumns,
    Tax,
    PropagateBudgetDiff,
    Loans,
    HideTransactions,
    InvestmentSum,
    SettlementPeriod,
    ShowSTLTCapitalGains,
    TermsSeparator,
    Pattern,
    CaseSensitive,
    RegEx,
    InvertText,
    State,
    From,
    To,
    NegExpenses,
    Validity,
    ChartPalette,
    // insert new entries above this line
    LastAttribute,
};

enum class Budget {
    ID = 0,
    Name,
    Start,
    Version,
    BudgetLevel,
    BudgetSubAccounts,
    Amount,
    BudgetType,
    // insert new entries above this line
    LastAttribute,
};
}

namespace MyMoneyXmlHelper {

enum class IdRequirement { Optional, Required };

QString attributeName(Attribute::Report attributeID);
QString elementName(Element::Report elementID);
QString elementName(Element::Budget elementID);
QString typeAttributeToString(int textID);
QString stateAttributeToString(int textID);
QString validityAttributeToString(int textID);
QString accountTypeAttributeToString(eMyMoney::Account::Type type);
QString attributeName(Attribute::Budget attributeID);
QString budgetLevels(eMyMoney::Budget::Level textID);

QString dateLockAttributeToString(eMyMoney::TransactionFilter::Date textID);
QString reportNames(eMyMoney::Report::DataLock textID);
QString reportNames(eMyMoney::Report::RowType textID);
QString reportNames(eMyMoney::Report::ColumnType textID);
QString reportNamesForQC(eMyMoney::Report::QueryColumn textID);
QString reportNames(eMyMoney::Report::DetailLevel textID);
QString reportNames(eMyMoney::Report::ChartType textID);
QString reportNames(eMyMoney::Report::ChartPalette textID);

QString readRequiredStringAttribute(QXmlStreamReader* reader, const QString& attribute);
QString readStringAttribute(QXmlStreamReader* reader, const QString& attribute, const QString& defaultValue = QString());
MyMoneyMoney readValueAttribute(QXmlStreamReader* reader, const QString& attribute);
uint readUintAttribute(QXmlStreamReader* reader, const QString& attribute, uint defaultValue = 0, int base = 0);
QDate readDateAttribute(QXmlStreamReader* reader, const QString& attribute);
QDateTime readDateTimeAttribute(QXmlStreamReader* reader, const QString& attribute);
QString readId(QXmlStreamReader* reader, IdRequirement idRequirement = IdRequirement::Required);
bool readBoolAttribute(QXmlStreamReader* reader, const QString& attribute, bool defaultValue = false);

void writeStartObject(QXmlStreamWriter* writer, const QString tagName, const QString& id);

MyMoneyReport readReport(QXmlStreamReader* reader);
MyMoneyBudget readBudget(QXmlStreamReader* reader);

void writeReport(const MyMoneyReport& report, QXmlStreamWriter* writer);
void writeBudget(const MyMoneyBudget& budget, QXmlStreamWriter* writer);
};

#endif
