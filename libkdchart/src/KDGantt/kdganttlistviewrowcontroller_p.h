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

#ifndef KDGANTTLISTVIEWROWCONTROLLER_P_H
#define KDGANTTLISTVIEWROWCONTROLLER_P_H

#include "kdganttlistviewrowcontroller.h"

#include <QListView>

namespace KDGantt {
    class ListViewRowController::Private {
    public:
        class HackListView : public QListView {
        public:
            using QListView::verticalOffset;
            using QListView::setViewportMargins;
        };

        Private(QListView* lv, QAbstractProxyModel* pm )
            : listview(lv), proxy(pm) {}
        QListView* listview;
        QAbstractProxyModel* proxy;
    };
}

#endif /* KDGANTTLISTVIEWROWCONTROLLER_P_H */

