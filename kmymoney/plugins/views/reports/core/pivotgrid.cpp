/*
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <pivotgrid.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QVariant>

// ----------------------------------------------------------------------------
// KDE Includes

#include <alkimia/alkdom.h>

// ----------------------------------------------------------------------------
// Project Includes

namespace reports {

const unsigned PivotOuterGroup::m_kDefaultSortOrder = 100;

PivotCell::PivotCell(const MyMoneyMoney& value)
    : MyMoneyMoney(value)
    , m_stockSplit(MyMoneyMoney::ONE)
    , m_cellUsed(!value.isZero())
{
}

PivotCell::PivotCell(const MyMoneyMoney& value, const QDate& date)
    : MyMoneyMoney(value)
    , m_stockSplit(MyMoneyMoney::ONE)
    , m_cellUsed(!value.isZero())
    , m_valueDate(date)
{
}

PivotCell::~PivotCell()
{
}

PivotCell PivotCell::operator += (const PivotCell& right)
{
    const MyMoneyMoney& r = static_cast<const MyMoneyMoney&>(right);
    *this += r;
    m_postSplit = m_postSplit * right.m_stockSplit;
    m_stockSplit = m_stockSplit * right.m_stockSplit;
    m_postSplit += right.m_postSplit;
    m_cellUsed |= right.m_cellUsed;
    return *this;
}

PivotCell PivotCell::operator += (const MyMoneyMoney& value)
{
    m_cellUsed |= !value.isZero();
    if (m_stockSplit != MyMoneyMoney::ONE)
        m_postSplit += value;
    else
        MyMoneyMoney::operator += (value);
    return *this;
}

PivotCell PivotCell::stockSplit(const MyMoneyMoney& factor)
{
    PivotCell s;
    s.m_stockSplit = factor;
    return s;
}

PivotCell PivotCell::stockSplit(const MyMoneyMoney& factor, const QDate& date)
{
    PivotCell s;
    s.m_stockSplit = factor;
    s.m_valueDate = date;
    return s;
}

const QString PivotCell::formatMoney(int fraction, bool showThousandSeparator) const
{
    return formatMoney(QString(), MyMoneyMoney::denomToPrec(fraction), showThousandSeparator);
}

const QString PivotCell::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
    // construct the result
    MyMoneyMoney res = (*this * m_stockSplit) + m_postSplit;
    return res.formatMoney(currency, prec, showThousandSeparator);
}

MyMoneyMoney PivotCell::calculateRunningSum(const MyMoneyMoney& runningSum)
{
    MyMoneyMoney::operator += (runningSum);
    MyMoneyMoney::operator = ((*this * m_stockSplit) + m_postSplit);
    m_postSplit = MyMoneyMoney();
    m_stockSplit = MyMoneyMoney::ONE;
    return *this;
}

MyMoneyMoney PivotCell::cellBalance(const MyMoneyMoney& _balance)
{
    MyMoneyMoney balance(_balance);
    balance += *this;
    balance = (balance * m_stockSplit) + m_postSplit;
    return balance;
}

bool PivotCell::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotCell");
    el.setAttribute("value", toString());
    el.setAttribute("isUsed", isUsed());
    el.setAttribute("stockSplit", m_stockSplit.toString());
    el.setAttribute("postSplit", m_postSplit.toString());
    el.setAttribute("date", m_valueDate.toString(Qt::ISODate));
    parent.appendChild(el);
    return true;
}

PivotGridRowSet::PivotGridRowSet(unsigned _numcolumns)
{
    insert(eActual, PivotGridRow(_numcolumns));
    insert(eBudget, PivotGridRow(_numcolumns));
    insert(eBudgetDiff, PivotGridRow(_numcolumns));
    insert(eForecast, PivotGridRow(_numcolumns));
    insert(eAverage, PivotGridRow(_numcolumns));
    insert(ePrice, PivotGridRow(_numcolumns));
}

PivotGridRowSet PivotGrid::rowSet(QString id)
{

    //go through the data and get the row that matches the id
    PivotGrid::iterator it_outergroup = begin();
    while (it_outergroup != end()) {
        PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
        while (it_innergroup != (*it_outergroup).end()) {
            PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
            while (it_row != (*it_innergroup).end()) {
                if (it_row.key().id() == id)
                    return it_row.value();

                ++it_row;
            }
            ++it_innergroup;
        }
        ++it_outergroup;
    }
    return PivotGridRowSet();
}

QStringList getKey(const ERowType& f)
{
    return QStringList() << "id" << QVariant::fromValue(f).toString();
}

QStringList getKey(const QString& f)
{
    return QStringList() << "id" << f;
}

QStringList getKey(const ReportAccount& f)
{
    return QStringList() << "id" << f.id();
}

template<class K, class V>
AlkDomElement createElement(AlkDomDocument& doc, const QString& mapName, const QMap<K, V>& map, const QString& keyType)
{
    AlkDomElement me = doc.createElement(mapName);
    for (const auto& key : map.keys()) {
        AlkDomElement mapEntry = doc.createElement(QString("%1Entry").arg(mapName));
        AlkDomElement mapKey = doc.createElement(QString("%1Key").arg(mapName));
        AlkDomElement entryKey = doc.createElement(keyType);
        QStringList keys = getKey(key);
        entryKey.setAttribute(keys[0], keys[1]);
        mapKey.appendChild(entryKey);
        mapEntry.appendChild(mapKey);
        AlkDomElement mapValue = doc.createElement(QString("%1Value").arg(mapName));
        map[key].saveToXml(doc, mapValue);
        mapEntry.appendChild(mapValue);
        me.appendChild(mapEntry);
    }
    return me;
}

bool PivotGridRowSet::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotGridRowSet");
    const QString mapName = "PivotGridRowSetMap";
    AlkDomElement map = createElement<ERowType, PivotGridRow>(doc, mapName, *this, "ERowType");
    el.appendChild(map);
    parent.appendChild(el);
    return true;
}

bool PivotGridRow::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotGridRow");
    el.setAttribute("total", m_total.toString());
    const QString listName = "PivotGridRowList";
    AlkDomElement list = doc.createElement(QString("%1").arg(listName));
    for (int i = 0; i < size(); i++) {
        const PivotCell& cell = at(i);
        cell.saveToXml(doc, list);
    }
    el.appendChild(list);
    parent.appendChild(el);
    return true;
}

bool PivotInnerGroup::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotInnerGroup");
    m_total.saveToXml(doc, el);
    AlkDomElement map = createElement<ReportAccount, PivotGridRowSet>(doc, "PivotInnerGroupMap", *this, "ReportAccount");
    el.appendChild(map);
    parent.appendChild(el);
    return true;
}

bool PivotOuterGroup::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotOuterGroup");
    m_total.saveToXml(doc, el);
    el.setAttribute("inverted", m_inverted);
    el.setAttribute("displayName", m_displayName);
    el.setAttribute("sortOrder", m_sortOrder);
    AlkDomElement map = createElement<QString, PivotInnerGroup>(doc, "PivotOuterGroupMap", *this, "String");
    el.appendChild(map);
    parent.appendChild(el);
    return true;
}

bool PivotGrid::saveToXml(AlkDomDocument& doc, AlkDomElement& parent) const
{
    AlkDomElement el = doc.createElement("PivotGrid");
    AlkDomElement map = createElement<QString, PivotOuterGroup>(doc, "PivotGridMap", *this, "String");
    el.appendChild(map);
    parent.appendChild(el);
    return true;
}

} // namespace
