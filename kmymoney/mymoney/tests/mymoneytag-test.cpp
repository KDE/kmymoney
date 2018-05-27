/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
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

#include "mymoneytag-test.h"

#include <QDomDocument>
#include <QDomElement>

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyTagTest;

#include "mymoneytag.h"
#include "mymoneytag_p.h"

using namespace std;

QTEST_GUILESS_MAIN(MyMoneyTagTest)

void MyMoneyTagTest::testXml()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyTag tag1;
  tag1.d_func()->m_id = "some random id";//if the ID isn't set, w ethrow an exception
  tag1.writeXML(doc, parent);
  QDomElement el = parent.firstChild().toElement();
  QVERIFY(!el.isNull());
  MyMoneyTag tag2(el);
}

void MyMoneyTagTest::testAttributeNames()
{
  for (auto i = (int)Tag::Attribute::Name; i < (int)Tag::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyTagPrivate::getAttrName(static_cast<Tag::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
