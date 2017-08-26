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

#ifndef KDGANTTCONSTRAINT_P_H
#define KDGANTTCONSTRAINT_P_H

#include <QSharedData>
#include <QPersistentModelIndex>
#include <QMap>

#include "kdganttconstraint.h"

namespace KDGantt {
    class Constraint::Private : public QSharedData {
    public:
        Private();
        Private( const Private& other );

        inline bool equals( const Private& other ) const {
            /* Due to a Qt bug we have to check separately for invalid indexes */
            return (start==other.start || (!start.isValid() && !other.start.isValid()))
                && (end==other.end || (!end.isValid() && !other.end.isValid()))
                && type==other.type
                && relationType==other.relationType
                && data==other.data;
        }

        QPersistentModelIndex start;
        QPersistentModelIndex end;
        Type type;
        RelationType relationType;
        QMap< int, QVariant > data;
    };
}

#endif /* KDGANTTCONSTRAINT_P_H */

