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

#include "kdganttitemdelegate_p.h"
#include "kdganttglobal.h"
#include "kdganttstyleoptionganttitem.h"
#include "kdganttconstraint.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QApplication>

#ifndef QT_NO_DEBUG_STREAM

#define PRINT_INTERACTIONSTATE(x) \
    case x: dbg << #x; break;


QDebug operator<<( QDebug dbg, KDGantt::ItemDelegate::InteractionState state )
{
    switch ( state ) {
        PRINT_INTERACTIONSTATE( KDGantt::ItemDelegate::State_None );
        PRINT_INTERACTIONSTATE( KDGantt::ItemDelegate::State_Move );
        PRINT_INTERACTIONSTATE( KDGantt::ItemDelegate::State_ExtendLeft );
        PRINT_INTERACTIONSTATE( KDGantt::ItemDelegate::State_ExtendRight );
    default:
        break;
    }
    return dbg;
}

#undef PRINT_INTERACTIONSTATE

#endif /* QT_NO_DEBUG_STREAM */

using namespace KDGantt;

/*!\class KDGantt::ItemDelegate kdganttitemdelegate.h KDGanttItemDelegate
 *\ingroup KDGantt
 *\brief Class used to render gantt items in a KDGantt::GraphicsView
 *
 */

/*!\enum KDGantt::ItemDelegate::InteractionState
 * This enum is used for communication between the view and
 * the delegate about user interaction with gantt items.
 *
 * \see KDGantt::ItemDelegate::interactionStateFor
 */

ItemDelegate::Private::Private()
{
    // Brushes
    QLinearGradient taskgrad( 0., 0., 0., QApplication::fontMetrics().height() );
    taskgrad.setColorAt( 0., Qt::green );
    taskgrad.setColorAt( 1., Qt::darkGreen );

    QLinearGradient summarygrad( 0., 0., 0., QApplication::fontMetrics().height() );
    summarygrad.setColorAt( 0., Qt::blue );
    summarygrad.setColorAt( 1., Qt::darkBlue );

    QLinearGradient eventgrad( 0., 0., 0., QApplication::fontMetrics().height() );
    eventgrad.setColorAt( 0., Qt::red );
    eventgrad.setColorAt( 1., Qt::darkRed );

    defaultbrush[TypeTask]    = taskgrad;
    defaultbrush[TypeSummary] = summarygrad;
    defaultbrush[TypeEvent]   = eventgrad;

    // Pens
    QPen pen( Qt::black, 1. );

    defaultpen[TypeTask]    = pen;
    defaultpen[TypeSummary] = pen;
    defaultpen[TypeEvent]   = pen;
}

QPen ItemDelegate::Private::constraintPen( const QPointF& start, const QPointF& end, const Constraint& constraint )
{
    QPen pen;
    QVariant dataPen;

    // Use default pens...
    if ( start.x() < end.x() ) {
        pen = QPen( Qt::black );
        dataPen = constraint.data( Constraint::ValidConstraintPen );
    } else {
        pen = QPen( Qt::red );
        dataPen = constraint.data( Constraint::InvalidConstraintPen );
    }

    // ... unless constraint.data() returned a valid pen for this case
    if ( dataPen.canConvert( QVariant::Pen ) ) {
        pen = dataPen.value< QPen >();
    }

    return pen;
}

/*! Constructor. Creates an ItemDelegate with parent \a parent */
ItemDelegate::ItemDelegate( QObject* parent )
    : QItemDelegate( parent ), _d( new Private )
{
}

/*! Destructor */
ItemDelegate::~ItemDelegate()
{
    delete _d;
}

#define d d_func()

/*! Sets the default brush used for items of type \a type to
 * \a brush. The default brush is used in the case when the model
 * does not provide an explicit brush.
 *
 * \todo Move this to GraphicsView to make delegate stateless.
 */
void ItemDelegate::setDefaultBrush( ItemType type, const QBrush& brush )
{
    d->defaultbrush[type] = brush;
}

/*!\returns The default brush for item type \a type
 *
 * \todo Move this to GraphicsView to make delegate stateless.
 */
QBrush ItemDelegate::defaultBrush( ItemType type ) const
{
    return d->defaultbrush[type];
}

