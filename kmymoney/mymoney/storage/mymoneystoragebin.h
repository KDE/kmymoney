/***************************************************************************
                          imymoneystoragebin.h  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTORAGEBIN_H
#define MYMONEYSTORAGEBIN_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  *@author Thomas Baumgart
  */

  #define VERSION_0_3_3 0x00000006    // MAGIC1 for version 0.33 files
  #define VERSION_0_4_0 0x00000007    // MAGIC1 for version 0.4 files

  #define MAGIC_0_50  0x4B4D794D      // "KMyM" MAGIC1 for version 0.5 files
  #define MAGIC_0_51  0x6F6E6579      // "oney" second part of MAGIC

  #define VERSION_0_50  0x00000010    // Version 0.5 file version info
  #define VERSION_0_51  0x00000011    // use 8 bytes for MyMoneyMoney objects

  // add new definitions above and make sure to adapt MAX_FILE_VERSION below
  #define MIN_FILE_VERSION  VERSION_0_50
  #define MAX_FILE_VERSION  VERSION_0_51

#endif
