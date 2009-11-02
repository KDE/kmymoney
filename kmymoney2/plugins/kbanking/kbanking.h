/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifndef KBANKING_KBANKING_H
#define KBANKING_KBANKING_H

#include <aqbanking/version.h>
#include <aqbanking/banking.h>
#include <qbanking/qbanking.h>

#ifndef AQB_MAKE_VERSION
  #define AQB_MAKE_VERSION(a,b,c,d) (((a)<<24) | ((b)<<16) | (c<<8) | (d))
#endif

#ifndef AQBANKING_VERSION
  #define AQBANKING_VERSION AQB_MAKE_VERSION(AQBANKING_VERSION_MAJOR,AQBANKING_VERSION_MINOR,AQBANKING_VERSION_PATCHLEVEL,AQBANKING_VERSION_BUILD)
#endif

#ifndef AQB_IS_VERSION
  #define AQB_IS_VERSION(a,b,c,d) (AQBANKING_VERSION >= AQB_MAKE_VERSION(a,b,c,d))
#endif

#ifndef KBANKING_GUIID
  #if AQB_IS_VERSION(3,9,0,0)
    #define onlineInit() onlineInit(0)
    #define onlineFini() onlineFini(0)
    #define AB_BANKING_GETACCOUNTBYALIAS(a, b) AB_Banking_GetAccountByAlias(a, b, 0)
  #else
    #define AB_BANKING_GETACCOUNTBYALIAS(a, b) AB_Banking_GetAccountByAlias(a, b)
  #endif
#endif


#include <list>


class KBanking: public QBanking {
private:
  AB_JOB_LIST2 *_jobQueue;

public:
  KBanking(const char *appname,
           const char *cfgDir=0);
  virtual ~KBanking();

  int init();
  int fini();

  int executeQueue(AB_IMEXPORTER_CONTEXT *ctx);

  int enqueueJob(AB_JOB *j);
  int dequeueJob(AB_JOB *j);
  std::list<AB_JOB*> getEnqueuedJobs();

};




#endif /* KBANKING_KBANKING_H */