/*! Sets the default pen used for items of type \a type to
 * \a pen. The default pen is used in the case when the model
 * does not provide an explicit pen.
 *
 * \todo Move this to GraphicsView to make delegate stateless.
 */
void ItemDelegate::setDefaultPen( ItemType type, const QPen& pen )
{
    d->defaultpen[type]=pen;
}

/*!\returns The default pen for item type \a type
 *
 * \todo Move this to GraphicsView to make delegate stateless.
 */
QPen ItemDelegate::defaultPen( ItemType type ) const
{
    return d->defaultpen[type];
}

/*!\returns The tooltip for index \a idx
 */
QString ItemDelegate::toolTip( const QModelIndex &idx ) const
{
    if ( !idx.isValid() ) return QString();

    const QAbstractItemModel* model = idx.model();
    if ( !model ) return QString();
    QString tip = model->data( idx, Qt::ToolTipRole ).toString();
    if ( !tip.isNull() ) return tip;
    else return tr( "%1 -> %2: %3" )
                .arg( model->data( idx, StartTimeRole ).toString() )
                .arg( model->data( idx, EndTimeRole ).toString() )
                .arg( model->data( idx, Qt::DisplayRole ).toString() );
}

/*! \returns The bounding Span for the item identified by \a idx
 * when rendered with options \a opt. This is often the same as the
 * span given by the AbstractGrid for \a idx, but it might be larger
 * in case there are additional texts or decorations on the item.
 *
 * Override this to implement new itemtypes or to change the look
 * of the existing ones.
 */
Span ItemDelegate::itemBoundingSpan( const StyleOptionGanttItem& opt,
                                 const QModelIndex& idx ) const
{
    if ( !idx.isValid() ) return Span();

    const QString txt = idx.model()->data( idx, Qt::DisplayRole ).toString();
    const int typ = idx.model()->data( idx, ItemTypeRole ).toInt();
    QRectF itemRect = opt.itemRect;


    if ( typ == TypeEvent ) {
        itemRect = QRectF( itemRect.left()-itemRect.height()/2.,
                           itemRect.top(),
                           itemRect.height(),
                           itemRect.height() );
    }

    int tw = opt.fontMetrics.width( txt );
    tw += static_cast<int>( itemRect.height()/2. );
    Span s;
    switch ( opt.displayPosition ) {
    case StyleOptionGanttItem::Left:
        s = Span( itemRect.left()-tw, itemRect.width()+tw ); break;
    case StyleOptionGanttItem::Right:
        s = Span( itemRect.left(), itemRect.width()+tw ); break;
    case StyleOptionGanttItem::Hidden: // fall through
    case StyleOptionGanttItem::Center:
        s = Span( itemRect.left(), itemRect.width() ); break;
    }
    return s;
}

/*! \returns The interaction state for position \a pos on item \a idx
 * when rendered with options \a opt. This is used to tell the view
 * about how the item should react to mouse click/drag.
 *
 * Override to implement new items or interactions.
 */
ItemDelegate::InteractionState ItemDelegate::interactionStateFor( const QPointF& pos,
								  const StyleOptionGanttItem& opt,
								  const QModelIndex& idx ) const
{
    if ( !idx.isValid() ) return State_None;
    if ( !( idx.model()->flags( idx ) & Qt::ItemIsEditable ) ) return State_None;

    const int typ = static_cast<ItemType>( idx.model()->data( idx, ItemTypeRole ).toInt() );

    QRectF itemRect( opt.itemRect );

    // An event item is infinitely thin, basically just a line, because it has only one date instead of two.
    // It is painted with an offset of -height/2, which is taken into account here.
    if ( typ == TypeEvent )
        itemRect = QRectF( itemRect.topLeft() - QPointF( itemRect.height() / 2.0, 0 ), QSizeF( itemRect.height(),
                                                                                               itemRect.height() ) );

    if ( typ == TypeNone || typ == TypeSummary ) return State_None;
    if ( !itemRect.contains(pos) ) return State_None;
    if ( typ == TypeEvent )
        return State_Move;

    qreal delta = 5.;
    if ( itemRect.width() < 15 ) delta = 1.;
    if ( pos.x() >= itemRect.left() && pos.x() < itemRect.left()+delta ) {
        return State_ExtendLeft;
    } else   if ( pos.x() <= itemRect.right() && pos.x() > itemRect.right()-delta ) {
        return State_ExtendRight;
    } else {
        return State_Move;
    }
}

