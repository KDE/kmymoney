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

#include "kdganttview.h"
#include "kdganttview_p.h"

#include "kdganttitemdelegate.h"
#include "kdganttgraphicsitem.h"
#include "kdganttsummaryhandlingproxymodel.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QPaintEvent>

#include <QDebug>

#include <cassert>

#if defined KDAB_EVAL
#include "../evaldialog/evaldialog.h"
#endif

using namespace KDGantt;

namespace {
    class HeaderView : public QHeaderView {
    public:
        explicit HeaderView( QWidget* parent=0 ) : QHeaderView( Qt::Horizontal, parent ) {
        }

        QSize sizeHint() const { QSize s = QHeaderView::sizeHint(); s.rheight() *= 2; return s; }
    };
}

KDGanttTreeView::KDGanttTreeView( QAbstractProxyModel* proxy, QWidget* parent )
    : QTreeView( parent ),
      m_controller( this, proxy )
{
    setHeader( new HeaderView );
}

KDGanttTreeView::~KDGanttTreeView()
{
}

void KDGanttTreeView::expandAll(QModelIndex index)
{
    for (int i = 0; i < model()->rowCount(index); i++) {
        QModelIndex indexAt = model()->index(i, 0, index);
        if (model()->hasChildren(indexAt))
            expandAll(indexAt);
        if (isExpanded(indexAt))
            continue;
        expand(indexAt);
    }
}

void KDGanttTreeView::collapseAll(QModelIndex index)
{
    for (int i = 0; i < model()->rowCount(index); i++) {
        QModelIndex indexAt = model()->index(i, 0, index);
        if (model()->hasChildren(indexAt))
            collapseAll(indexAt);
        if (!isExpanded(indexAt))
            continue;
        collapse(indexAt);
    }
}

View::Private::Private(View* v)
    : q(v),
      splitter(v),
      rowController(0),
      gfxview( new GraphicsView( &splitter ) ),
      model(0)
{
    //init();
}

View::Private::~Private()
{
    delete gfxview;
}

void View::Private::init()
{
    KDGanttTreeView* tw = new KDGanttTreeView( &ganttProxyModel, &splitter );
    tw->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tw->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

    q->setLeftView( tw );
    q->setRowController( tw->rowController() );

    //gfxview.setRenderHints( QPainter::Antialiasing );

    tw->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QVBoxLayout* layout = new QVBoxLayout(q);
    layout->setMargin(0);
    layout->addWidget(&splitter);
    q->setLayout(layout);

    constraintProxy.setProxyModel( &ganttProxyModel );
    constraintProxy.setDestinationModel( &mappedConstraintModel );
    setupGraphicsView();
}

void View::Private::setupGraphicsView()
{
    gfxview->setParent( &splitter );
    gfxview->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    gfxview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    gfxview->setSelectionModel( leftWidget->selectionModel() );
    gfxview->setConstraintModel( &mappedConstraintModel );
    q->setLeftView( leftWidget );
    q->setRowController( rowController );
    updateScene();
}

void View::Private::updateScene()
{
    gfxview->clearItems();
    if ( !model) return;

    if ( QTreeView* tw = qobject_cast<QTreeView*>(leftWidget)) {
      QModelIndex idx = ganttProxyModel.mapFromSource( model->index( 0, 0, leftWidget->rootIndex() ) );
      do {
        gfxview->updateRow( idx );
      } while ( ( idx = tw->indexBelow( idx ) ) != QModelIndex() &&
		gfxview->rowController()->isRowVisible(idx) );
      gfxview->updateSceneRect();
    } else {
      const QModelIndex rootidx = ganttProxyModel.mapFromSource( leftWidget->rootIndex() );
      for ( int r = 0; r < ganttProxyModel.rowCount(rootidx); ++r ) {
	gfxview->updateRow( ganttProxyModel.index( r, 0, rootidx ) );
      }
    }
}

