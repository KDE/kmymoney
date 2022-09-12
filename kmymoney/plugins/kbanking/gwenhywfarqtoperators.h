/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GWENHYWFARQTOPERATORS_H
#define GWENHYWFARQTOPERATORS_H

#include <gwenhywfar/stringlist.h>

class QString;
#include "qcontainerfwd.h"

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
