/***************************************************************************
                          autotest.cpp
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

#include "config-kmymoney.h"

#include <iostream>
#include <string>
#include <stdexcept>

/* required for Q_UNUSED( ) */
#include <qglobal.h>

#ifdef HAVE_LIBCPPUNIT

#include <kcmdlineargs.h>
#include <kapplication.h>

#include "cppunit/TextTestRunner.h"
#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"

#include "mymoney/mymoneyutils.h"

#define private public
#include "mymoney/mymoneysubject.h"
#include "mymoney/mymoneyobserver.h"
#undef private

#include "mymoney/mymoneyexceptiontest.h"
#include "mymoney/mymoneymoneytest.h"
#include "mymoney/mymoneyobservertest.h"
#include "mymoney/mymoneyinstitutiontest.h"
#include "mymoney/mymoneysplittest.h"
#include "mymoney/mymoneyaccounttest.h"
#include "mymoney/mymoneytransactiontest.h"
#include "mymoney/storage/mymoneyseqaccessmgrtest.h"
#include "mymoney/storage/mymoneydatabasemgrtest.h"
#include "mymoney/mymoneyfiletest.h"
#include "mymoney/mymoneykeyvaluecontainertest.h"
#include "mymoney/mymoneyscheduletest.h"
#include "mymoney/mymoneyfinancialcalculatortest.h"
#include "mymoney/mymoneysecuritytest.h"
#include "mymoney/mymoneypricetest.h"
#include "mymoney/mymoneyobjecttest.h"
#include "mymoney/mymoneyforecasttest.h"
#include "mymoney/mymoneypayeetest.h"

#include "mymoney/storage/mymoneymaptest.h"

#include "reports/pivottabletest.h"
#include "reports/pivotgridtest.h"
#include "reports/querytabletest.h"

#include "converter/convertertest.h"

#include "cppunit/TextTestProgressListener.h"

class MyProgressListener : public CppUnit::TextTestProgressListener
{
  void startTest(CppUnit::Test *test) {
    QString name = test->getName().c_str();
    if(name.find('.') != -1) {    // in CPPUNIT 1.8.0
      name = name.mid(2);   // cut off first 2 chars
      name = name.left(name.find('.'));
    } else if(name.find("::") != -1) {  // in CPPUNIT 1.9.14
      name = name.left(name.find("::"));
    }
    if(m_name != name) {
      if(m_name != "")
        std::cout << std::endl;
      std::cout << "Running: " << name << std::endl;
      m_name = name;
    }
  }
private:
  QString m_name;
};

void unexpectedException(MyMoneyException *e)
{
  std::string msg = "Unexpected exception: ";
  msg += e->what().toLatin1();
  msg += " thrown in ";
  msg += e->file().toLatin1();
  msg += ":";
  char line[8];
  sprintf(line, "%ld", e->line());
  msg += line;
  delete e;
  CPPUNIT_FAIL(msg);
}

#endif // HAVE_LIBCPPUNIT

int main(int testargc, char** testargv)
{
  int rc = 0;

#ifdef HAVE_LIBCPPUNIT
  static const KCmdLineOptions options[] =
  {
     { "+[test_suite]", ("Optionally specify a test suite"), 0 },
     { "", ("Optional arguments are for ctest"), 0 },
     KCmdLineLastOption // End of options.
  };

  // we seem to need a KApplication object to use KGlobal::locale()
  KCmdLineArgs::init(testargc, testargv, testargv[0], "UNIT TESTS", "", "0.1");
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication::disableAutoDcopRegistration();
  KApplication app(false, false);

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif

  // mymoney tests
  //CPPUNIT_TEST_SUITE_REGISTRATION(KReportsViewTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyMapTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(ConverterTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyKeyValueContainerTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySplitTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyMoneyTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyAccountTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyScheduleTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyDatabaseMgrTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySeqAccessMgrTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyFileTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyObjectTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyInstitutionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyFinancialCalculatorTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyTransactionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySecurityTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyForecastTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyExceptionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyObserverTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyPriceTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyPayeeTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(PivotGridTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(PivotTableTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(QueryTableTest);

  // off we go
  CppUnit::TestFactoryRegistry &registry =
    CppUnit::TestFactoryRegistry::getRegistry();

  // run all tests if no test is specified on the command line
  // this way, CTest can perform each test individually
  CppUnit::Test *suite = registry.makeTest();
  if (testargc>1)
  {
    try
    {
      suite = suite->findTest(testargv[1]);
    }
    catch(const std::invalid_argument &ex)
    {
      // oh, cmake perfomed bad at guessing the correct test names.
      std::cout << ex.what() << std::endl;
      // we output that the test passed since the test is deactivated
      return 0;
    }
  }

  CppUnit::TextTestRunner* runner = new CppUnit::TextTestRunner();

  runner->addTest(suite);

  MyProgressListener progress;
  CppUnit::TestResultCollector result;

  runner->eventManager().addListener(&progress);
  runner->eventManager().addListener(&result);

  runner->run();
  std::cout << "Tests were run with CPPUNIT version " CPPUNIT_VERSION << std::endl;

  rc = result.wasSuccessful() ? 0 : 1;
  delete runner;

  // make sure to delete the singletons before we start memory checking
  // to avoid false error reports
  // delete MyMoneyFile::instance();

#ifdef _CHECK_MEMORY
  chkmem.CheckMemoryLeak( true );
  _CheckMemory_End();
#endif // _CHECK_MEMORY

#else
  std::cout << "libcppunit not installed. no automatic tests available."
     << std::endl;
#endif // HAVE_LIBCPPUNIT
  return rc;
}

// required for the testcases (mymoneystoragesql references it)
void timetrace(const char *txt)
{
  Q_UNUSED(txt);
}

#if 0
#ifdef HAVE_LIBOFX

// these symbols are needed when linking with libofx because it requires
// these global symbols as part of its callback interface
extern "C" {
  void ofx_proc_security_cb() {}
  void ofx_proc_transaction_cb() {}
  void ofx_proc_statement_cb() {}
  void ofx_proc_status_cb() {}
  void ofx_proc_account_cb() {}
}
#endif
#endif