void View::Private::slotCollapsed(const QModelIndex& _idx)
{
    QTreeView* tw = qobject_cast<QTreeView*>(leftWidget);
    if (!tw) return;

    bool blocked = gfxview->blockSignals( true );

    QModelIndex idx( _idx );
    const QAbstractItemModel* model = leftWidget->model();
    const QModelIndex pidx = ganttProxyModel.mapFromSource(idx);
    bool isMulti = false;
    for ( QModelIndex treewalkidx = pidx; treewalkidx.isValid(); treewalkidx = treewalkidx.parent() ) {
        if ( treewalkidx.data( ItemTypeRole ).toInt() == TypeMulti
             && !gfxview->rowController()->isRowExpanded( treewalkidx ) ) {
            isMulti = true;
            break;
        }
    }

    if ( !isMulti ) {
        for ( int i = 0; i < model->rowCount( idx ); ++i ) {
            gfxview->deleteSubtree( ganttProxyModel.index( i, 0, pidx ) );
        }
    } else {
        gfxview->updateRow(pidx);
    }
    //qDebug() << "Looking to update from " << idx;
    while ( ( idx=tw->indexBelow( idx ) ) != QModelIndex() &&
            gfxview->rowController()->isRowVisible( ganttProxyModel.mapFromSource(idx) ) ) {
        const QModelIndex proxyidx( ganttProxyModel.mapFromSource( idx ) );
        gfxview->updateRow(proxyidx);
    }
    gfxview->blockSignals( blocked );
    gfxview->updateSceneRect();
}

void View::Private::slotExpanded(const QModelIndex& _idx)
{
    QModelIndex idx( ganttProxyModel.mapFromSource( _idx ) );
    do {
        //qDebug() << "Updating row" << idx << idx.data( Qt::DisplayRole ).toString();
        gfxview->updateRow(idx);
    } while ( ( idx=gfxview->rowController()->indexBelow( idx ) ) != QModelIndex()
             && gfxview->rowController()->isRowVisible( idx ) );
    gfxview->updateSceneRect();
}

void View::Private::slotVerticalScrollValueChanged( int val )
{
#if 0
    qDebug() << "View::Private::slotVerticalScrollValueChanged("<<val<<")="
             << val/gfxview->verticalScrollBar()->singleStep();
#endif
    leftWidget->verticalScrollBar()->setValue( val/gfxview->verticalScrollBar()->singleStep() );
}

void View::Private::slotLeftWidgetVerticalRangeChanged(int min, int max )
{
    //qDebug() << "View::Private::slotLeftWidgetVerticalRangeChanged("<<min<<max<<")";
    gfxview->verticalScrollBar()->setRange( min, max );
    gfxview->updateSceneRect();
}

void View::Private::slotGfxViewVerticalRangeChanged( int min, int max )
{
    //qDebug() << "View::Private::slotGfxViewVerticalRangeChanged("<<min<<max<<")";
    if ( !leftWidget.isNull() && !gfxview.isNull() ) {
        int leftMin = leftWidget->verticalScrollBar()->minimum();
        int leftMax = leftWidget->verticalScrollBar()->maximum();
        bool blocked = gfxview->verticalScrollBar()->blockSignals( true );
        gfxview->verticalScrollBar()->setRange( qMax( min, leftMin ), qMax( max, leftMax ) );
        gfxview->verticalScrollBar()->blockSignals( blocked );
    }
}

/*!\class KDGantt::View kdganttview.h KDGanttView
 * \ingroup KDGantt
 * \brief This widget that consists of a QTreeView and a GraphicsView
 *
 * This is the easy to use, complete gantt chart widget. It
 * consists of a QTreeView on the left and a KDGantt::GraphicsView
 * on the right separated by a QSplitter. The two views share the same
 * model.
 */

/*! Constructor. Creates a View with parent \a parent,
 * a DateTimeGrid as default grid implementaion and no model etc.
 */
View::View(QWidget* parent)
    : QWidget(parent),
      _d(new Private(this))
{
#if defined KDAB_EVAL
   EvalDialog::checkEvalLicense( "KD Gantt" );
#endif
   _d->init();
}

View::~View()
{
    delete _d;
}

