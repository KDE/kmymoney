/****************************************************************************
** Copyright (C) 2001-2016 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDAB_NO_UNIT_TESTS

#include "testregistry.h"

#include "test.h"

#include <memory>
#include <iostream>
#include <iomanip>
#include <cassert>

KDAB::UnitTest::TestRegistry::TestRegistry()
    : mTests()
{

}

KDAB::UnitTest::TestRegistry::~TestRegistry() {}

KDAB::UnitTest::TestRegistry * KDAB::UnitTest::TestRegistry::mSelf = 0;

// static
KDAB::UnitTest::TestRegistry * KDAB::UnitTest::TestRegistry::instance() {
    if ( !mSelf )
        mSelf = new TestRegistry;
    return mSelf;
}

// static
void KDAB::UnitTest::TestRegistry::deleteInstance() {
    delete mSelf; mSelf = 0;
}

void KDAB::UnitTest::TestRegistry::registerTestFactory( const TestFactory * tf, const char * group ) {
    assert( tf );
    mTests[group].push_back( tf );
}

unsigned int KDAB::UnitTest::TestRegistry::run() const {
  unsigned int failed = 0;
  for ( std::map< std::string, std::vector<const TestFactory*> >::const_iterator g = mTests.begin() ; g != mTests.end() ; ++g ) {
    std::cerr << "===== GROUP \"" << g->first << "\" =========" << std::endl;
    for ( std::vector<const TestFactory*>::const_iterator it = g->second.begin() ; it != g->second.end() ; ++it ) {
      std::auto_ptr<Test> t( (*it)->create() );
      assert( t.get() );
      std::cerr << "  === \"" << t->name() << "\" ===" << std::endl;
      t->run();
      std::cerr << "    Succeeded: " << std::setw( 4 ) << t->succeeded()
                << ";  failed: " << std::setw( 4 ) << t->failed() << std::endl;
      failed += t->failed();
    }
  }
  return failed;
}


unsigned int KDAB::UnitTest::TestRegistry::run( const char * group ) const {
  assert( group ); assert( *group );
  unsigned int failed = 0;
  const std::map< std::string, std::vector<const TestFactory*> >::const_iterator g = mTests.find( group );
  if ( g == mTests.end() ) {
    std::cerr << "ERROR: No such group \"" << group << "\"" << std::endl;
    return 1;
  }
  std::cerr << "===== GROUP \"" << g->first << "\" =========" << std::endl;
  for ( std::vector<const TestFactory*>::const_iterator it = g->second.begin() ; it != g->second.end() ; ++it ) {
    std::auto_ptr<Test> t( (*it)->create() );
    assert( t.get() );
    std::cerr << "  === \"" << t->name() << "\" ===" << std::endl;
    t->run();
    std::cerr << "    Succeeded: " << t->succeeded() << ";  failed: " << t->failed() << std::endl;
    failed += t->failed();
  }
  return failed;
}

KDAB::UnitTest::Runner::~Runner()
{
	TestRegistry::deleteInstance();
}

unsigned int KDAB::UnitTest::Runner::run( const char * group ) const
{
  if ( group && *group )
    return TestRegistry::instance()->run( group );
  else
    return TestRegistry::instance()->run();
}


#endif // KDAB_NO_UNIT_TESTS