/*! Paints the gantt item \a idx using \a painter and \a opt
 */
void ItemDelegate::paintGanttItem( QPainter* painter,
                                   const StyleOptionGanttItem& opt,
                                   const QModelIndex& idx )
{
    if ( !idx.isValid() ) return;
    const ItemType typ = static_cast<ItemType>( idx.model()->data( idx, ItemTypeRole ).toInt() );
    const QString& txt = opt.text;
    QRectF itemRect = opt.itemRect;
    QRectF boundingRect = opt.boundingRect;
    boundingRect.setY( itemRect.y() );
    boundingRect.setHeight( itemRect.height() );

    //qDebug() << "itemRect="<<itemRect<<", boundingRect="<<boundingRect;
    //qDebug() << painter->font() << opt.fontMetrics.height() << painter->device()->width() << painter->device()->height();

    painter->save();

    QPen pen = defaultPen( typ );
    if ( opt.state & QStyle::State_Selected ) pen.setWidth( 2*pen.width() );
    painter->setPen( pen );
    painter->setBrush( defaultBrush( typ ) );

    bool drawText = true;
    qreal pw = painter->pen().width()/2.;
    switch ( typ ) {
    case TypeTask:
        if ( itemRect.isValid() ) {
            // TODO
            qreal pw = painter->pen().width()/2.;
            pw-=1;
            QRectF r = itemRect;
            r.translate( 0., r.height()/6. );
            r.setHeight( 2.*r.height()/3. );
            painter->setBrushOrigin( itemRect.topLeft() );
            painter->save();
            painter->translate( 0.5, 0.5 );
            painter->drawRect( r );
            bool ok;
            qreal completion = idx.model()->data( idx, KDGantt::TaskCompletionRole ).toReal( &ok );
            if ( ok ) {
                qreal h = r.height();
                QRectF cr( r.x(), r.y()+h/4.,
                           r.width()*completion/100., h/2.+1 /*??*/ );
                QColor compcolor( painter->pen().color() );
                compcolor.setAlpha( 150 );
                painter->fillRect( cr, compcolor );
            }
            painter->restore();
        }
        break;
    case TypeSummary:
        if ( opt.itemRect.isValid() ) {
            // TODO
            pw-=1;
            const QRectF r = QRectF( opt.itemRect ).adjusted( -pw, -pw, pw, pw );
            QPainterPath path;
            const qreal deltaY = r.height()/2.;
            const qreal deltaXBezierControl = .25*qMin( r.width(), r.height() );
            const qreal deltaX = qMin( r.width()/2., r.height() );
            path.moveTo( r.topLeft() );
            path.lineTo( r.topRight() );
            path.lineTo( QPointF( r.right(), r.top() + 2.*deltaY ) );
            //path.lineTo( QPointF( r.right()-3./2.*delta, r.top() + delta ) );
            path.quadTo( QPointF( r.right()-deltaXBezierControl, r.top() + deltaY ), QPointF( r.right()-deltaX, r.top() + deltaY ) );
            //path.lineTo( QPointF( r.left()+3./2.*delta, r.top() + delta ) );
            path.lineTo( QPointF( r.left() + deltaX, r.top() + deltaY ) );
            path.quadTo( QPointF( r.left()+deltaXBezierControl, r.top() + deltaY ), QPointF( r.left(), r.top() + 2.*deltaY ) );
            path.closeSubpath();
            painter->setBrushOrigin( itemRect.topLeft() );
            painter->save();
            painter->translate( 0.5, 0.5 );
            painter->drawPath( path );
            painter->restore();
        }
        break;
    case TypeEvent: /* TODO */
        //qDebug() << opt.boundingRect << opt.itemRect;
        if ( opt.boundingRect.isValid() ) {
            const qreal pw = painter->pen().width() / 2. - 1;
            const QRectF r = QRectF( opt.itemRect ).adjusted( -pw, -pw, pw, pw ).translated( -opt.itemRect.height()/2, 0 );
            QPainterPath path;
            const qreal delta = static_cast< int >( r.height() / 2 );
            path.moveTo( delta, 0. );
            path.lineTo( 2.*delta, delta );
            path.lineTo( delta, 2.*delta );
            path.lineTo( 0., delta );
            path.closeSubpath();
            painter->save();
            painter->translate( r.topLeft() );
            painter->translate( 0, 0.5 );
            painter->drawPath( path );
            painter->restore();
#if 0
            painter->setBrush( Qt::NoBrush );
            painter->setPen( Qt::black );
            painter->drawRect( opt.boundingRect );
            painter->setPen( Qt::red );
            painter->drawRect( r );
#endif
        }
        break;
    default:
        drawText = false;
        break;
    }

    Qt::Alignment ta;
    switch ( opt.displayPosition ) {
        case StyleOptionGanttItem::Left: ta = Qt::AlignLeft; break;
        case StyleOptionGanttItem::Right: ta = Qt::AlignRight; break;
        case StyleOptionGanttItem::Center: ta = Qt::AlignCenter; break;
        case StyleOptionGanttItem::Hidden: drawText = false; break;
    }
    if ( drawText ) {
        painter->drawText( boundingRect, ta | Qt::AlignVCenter, txt );
    }

    painter->restore();
}