#define d d_func()

/*! Replaces the left widget with a custom QAbstractItemView.
 *
 * \param aiv The view to be used to the left, instead of the default tree view
 * \sa setRowController()
 */
void View::setLeftView( QAbstractItemView* aiv )
{
    assert( aiv );
    if ( aiv==d->leftWidget ) return;
    if ( !d->leftWidget.isNull() ) {
        d->leftWidget->disconnect( this );
        d->leftWidget->hide();
        d->leftWidget->verticalScrollBar()->disconnect( d->gfxview->verticalScrollBar() );
        d->gfxview->verticalScrollBar()->disconnect( d->leftWidget->verticalScrollBar() );
    }

    d->leftWidget = aiv;
    d->splitter.insertWidget( 0, d->leftWidget );

    if ( qobject_cast<QTreeView*>(d->leftWidget) ) {
      connect( d->leftWidget,  SIGNAL( collapsed( const QModelIndex& ) ),
	       this, SLOT( slotCollapsed( const QModelIndex& ) ) );
      connect( d->leftWidget,  SIGNAL( expanded( const QModelIndex& ) ),
	       this, SLOT( slotExpanded( const QModelIndex& ) ) );
    }

    connect( d->gfxview->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             d->leftWidget->verticalScrollBar(), SLOT( setValue( int ) ) );
    connect( d->leftWidget->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             d->gfxview->verticalScrollBar(), SLOT( setValue( int ) ) );
    connect( d->leftWidget->verticalScrollBar(), SIGNAL( rangeChanged( int, int ) ),
             this, SLOT( slotLeftWidgetVerticalRangeChanged( int, int ) ) );
    connect( d->gfxview->verticalScrollBar(), SIGNAL( rangeChanged( int, int ) ),
             this, SLOT( slotGfxViewVerticalRangeChanged( int, int ) ) );
}

/*! Sets \a ctrl to be the rowcontroller used by this View.
 * The default rowcontroller is owned by KDGantt::View and is
 * suitable for the default treeview in the left part of the view.
 * You probably only want to change this if you replace the treeview.
 */
void View::setRowController( AbstractRowController* ctrl )
{
    if ( ctrl == d->rowController && d->gfxview->rowController() == ctrl ) return;
    d->rowController = ctrl;
    d->gfxview->setRowController( d->rowController );
}

/*! \returns a pointer to the current rowcontroller.
 * \see AbstractRowController
 */
AbstractRowController* View::rowController()
{
    return d->rowController;
}

/*! \overload AbstractRowController* KDGantt::View::rowController()
 */
const AbstractRowController* View::rowController() const
{
    return d->rowController;
}

/*!
 * \returns a pointer to the QAbstractItemView in the left
 * part of the widget.
 * */
const QAbstractItemView* View::leftView() const
{
    return d->leftWidget;
}

/*!
 * \overload const QAbstractItemView* KDGantt::View::leftView() const
 */
QAbstractItemView* View::leftView()
{
    return d->leftWidget;
}

/*! Set the GraphicsView to be used for this View. It only makes sense to call this
 * if you need to subclass GraphicsView.
 *
 * NOTE: _Only_ call this right after creating the View, before setting a model or any other
 * attributes.
 */
void View::setGraphicsView( GraphicsView* gv )
{
    if ( gv != d->gfxview ) {
        GraphicsView* old = d->gfxview;
        d->gfxview = gv;
        d->setupGraphicsView();
        d->gfxview->setGrid( old->grid() );
        delete old;
    }
}

/*!
 * \returns a pointer to the GraphicsView
 */
const GraphicsView* View::graphicsView() const
{
    return d->gfxview;
}

/*!
 * \overload const GraphicsView* KDGantt::View::graphicsView() const
 */
GraphicsView* View::graphicsView()
{
    return d->gfxview;
}

/*!
 * \returns a pointer to the QSplitter that manages the left view and graphicsView
 */
const QSplitter* View::splitter() const
{
    return &d->splitter;
}

/*!
 * \overload const QSplitter* KDGantt::View::splitter() const
 */
