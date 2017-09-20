/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GWENHYWFARQTOPERATORS_H
#define GWENHYWFARQTOPERATORS_H

#include <gwenhywfar/stringlist.h>

class QString;
class QStringList;

/**
 * @defgroup gwenhywfarqtoperators Helper functions for using gwenhywfar with Qt
 *
 * These functions are similar to original gwenhywfar ones. They are meant to glue qt and gwenhywfar.
 *
 * @{
 */

/**
 * @brief Create GWEN_STRINGLIST from QStringList
 */
GWEN_STRINGLIST* GWEN_StringList_fromQStringList(const QStringList& input);

/**
 * @brief Create GWEN_STRINGLIST from QString
 */
GWEN_STRINGLIST* GWEN_StringList_fromQString(const QString& input);

/** @} */ // end of gwenhywfarqtoperators

#endif // GWENHYWFARQTOPERATORS_H
