/*
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2007-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