QSplitter* View::splitter()
{
    return &d->splitter;
}


/*! \returns the current model displayed by this view
 */
QAbstractItemModel* View::model() const
{
    return leftView()->model();
}

/*! Sets the QAbstractItemModel to be displayed in this view
 * to \a model.
 *
 * \see GraphicsView::setModel
 */
void View::setModel( QAbstractItemModel* model )
{
    leftView()->setModel( model );
    d->ganttProxyModel.setSourceModel( model );
    d->gfxview->setModel( &d->ganttProxyModel );
}

/*! \returns the QItemSelectionModel used by this view
 */
QItemSelectionModel* View::selectionModel() const
{
    return leftView()->selectionModel();
}

/*! Sets the QItemSelectionModel used by this view to manage
 * selections. Similar to QAbstractItemView::setSelectionModel
 */
void View::setSelectionModel( QItemSelectionModel* smodel )
{
    leftView()->setSelectionModel( smodel );
    d->gfxview->setSelectionModel( new QItemSelectionModel( &( d->ganttProxyModel ),this ) );
}

/*! Sets the AbstractGrid for this view. The grid is an
 * object that controls how QModelIndexes are mapped
 * to and from the view and how the background and header
 * is rendered. \see AbstractGrid and DateTimeGrid.
 */
void View::setGrid( AbstractGrid* grid )
{
    d->gfxview->setGrid( grid );
}

void View::expandAll( QModelIndex index )
{
    KDGanttTreeView* tw = qobject_cast<KDGanttTreeView*>(leftView());
    tw->expandAll(index);
}

void View::collapseAll( QModelIndex index )
{
    KDGanttTreeView* tw = qobject_cast<KDGanttTreeView*>(leftView());
    tw->collapseAll(index);
}

/*! \returns the AbstractGrid used by this view.
 */
AbstractGrid* View::grid() const
{
    return d->gfxview->grid();
}

/*! \returns the rootindex for this view.
 */
QModelIndex View::rootIndex() const
{
    return leftView()->rootIndex();
}

/*! Sets the root index of the model displayed by this view.
 * Similar to QAbstractItemView::setRootIndex, default is QModelIndex().
 */
void View::setRootIndex( const QModelIndex& idx )
{
    leftView()->setRootIndex( idx );
    d->gfxview->setRootIndex( idx );
}

/*! \returns the ItemDelegate used by this view to render items
*/
ItemDelegate* View::itemDelegate() const
{
    return d->gfxview->itemDelegate();
}

/*! Sets the KDGantt::ItemDelegate used for rendering items on this
 * view. \see ItemDelegate and QAbstractItemDelegate.
 */
void View::setItemDelegate( ItemDelegate* delegate )
{
    leftView()->setItemDelegate( delegate );
    d->gfxview->setItemDelegate( delegate );
}

/*! Sets the constraintmodel displayed by this view.
 * \see KDGantt::ConstraintModel.
 */
void View::setConstraintModel( ConstraintModel* cm )
{
    d->constraintProxy.setSourceModel( cm );
    d->gfxview->setConstraintModel( &d->mappedConstraintModel );
}

/*! \returns the KDGantt::ConstraintModel displayed by this view.
 */
ConstraintModel* View::constraintModel() const
{
    return d->constraintProxy.sourceModel();
}

const QAbstractProxyModel* View::ganttProxyModel() const
{
    return &( d->ganttProxyModel );
}

QAbstractProxyModel* View::ganttProxyModel()
{
    return &( d->ganttProxyModel );
}

void View::ensureVisible(const QModelIndex& index)
{
    QGraphicsView* view = graphicsView();
    KDGantt::GraphicsScene* scene = static_cast<KDGantt::GraphicsScene*>(view->scene());
    if (!scene)
        return;

    KDGantt::SummaryHandlingProxyModel* model = static_cast<KDGantt::SummaryHandlingProxyModel*>(scene->summaryHandlingModel());

    const QModelIndex pidx = d->ganttProxyModel.mapFromSource(index);
    const QModelIndex idx = model->mapFromSource( pidx );
    QGraphicsItem* item = scene->findItem(idx);
    view->ensureVisible(item);
}

