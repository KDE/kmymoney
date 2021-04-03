/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
