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

#ifndef KDGANTTTREEVIEWROWCONTROLLER_P_H
#define KDGANTTTREEVIEWROWCONTROLLER_P_H

#include "kdganttabstractrowcontroller.h"

#include <QTreeView>

QT_BEGIN_NAMESPACE
class QAbstractProxyModel;
QT_END_NAMESPACE

namespace KDGantt {
    class TreeViewRowController::Private {
    public:
        class HackTreeView : public QTreeView {
        public:
            using QTreeView::verticalOffset;
#if QT_VERSION >= 0x040300
            using QTreeView::rowHeight;
#else
            using QTreeView::indexRowSizeHint;
#endif
        };
        HackTreeView* treeview;
        QAbstractProxyModel* proxy;
    };
}

#endif /* KDGANTTTREEVIEWROWCONTROLLER_P_H */

