/***************************************************************************
                          mymoneytagtest.cpp
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneytag-test.h"

#include <QDomDocument>
#include <QDomElement>

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyTagTest;

#include "mymoneytag.h"

using namespace std;

QTEST_GUILESS_MAIN(MyMoneyTagTest)

void MyMoneyTagTest::testXml()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyTag tag1;
  tag1.m_id = "some random id";//if the ID isn't set, w ethrow an exception
  tag1.writeXML(doc, parent);
  QDomElement el = parent.firstChild().toElement();
  QVERIFY(!el.isNull());
  MyMoneyTag tag2(el);
}

void MyMoneyTagTest::testAttributeNames()
{
  for (auto i = (int)MyMoneyTag::Attribute::Name; i < (int)MyMoneyTag::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyTag::getAttrName(static_cast<MyMoneyTag::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