void View::resizeEvent(QResizeEvent*ev)
{
    QWidget::resizeEvent(ev);
}

/*!\returns The QModelIndex for the item located at
 * position \a pos in the view or an invalid index
 * if no item was present at that position.
 *
 * \see GraphicsView::indexAt
 */
QModelIndex View::indexAt( const QPoint& pos ) const
{
    return d->gfxview->indexAt( pos );
}

/*! Print the Gantt chart using \a printer. If \a drawRowLabels
 * is true (the default), each row will have it's label printed
 * on the left side. If \a drawColumnLabels is true (the
 * default), each column will have it's label printed at the
 * top side.
 *
 * This version of print() will print multiple pages.
 */
void View::print( QPrinter* printer, bool drawRowLabels, bool drawColumnLabels )
{
    graphicsView()->print( printer, drawRowLabels, drawColumnLabels );
}

/*! Print part of the Gantt chart from \a start to \a end using \a printer.
 * If \a drawRowLabels is true (the default), each row will have it's
 * label printed on the left side. If \a drawColumnLabels is true (the
 * default), each column will have it's label printed at the
 * top side.
 *
 * This version of print() will print multiple pages.
 *
 * To print a certain range of a chart with a DateTimeGrid, use
 * qreal DateTimeGrid::mapFromDateTime( const QDateTime& dt) const
 * to figure out the values for \a start and \a end.
 */
void View::print( QPrinter* printer, qreal start, qreal end, bool drawRowLabels, bool drawColumnLabels )
{
    graphicsView()->print( printer, start, end, drawRowLabels, drawColumnLabels );
}

/*! Render the GanttView inside the rectangle \a target using the painter \a painter.
 * If \a drawRowLabels is true (the default), each row will have it's
 * label printed on the left side. If \a drawColumnLabels is true (the
 * default), each column will have it's label printed at the
 * top side.
 */
void View::print( QPainter* painter, const QRectF& target, bool drawRowLabels, bool drawColumnLabels)
{
    d->gfxview->print( painter,
		      target,
		      drawRowLabels,
              drawColumnLabels);
}

/*! Render the GanttView inside the rectangle \a target using the painter \a painter.
 * If \a drawRowLabels is true (the default), each row will have it's
 * label printed on the left side. If \a drawColumnLabels is true (the
 * default), each column will have it's label printed at the
 * top side.
 *
 * To print a certain range of a chart with a DateTimeGrid, use
 * qreal DateTimeGrid::mapFromDateTime( const QDateTime& dt) const
 * to figure out the values for \a start and \a end.
 */
void View::print( QPainter* painter, qreal start, qreal end, const QRectF& target, bool drawRowLabels, bool drawColumnLabels)
{
    d->gfxview->print( painter,
                      start, end,
		      target,
		      drawRowLabels,
              drawColumnLabels);
}


#include "moc_kdganttview.cpp"

#ifndef KDAB_NO_UNIT_TESTS
#include "unittest/test.h"

#include "kdganttlistviewrowcontroller.h"
#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QListView>

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, View, "test" ) {
    View view( 0 );
#if 0 // GUI tests do not work well on the server
    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );
    view.show();

    qApp->exec();
    QPixmap screenshot1 = QPixmap::grabWidget( &view );

    QTreeView* tv = new QTreeView;
    view.setLeftView( tv );
    view.setRowController( new TreeViewRowController(tv,view.ganttProxyModel()) );

    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );

    qApp->exec();
    QPixmap screenshot2 = QPixmap::grabWidget( &view );

    assertEqual( screenshot1.toImage(),  screenshot2.toImage() );

    QListView* lv = new QListView;
    view.setLeftView(lv);
    view.setRowController( new ListViewRowController(lv,view.ganttProxyModel()));
    view.show();
    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );
    qApp->exec();
#endif
}
#endif /* KDAB_NO_UNIT_TESTS */
