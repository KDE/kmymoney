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

#include "kdganttconstraintgraphicsitem.h"
#include "kdganttconstraintmodel.h"
#include "kdganttgraphicsscene.h"
#include "kdganttitemdelegate.h"
#include "kdganttsummaryhandlingproxymodel.h"

#include <QPainter>
#include <QDebug>

using namespace KDGantt;

/*!\class KDGantt::ConstraintGraphicsItem
 * \internal
 */
#if QT_VERSION < 0x050000
ConstraintGraphicsItem::ConstraintGraphicsItem( const Constraint& c, QGraphicsItem* parent, GraphicsScene* scene )
    : QGraphicsItem( parent, scene ),  m_constraint( c )
{
    setPos( QPointF( 0., 0. ) );
    setAcceptsHoverEvents( false );
    setAcceptedMouseButtons( Qt::NoButton );
    setZValue( 10. );
}
#else
ConstraintGraphicsItem::ConstraintGraphicsItem( const Constraint& c, QGraphicsItem* parent, GraphicsScene* scene )
    : QGraphicsItem( parent ),  m_constraint( c )
{
    if ( scene )
        scene->addItem( this );
    setPos( QPointF( 0., 0. ) );
    setAcceptHoverEvents( false );
    setAcceptedMouseButtons( Qt::NoButton );
    setZValue( 10. );
}
#endif

ConstraintGraphicsItem::~ConstraintGraphicsItem()
{
}

int ConstraintGraphicsItem::type() const
{
    return Type;
}

GraphicsScene* ConstraintGraphicsItem::scene() const
{
    return qobject_cast<GraphicsScene*>( QGraphicsItem::scene() );
}

Constraint ConstraintGraphicsItem::proxyConstraint() const
{
    return Constraint( scene()->summaryHandlingModel()->mapFromSource( m_constraint.startIndex() ),
                       scene()->summaryHandlingModel()->mapFromSource( m_constraint.endIndex() ),
                       m_constraint.type(), m_constraint.relationType(), m_constraint.dataMap() );
}

QRectF ConstraintGraphicsItem::boundingRect() const
{
    return scene()->itemDelegate()->constraintBoundingRect( m_start, m_end, m_constraint );
}

void ConstraintGraphicsItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option,
                                    QWidget* widget )
{
    Q_UNUSED( widget );
    //qDebug() << "ConstraintGraphicsItem::paint(...), c=" << m_constraint;
    scene()->itemDelegate()->paintConstraintItem( painter, *option, m_start, m_end, m_constraint );
}

QString ConstraintGraphicsItem::ganttToolTip() const
{
    return m_constraint.data( Qt::ToolTipRole ).toString();
}

void ConstraintGraphicsItem::setStart( const QPointF& start )
{
    prepareGeometryChange();
    m_start = start;
    update();
}

void ConstraintGraphicsItem::setEnd( const QPointF& end )
{
    prepareGeometryChange();
    m_end = end;
    update();
}

void ConstraintGraphicsItem::updateItem( const QPointF& start,const QPointF& end )
{
    setStart( start );
    setEnd( end );
}