static const qreal TURN = 10.;
static const qreal PW = 1.5;

/*! \return The bounding rectangle for the graphics used to represent
 * a constraint between points \a start and \a end (typically an
 * arrow)
 */
QRectF ItemDelegate::constraintBoundingRect( const QPointF& start, const QPointF& end, const Constraint &constraint ) const
{
    QPolygonF poly;
    switch ( constraint.relationType() ) {
        case Constraint::FinishStart:
            poly = finishStartLine( start, end ) + finishStartArrow( start, end );
            break;
        case Constraint::FinishFinish:
            poly = finishFinishLine( start, end ) + finishFinishArrow( start, end );
            break;
        case Constraint::StartStart:
            poly = startStartLine( start, end ) + startStartArrow( start, end );
            break;
        case Constraint::StartFinish:
            poly = startFinishLine( start, end ) + startFinishArrow( start, end );
            break;
    }
    return poly.boundingRect().adjusted( -PW, -PW, PW, PW );
}


/*! Paints the \a constraint between points \a start and \a end
 * using \a painter and \a opt.
 *
 * \todo Review \a opt's type
 */
void ItemDelegate::paintConstraintItem( QPainter* painter, const QStyleOptionGraphicsItem& opt,
                                        const QPointF& start, const QPointF& end, const Constraint &constraint )
{
    //qDebug()<<"ItemDelegate::paintConstraintItem"<<start<<end<<constraint;
    switch ( constraint.relationType() ) {
        case Constraint::FinishStart:
            paintFinishStartConstraint( painter, opt, start, end, constraint );
            break;
        case Constraint::FinishFinish:
            paintFinishFinishConstraint( painter, opt, start, end, constraint );
            break;
        case Constraint::StartStart:
            paintStartStartConstraint( painter, opt, start, end, constraint );
            break;
        case Constraint::StartFinish:
            paintStartFinishConstraint( painter, opt, start, end, constraint );
            break;
    }
}

void ItemDelegate::paintFinishStartConstraint( QPainter* painter, const QStyleOptionGraphicsItem& opt, const QPointF& start, const QPointF& end, const Constraint &constraint )
{
    Q_UNUSED( opt );

    const QPen pen = d->constraintPen( start, end, constraint );

    painter->setPen( pen );
    painter->setBrush( pen.color() );

    painter->drawPolyline( finishStartLine( start, end ) );
    painter->drawPolygon( finishStartArrow( start, end ) );
}

QPolygonF ItemDelegate::finishStartLine( const QPointF& start, const QPointF& end ) const
{
    QPolygonF poly;
    qreal midx = end.x() - TURN;
    qreal midy = ( end.y()-start.y() )/2. + start.y();

    if ( start.x() > end.x()-TURN ) {
        poly << start
                << QPointF( start.x()+TURN, start.y() )
                << QPointF( start.x()+TURN, midy )
                << QPointF( end.x()-TURN, midy )
                << QPointF( end.x()-TURN, end.y() )
                << end;
    } else {
        poly << start
                << QPointF( midx, start.y() )
                << QPointF( midx, end.y() )
                << end;
    }
    return poly;
}

QPolygonF ItemDelegate::finishStartArrow( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;
    poly << end
            << QPointF( end.x()-TURN/2., end.y()-TURN/2. )
            << QPointF( end.x()-TURN/2., end.y()+TURN/2. );
    return poly;
}

