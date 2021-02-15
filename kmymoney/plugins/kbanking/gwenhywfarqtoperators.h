/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
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
