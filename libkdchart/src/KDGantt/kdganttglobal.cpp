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

#include "kdganttglobal.h"

using namespace KDGantt;

/* Older Qt don't have this macro, so define it... */
#ifndef QT_VERSION_CHECK
#  define QT_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#endif

/* Version check */
#if QT_VERSION < QT_VERSION_CHECK(4,3,0)
#  error "The minimum required version of Qt for KD Gantt is 4.3.0"
#endif

/*!\enum KDGantt::ItemDataRole
 *\ingroup KDGantt
 * The values of this enum are used for the special data roles
 * for gantt items
 */

/*!\enum KDGantt::ItemDataRole KDGantt::KDGanttRoleBase
 * The base value used for the KDGantt role enum values.
 * The actual roles have values base+1, base+2, ...
 */

/*!\enum KDGantt::ItemDataRole KDGantt::StartTimeRole
 * Start time (or other start value) for a gantt item.
 */

/*!\enum KDGantt::ItemDataRole KDGantt::EndTimeRole
 * End time (or other end value) for a gantt item.
 */

/*!\enum KDGantt::ItemDataRole KDGantt::TaskCompletionRole
 * Task completetion percentage used by Task items. Should be an
 * integer og a qreal between 0 and 100.
 */

/*!\enum KDGantt::ItemDataRole KDGantt::ItemTypeRole
 * The item type. \see KDGantt::ItemType.
 */

/*!\enum KDGantt::ItemDataRole KDGantt::TextPositionRole
 * The position of the text label on the item.
 * The type of this value is KDGantt::StyleOptionGanttItem::Position
 * and the default values is Right.
 */

/*!\enum KDGantt::ItemType
 *\ingroup KDGantt
 * The values of this enum are used to represent the different
 * types of gantt items that KDGantt understands. The itemtype
 * is served through the KDGantt::ItemTypeRole role
 */

/*!\class KDGantt::Span kdganttglobal.h KDGanttGlobal
 *\ingroup KDGantt
 * \brief A class representing a start point and a length */

DateTimeSpan::DateTimeSpan()
{
}

DateTimeSpan::DateTimeSpan( const QDateTime& start, const QDateTime& end )
    : m_start( start ), m_end( end )
{
}

DateTimeSpan::DateTimeSpan( const DateTimeSpan& other )
{
    *this = other;
}

DateTimeSpan::~DateTimeSpan()
{
}

DateTimeSpan& DateTimeSpan::operator=( const DateTimeSpan& other )
{
    if ( this != &other ) {
        m_start = other.m_start;
        m_end = other.m_end;
    }
    return *this;
}

bool DateTimeSpan::isValid() const
{
    return m_start.isValid() && m_end.isValid();
}

bool DateTimeSpan::equals( const DateTimeSpan& other ) const
{
    return m_start==other.m_start && m_end==other.m_end;
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<( QDebug dbg, KDGantt::ItemDataRole r)
{
  switch (r) {
  case KDGantt::StartTimeRole:      dbg << "KDGantt::StartTimeRole"; break;
  case KDGantt::EndTimeRole:        dbg << "KDGantt::EndTimeRole"; break;
  case KDGantt::TaskCompletionRole: dbg << "KDGantt::TaskCompletionRole"; break;
  case KDGantt::ItemTypeRole:       dbg << "KDGantt::ItemTypeRole"; break;
  case KDGantt::LegendRole:         dbg << "KDGantt::LegendRole"; break;
  default: dbg << static_cast<Qt::ItemDataRole>(r);
  }
  return dbg;
}

QDebug operator<<( QDebug dbg, KDGantt::ItemType t)
{
    switch ( t ) {
    case KDGantt::TypeNone:        dbg << "KDGantt::TypeNone"; break;
    case KDGantt::TypeEvent:       dbg << "KDGantt::TypeEvent"; break;
    case KDGantt::TypeTask:        dbg << "KDGantt::TypeTask"; break;
    case KDGantt::TypeSummary:     dbg << "KDGantt::TypeSummary"; break;
    case KDGantt::TypeMulti:       dbg << "KDGantt::TypeMulti"; break;
    case KDGantt::TypeUser:        dbg << "KDGantt::TypeUser"; break;
    default: dbg << static_cast<int>(t);
    }
    return dbg;
}

QDebug operator<<( QDebug dbg, const KDGantt::Span& s )
{
    dbg << "KDGantt::Span[ start="<<s.start()<<" length="<<s.length()<<"]";
    return dbg;
}
QDebug operator<<( QDebug dbg, const KDGantt::DateTimeSpan& s )
{
    dbg << "KDGantt::DateTimeSpan[ start="<<s.start()<<" end="<<s.end()<<"]";
    return dbg;
}

#endif /* QT_NO_DEBUG_STREAM */

#ifndef KDAB_NO_UNIT_TESTS

#include <ostream>

static std::ostream& operator<<( std::ostream& os, const Span& span )
{
    os << "Span[ start="<<span.start()<<", length="<<span.length()<<"]";
    return os;
}

static std::ostream& operator<<( std::ostream& os, const DateTimeSpan& span )
{
#ifdef QT_NO_STL
    os << "DateTimeSpan[ start="<<span.start().toString().toLatin1().constData()
       << ", end="<<span.end().toString().toLatin1().constData() << "]";
#else
    os << "DateTimeSpan[ start="<<span.start().toString().toStdString()
       << ", end="<<span.end().toString().toStdString() << "]";
#endif
    return os;
}

#include "unittest/test.h"

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, Span, "test" ) {
    Span s1;
    assertFalse( s1.isValid() );
    s1.setStart( 10. );
    s1.setLength( 2. );

    Span s2( s1.start(), s1.length() );
    assertEqual( s1, s2 );
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, DateTimeSpan, "test" ) {
    DateTimeSpan s1;
    assertFalse( s1.isValid() );
    QDateTime dt = QDateTime::currentDateTime();
    s1.setStart( dt );
    assertTrue( dt.isValid() );
    s1.setEnd( dt.addDays( 1 ) );

    DateTimeSpan s2( dt, dt.addDays( 1 ) );

    assertEqual( s1, s2 );

    DateTimeSpan s3;

    assertNotEqual( s1, s3 );
}
#endif /* KDAB_NO_UNIT_TESTS */
