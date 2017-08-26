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

#ifndef KDGANTTCONSTRAINTMODEL_P_H
#define KDGANTTCONSTRAINTMODEL_P_H

#include "kdganttconstraintmodel.h"

#include <QList>
#include <QMultiHash>
#include <QPersistentModelIndex>

namespace KDGantt {
    class ConstraintModel::Private {
    public:
        Private();

        void addConstraintToIndex( const QModelIndex& idx, const Constraint& c );
        void removeConstraintFromIndex( const QModelIndex& idx,  const Constraint& c );

        typedef QMultiHash<QPersistentModelIndex,Constraint> IndexType;

        QList<Constraint> constraints;
        IndexType indexMap;
    };
}

#endif /* KDGANTTCONSTRAINTMODEL_P_H */

