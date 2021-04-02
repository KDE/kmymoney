/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XMLSTORAGEHELPER_H
#define XMLSTORAGEHELPER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyReport;
class MyMoneyBudget;

class QDomDocument;
class QDomElement;

namespace MyMoneyXmlContentHandler2 {
MyMoneyReport readReport(const QDomElement &node);
void writeReport(const MyMoneyReport &report, QDomDocument &document, QDomElement &parent);

MyMoneyBudget readBudget(const QDomElement &node);
void writeBudget(const MyMoneyBudget &budget, QDomDocument &document, QDomElement &parent);
}

#endif
