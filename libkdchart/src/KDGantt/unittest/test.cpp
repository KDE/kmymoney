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

#include "test.h"

#ifdef TEMPORARILY_REMOVED
#include <trinav/ascshared/libfakes/fakes.h>
#endif

#include <cmath>
#include <limits>

KDAB::UnitTest::Test::Test( const std::string & n )
    : mName( n ), mFailed( 0 ), mSucceeded( 0 ) {}

void KDAB::UnitTest::Test::_assertNotNull( const void * x, const char * expression, const char * file, unsigned int line ) {
    if ( x ) success();
    else fail( file, line ) << '"' << expression << "\" is NULL, expected non-NULL" << std::endl;
}

void KDAB::UnitTest::Test::_assertNull( const void * x, const char * expression, const char * file, unsigned int line ) {
    if ( !x ) success();
    else fail( file, line ) << '"' << expression << "\" is not NULL, expected NULL" << std::endl;
}

#if 0
void KDAB::UnitTest::Test::_assertIsNaN( qreal d, const char * expression, const char * file, unsigned int line ) {
    if ( std::isnan( d ) ) success();
    else fail( file, line ) << '"' << expression << "\" yielded " << d << "; expected NaN" << std::endl;
}

void KDAB::UnitTest::Test::_assertIsNotNaN( qreal d, const char * expression, const char * file, unsigned int line ) {
    if ( !std::isnan( d ) ) success();
    else fail( file, line ) << '"' << expression << "\" yielded nan; expected something else" << std::endl;
}
#endif

void KDAB::UnitTest::Test::_assertTrue( bool x, const char * expression, const char * file, unsigned int line ) {
    if ( x ) success();
    else fail( file, line ) << '"' << expression << "\" != TRUE" << std::endl;
}

void KDAB::UnitTest::Test::_assertFalse( bool x, const char * expression, const char * file, unsigned int line ) {
    if ( !x ) success();
    else fail( file, line ) << '"' << expression << "\" != FALSE" << std::endl;
}

void KDAB::UnitTest::Test::_assertEqualWithEpsilons( float x1, float x2, int prec, const char * expr1, const char * expr2, const char * exprP, const char * file, unsigned int line ) {
    if ( qAbs( x1/x2 - 1.0f ) <= prec * std::numeric_limits<float>::epsilon() ) success();
    else fail( file, line ) << x1 << " (" << expr1 << ") deviates from expected "
                            << x2 << " (" << expr2 << ") by more than "
                            << prec << " (" << exprP << ") epsilons." << std::endl;
}

void KDAB::UnitTest::Test::_assertEqualWithEpsilons( qreal x1, qreal x2, int prec, const char * expr1, const char * expr2, const char * exprP, const char * file, unsigned int line ) {
    if ( qAbs( x1/x2 - 1.0 ) <= prec * std::numeric_limits<qreal>::epsilon() ) success();
    else fail( file, line ) << x1 << " (" << expr1 << ") deviates from expected "
                            << x2 << " (" << expr2 << ") by more than "
                            << prec << " (" << exprP << ") epsilons." << std::endl;
}

void KDAB::UnitTest::Test::_assertEqualWithEpsilons( long double x1, long double x2, int prec, const char * expr1, const char * expr2, const char * exprP, const char * file, unsigned int line ) {
    if ( qAbs( x1/x2 - 1.0l ) <= prec * std::numeric_limits<long double>::epsilon() ) success();
    else fail( file, line ) << x1 << " (" << expr1 << ") deviates from expected "
                            << x2 << " (" << expr2 << ") by more than "
                            << prec << " (" << exprP << ") epsilons." << std::endl;
}

std::ostream & KDAB::UnitTest::Test::fail( const char * file, unsigned int line ) {
    ++mFailed;
    return std::cerr << "FAIL: " << file << ':' << line << ": ";
}

#endif // KDAB_NO_UNIT_TESTS
