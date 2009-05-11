/***************************************************************************
                          mymoneyobservertest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYOBSERVERTEST_H__
#define __MYMONEYOBSERVERTEST_H__

#include <cppunit/extensions/HelperMacros.h>

class TestObserver : public MyMoneyObserver
{
public:
        TestObserver() { m_updated = ""; }
        void update(const QString& id) { m_updated = id; };
        const QString& updated(void) { return m_updated; };
        void reset(void) { m_updated = ""; };
private:
        QString m_updated;
};

class TestSubject : public MyMoneySubject
{
};

class MyMoneyObserverTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyObserverTest);
        CPPUNIT_TEST(testEmptySubject);
        CPPUNIT_TEST(testAddObserver);
        CPPUNIT_TEST(testRemoveObserver);
        CPPUNIT_TEST(testNotifyObserver);
        CPPUNIT_TEST(testNotifyMultipleObserver);
        CPPUNIT_TEST_SUITE_END();

protected:
        TestSubject *subject;
        TestObserver *observer1;
        TestObserver *observer2;

public:
        MyMoneyObserverTest () {}


void setUp () {
        subject = new TestSubject;
        observer1 = new TestObserver;
        observer2 = new TestObserver;
}

void tearDown () {
        delete observer1;
        delete observer2;
        delete subject;
}

void testEmptySubject() {
        CPPUNIT_ASSERT(subject->m_observers.count() == 0);
}

void testAddObserver() {
        subject->attach(observer1);
        CPPUNIT_ASSERT(subject->m_observers.count() == 1);
        CPPUNIT_ASSERT(subject->m_observers.at(0) == observer1);
}

void testRemoveObserver() {
        testAddObserver();
        subject->detach(observer1);
        CPPUNIT_ASSERT(subject->m_observers.count() == 0);
}

void testNotifyObserver() {
        testAddObserver();
        CPPUNIT_ASSERT(observer1->updated() == "");
        subject->notify("my id");
        CPPUNIT_ASSERT(observer1->updated() == "my id");
}

void testNotifyMultipleObserver() {
        testAddObserver();
        subject->attach(observer2);
        CPPUNIT_ASSERT(subject->m_observers.count() == 2);
        CPPUNIT_ASSERT(subject->m_observers.at(0) == observer1);
        CPPUNIT_ASSERT(subject->m_observers.at(1) == observer2);

        CPPUNIT_ASSERT(observer1->updated() == "");
        CPPUNIT_ASSERT(observer2->updated() == "");
        subject->notify("my id");
        CPPUNIT_ASSERT(observer1->updated() == "my id");
        CPPUNIT_ASSERT(observer2->updated() == "my id");
}

};

#endif
