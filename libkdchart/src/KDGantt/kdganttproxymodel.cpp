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

#include "kdganttproxymodel.h"
#include "kdganttproxymodel_p.h"


using namespace KDGantt;

typedef ForwardingProxyModel BASE;

ProxyModel::Private::Private( ProxyModel* _q )
#if 0
    : calendarMode( false )
#endif
{
    Q_UNUSED( _q ); // for now

    columnMap[Qt::DisplayRole]    = 0;
    columnMap[ItemTypeRole]       = 1;
    columnMap[StartTimeRole]      = 2;
    columnMap[EndTimeRole]        = 3;
    columnMap[TaskCompletionRole] = 4;
    columnMap[LegendRole]         = 5;

    roleMap[Qt::DisplayRole]    = Qt::DisplayRole;
    roleMap[ItemTypeRole]       = Qt::DisplayRole;
    roleMap[StartTimeRole]      = StartTimeRole;
    roleMap[EndTimeRole]        = EndTimeRole;
    roleMap[TaskCompletionRole] = Qt::DisplayRole;
    roleMap[LegendRole]         = Qt::DisplayRole;
}

ProxyModel::ProxyModel( QObject* parent )
    : BASE( parent ), _d( new Private( this ) )
{
    init();
}

ProxyModel::~ProxyModel()
{
    delete _d; _d = 0;
}

#define d d_func()

void ProxyModel::init()
{
}

QModelIndex ProxyModel::mapFromSource( const QModelIndex& sourceIdx ) const
{
#if 0
    if ( sourceIdx.isValid() ) {
        if ( calendarMode() ) {
            const QAbstractItemModel* model = sourceIdx.model();
            if ( model->hasChildren( sourceIdx ) ) {
                return BASE::mapFromSource( model->index( sourceIdx.row(),0,sourceIdx.parent()));
            } else {
                // Map children to columns
                return BASE::mapFromSource( model->index( sourceIdx.row(),0,sourceIdx.parent()))
                    .child( 0, sourceIdx.column() );
            }
        }
        return BASE::mapFromSource( sourceIdx.model()->index( sourceIdx.row(),0,sourceIdx.parent()));
    }
    else return QModelIndex();
#else
    return BASE::mapFromSource( sourceIdx.model()?sourceIdx.model()->index( sourceIdx.row(),0,sourceIdx.parent()):QModelIndex());
#endif
}

QModelIndex ProxyModel::mapToSource( const QModelIndex& proxyIdx ) const
{
#if 0
    if ( proxyIdx.isValid() ) {
        if ( calendarMode() && proxyIdx.column() > 0 ) {
            return BASE::mapToSource( proxyIdx.model()->index( proxyIdx.column(), 0, proxyIdx ) );
        }
        return BASE::mapToSource( proxyIdx );
    }
    else return QModelIndex();
#else
    return BASE::mapToSource( proxyIdx );
#endif
}

void ProxyModel::setColumn( int ganttrole, int col )
{
    d->columnMap[ganttrole] = col;
}

int ProxyModel::column( int ganttrole ) const
{
    return d->columnMap[ganttrole];
}

void ProxyModel::setRole( int ganttrole, int role )
{
    d->roleMap[ganttrole] = role;
}

int ProxyModel::role( int ganttrole ) const
{
    return d->roleMap[ganttrole];
}

#if 0
void ProxyModel::setCalendarMode( bool enable )
{
    if ( d->calendarMode != enable ) {
        d->calendarMode = enable;
        reset();
    }
}

bool ProxyModel::calendarMode() const
{
    return d->calendarMode;
}
#endif

int ProxyModel::rowCount( const QModelIndex& proxyIndex ) const
{
    // TODO
    return BASE::rowCount( proxyIndex );
}

int ProxyModel::columnCount( const QModelIndex& proxyIndex ) const
{
    return qMin( sourceModel()->columnCount( mapToSource( proxyIndex ) ), 1 );
}

QVariant ProxyModel::data( const QModelIndex& proxyIdx, int role ) const
{
    int srole = role;
    int scol  = proxyIdx.column();
    QHash<int, int>::const_iterator it = d->roleMap.find( role );
    if ( it != d->roleMap.end() ) srole = *it;
    it = d->columnMap.find( role );
    if ( it != d->columnMap.end() ) scol = *it;

#if 0
    qDebug() << "mapping "<<static_cast<ItemDataRole>(role)<<", "<<proxyIdx.column()
             << " => " << static_cast<ItemDataRole>(srole)<<", " << scol
             << "value="
             << sourceModel()->data( sourceModel()->index( proxyIdx.row(), scol,
                                                           mapToSource( proxyIdx.parent() ) ), srole );
#endif

    const QAbstractItemModel* model = sourceModel();
    return model->data( model->index( proxyIdx.row(), scol, mapToSource( proxyIdx.parent() ) ), srole );
}

bool ProxyModel::setData( const QModelIndex& proxyIdx, const QVariant& value, int role )
{
    int srole = role;
    int scol  = proxyIdx.column();
    QHash<int, int>::const_iterator it = d->roleMap.constFind( role );
    if ( it != d->roleMap.constEnd() ) srole = *it;
    it = d->columnMap.constFind( role );
    if ( it != d->columnMap.constEnd() ) scol = *it;

    QAbstractItemModel* model = sourceModel();
    return model->setData( model->index( proxyIdx.row(), scol, mapToSource( proxyIdx.parent() ) ), value, srole );
}

#include "moc_kdganttproxymodel.cpp"
