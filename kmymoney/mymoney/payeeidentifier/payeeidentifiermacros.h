/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

# if defined(MAKE_KMM_PAYEEIDENTIFIER_LIB)
/* We are building this library */
#  define PAYEEIDENTIFIER_EXPORT KDE_EXPORT
# else
/* We are using this library */
#  define PAYEEIDENTIFIER_EXPORT KDE_IMPORT
# endif

#ifndef KMM_UNIT_TESTABLE
#  define KMM_UNIT_TESTABLE
#endif