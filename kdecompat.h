/***************************************************************************
                          kdecompat.h
                             -------------------
    copyright            : (C) 2004 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDECOMPAT_H
#define _KDECOMPAT_H

#include <kdeversion.h>

#ifndef KDE_MAKE_VERSION
#define KDE_MAKE_VERSION(a,b,c) (((a)<<16) | ((b)<<8) | (c))
#endif

#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) (KDE_VERSION >= KDE_MAKE_VERSION(a,b,c))
#endif

#ifndef QT_IS_VERSION
#define QT_IS_VERSION(a,b,c) (QT_VERSION >= KDE_MAKE_VERSION(a,b,c))
#endif

#if KDE_VERSION < KDE_MAKE_VERSION(3,2,0)
#define KDE_DEPRECATED
#endif

#endif // _KDECOMPAT_H