void ItemDelegate::paintFinishFinishConstraint( QPainter* painter, const QStyleOptionGraphicsItem& opt, const QPointF& start, const QPointF& end, const Constraint &constraint )
{
    Q_UNUSED( opt );

    const QPen pen = d->constraintPen( start, end, constraint );

    painter->setPen( pen );
    painter->setBrush( pen.color() );

    painter->drawPolyline( finishFinishLine( start, end ) );
    painter->drawPolygon( finishFinishArrow( start, end ) );
}

QPolygonF ItemDelegate::finishFinishLine( const QPointF& start, const QPointF& end ) const
{
    QPolygonF poly;
    qreal midx = end.x() + TURN;
    qreal midy = ( end.y()-start.y() )/2. + start.y();

    if ( start.x() > end.x()+TURN ) {
        poly << start
                << QPointF( start.x()+TURN, start.y() )
                << QPointF( start.x()+TURN, end.y() )
                << end;
    } else {
        poly << start
                << QPointF( midx, start.y() )
                << QPointF( midx, midy )
                << QPointF( end.x()+TURN, midy )
                << QPointF( end.x()+TURN, end.y() )
                << end;
    }
    return poly;
}

QPolygonF ItemDelegate::finishFinishArrow( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;
    poly << end
            << QPointF( end.x()+TURN/2., end.y()-TURN/2. )
            << QPointF( end.x()+TURN/2., end.y()+TURN/2. );
    return poly;
}

void ItemDelegate::paintStartStartConstraint( QPainter* painter, const QStyleOptionGraphicsItem& opt, const QPointF& start, const QPointF& end, const Constraint &constraint )
{
    Q_UNUSED( opt );

    const QPen pen = d->constraintPen( start, end, constraint );

    painter->setPen( pen );
    painter->setBrush( pen.color() );

    painter->drawPolyline( startStartLine( start, end ) );
    painter->drawPolygon( startStartArrow( start, end ) );

}

QPolygonF ItemDelegate::startStartLine( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;

    if ( start.x() > end.x() ) {
        poly << start
                << QPointF( end.x()-TURN, start.y() )
                << QPointF( end.x()-TURN, end.y() )
                << end;
    } else {
        poly << start
                << QPointF( start.x()-TURN, start.y() )
                << QPointF( start.x()-TURN, end.y() )
                << QPointF( end.x()-TURN, end.y() )
                << end;
    }
    return poly;
}

QPolygonF ItemDelegate::startStartArrow( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;
    poly << end
            << QPointF( end.x()-TURN/2., end.y()-TURN/2. )
            << QPointF( end.x()-TURN/2., end.y()+TURN/2. );
    return poly;
}

void ItemDelegate::paintStartFinishConstraint( QPainter* painter, const QStyleOptionGraphicsItem& opt, const QPointF& start, const QPointF& end, const Constraint &constraint )
{
    Q_UNUSED( opt );

    const QPen pen = d->constraintPen( start, end, constraint );

    painter->setPen( pen );
    painter->setBrush( pen.color() );

    painter->drawPolyline( startFinishLine( start, end ) );
    painter->drawPolygon( startFinishArrow( start, end ) );
}

QPolygonF ItemDelegate::startFinishLine( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;
    qreal midx = end.x() + TURN;
    qreal midy = ( end.y()-start.y() )/2. + start.y();

    if ( start.x()-TURN > end.x()+TURN ) {
        poly << start
                << QPointF( midx, start.y() )
                << QPointF( midx, end.y() )
                << end;
    } else {
        poly << start
                << QPointF( start.x()-TURN, start.y() )
                << QPointF( start.x()-TURN, midy )
                << QPointF( midx, midy )
                << QPointF( end.x()+TURN, end.y() )
                << end;
    }
    return poly;
}

QPolygonF ItemDelegate::startFinishArrow( const QPointF& start, const QPointF& end ) const
{
    Q_UNUSED(start);

    QPolygonF poly;
    poly << end
            << QPointF( end.x()+TURN/2., end.y()-TURN/2. )
            << QPointF( end.x()+TURN/2., end.y()+TURN/2. );
    return poly;
}


#include "moc_kdganttitemdelegate.cpp"
