/***************************************************************************
                          kmymoneyfile.h  -  description
                             -------------------
    begin                : Mon Jun 10 2002
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
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYFILE_H
#define KMYMONEYFILE_H

/*
 * This file is currently not used anymore, but kept here for reference purposes
 */
#if 0
#include "mymoneyaccount.h"
class MyMoneyStorageMgr;

/**
  *@author Michael Edwardes
  */

class KMyMoneyFile
{
private:
  // static KMyMoneyFile *_instance;
  // MyMoneyFile *m_file;
  MyMoneyStorageMgr *m_storage;
  bool m_open;

protected:
  // KMyMoneyFile(const QString&);

public:
  KMyMoneyFile();
  ~KMyMoneyFile();
//  static KMyMoneyFile *instance();

  // MyMoneyFile* file();
  MyMoneyStorageMgr* storage();
  void reset();
  void open();
  void close();
  bool isOpen();

};
#endif
#endif
