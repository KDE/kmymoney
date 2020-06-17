/***************************************************************************
                          main.cpp
                             -------------------
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




// ----------------------------------------------------------------------------
// Project Includes

#include "kmmapplication.h"
#include "kmymoney.h"

#include <csignal>

KMyMoney* kmymoney;

int main(int argc, char *argv[])
{
#ifdef KMM_DEBUG
  // make sure the DOM attributes are stored in the same order
  // each time a file is saved. Don't use QHash for security
  // relevant things from here on. See
  // https://doc.qt.io/qt-5/qhash.html#algorithmic-complexity-attacks
  // for details.
  qSetGlobalQHashSeed(0);
#endif

  raise(SIGSTOP);

  /**
   * Create application
   */
  KMMApplication app(argc, argv, kmymoney);

  // TODO: handle the returned value
  //return rc;
  return 0;
}
