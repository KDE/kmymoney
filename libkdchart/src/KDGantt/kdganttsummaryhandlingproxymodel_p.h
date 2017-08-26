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

#ifndef KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H
#define KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H

#include "kdganttsummaryhandlingproxymodel.h"

#include <QDateTime>
#include <QHash>
#include <QPair>
#include <QPersistentModelIndex>

namespace KDGantt {
    class SummaryHandlingProxyModel::Private {
    public:
        bool cacheLookup( const QModelIndex& idx,
                          QPair<QDateTime,QDateTime>* result ) const;
        void insertInCache( const SummaryHandlingProxyModel* model, const QModelIndex& idx ) const;
        void removeFromCache( const QModelIndex& idx ) const;
        void clearCache() const;
		
		inline bool isSummary( const QModelIndex& idx ) const {
			int typ = idx.data( ItemTypeRole ).toInt();
			return (typ==TypeSummary) || (typ==TypeMulti);
		}

        mutable QHash<QModelIndex, QPair<QDateTime, QDateTime> > cached_summary_items;
    };
}

#endif /* KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H */

