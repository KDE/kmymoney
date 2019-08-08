/*
 * Copyright 2004       Martin Preuss <martin@libchipcard.de>
 * Copyright 2004-2019  Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif


#include "banking.hpp"
#include <aqbanking/banking_be.h>
#include <aqbanking/banking_cfg.h>
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>



AB_Banking::AB_Banking(const char *appname, const char *fname)
{
  assert(appname);
  _banking = AB_Banking_new(appname, fname, 0);
}

void AB_Banking::registerFinTs(const char* regKey, const char* version) const
{
#ifdef ENABLE_FINTS_REGISTRATION
  AB_Banking_RuntimeConfig_SetCharValue(_banking, "fintsRegistrationKey", regKey);
  AB_Banking_RuntimeConfig_SetCharValue(_banking, "fintsApplicationVersionString", version);
#endif
}


AB_Banking::~AB_Banking()
{
  DBG_NOTICE(AQBANKING_LOGDOMAIN, "~AB_Banking: Freeing AB_Banking");
  AB_Banking_free(_banking);
}



int AB_Banking::init()
{
  return AB_Banking_Init(_banking);
}



int AB_Banking::fini()
{
  return AB_Banking_Fini(_banking);
}



int AB_Banking::onlineInit()
{
  return AB_Banking_OnlineInit(_banking);
}



int AB_Banking::onlineFini()
{
  return AB_Banking_OnlineFini(_banking);
}



const char *AB_Banking::getAppName()
{
  return AB_Banking_GetAppName(_banking);
}



std::list<AB_ACCOUNT*> AB_Banking::getAccounts()
{
  AB_ACCOUNT_LIST2 *ll;
  std::list<AB_ACCOUNT*> rl;

  ll = AB_Banking_GetAccounts(_banking);
  if (ll) {
    AB_ACCOUNT *a;
    AB_ACCOUNT_LIST2_ITERATOR *it;

    it = AB_Account_List2_First(ll);
    assert(it);
    a = AB_Account_List2Iterator_Data(it);
    assert(a);
    while (a) {
      rl.push_back(a);
      a = AB_Account_List2Iterator_Next(it);
    }
    AB_Account_List2Iterator_free(it);
    AB_Account_List2_free(ll);
  }
  return rl;
}



AB_ACCOUNT *AB_Banking::getAccount(uint32_t uniqueId)
{
  return AB_Banking_GetAccount(_banking, uniqueId);
}



std::list<AB_USER*> AB_Banking::getUsers()
{
  AB_USER_LIST2 *ll;
  std::list<AB_USER*> rl;

  ll = AB_Banking_GetUsers(_banking);
  if (ll) {
    AB_USER *a;
    AB_USER_LIST2_ITERATOR *it;

    it = AB_User_List2_First(ll);
    assert(it);
    a = AB_User_List2Iterator_Data(it);
    assert(a);
    while (a) {
      rl.push_back(a);
      a = AB_User_List2Iterator_Next(it);
    }
    AB_User_List2Iterator_free(it);
    AB_User_List2_free(ll);
  }
  return rl;
}



int AB_Banking::getUserDataDir(GWEN_BUFFER *buf) const
{
  return AB_Banking_GetUserDataDir(_banking, buf);
}



int AB_Banking::getAppUserDataDir(GWEN_BUFFER *buf) const
{
  return AB_Banking_GetAppUserDataDir(_banking, buf);
}



AB_BANKING *AB_Banking::getCInterface()
{
  return _banking;
}



std::list<std::string> AB_Banking::getActiveProviders()
{
  const GWEN_STRINGLIST *sl;
  std::list<std::string> l;

  sl = AB_Banking_GetActiveProviders(_banking);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se = GWEN_StringList_FirstEntry(sl);
    assert(se);
    while (se) {
      const char *p;

      p = GWEN_StringListEntry_Data(se);
      assert(p);
      l.push_back(p);
      se = GWEN_StringListEntry_Next(se);
    } /* while */
  }
  return l;
}


AB_PROVIDER *AB_Banking::getProvider(const char *name)
{
  return AB_Banking_GetProvider(_banking, name);
}



bool AB_Banking::importContext(AB_IMEXPORTER_CONTEXT *ctx, uint32_t flags)
{
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ai = AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (ai) {
    if (!importAccountInfo(ai, flags))
      return false;
    ai = AB_ImExporterContext_GetNextAccountInfo(ctx);
  }

  return true;
}



bool AB_Banking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO*, uint32_t)
{
  return false;
}



int AB_Banking::executeJobs(AB_JOB_LIST2 *jl, AB_IMEXPORTER_CONTEXT *ctx)
{
  return AB_Banking_ExecuteJobs(_banking, jl, ctx);
}



int AB_Banking::loadSharedConfig(const char *name, GWEN_DB_NODE **pDb)
{
  return AB_Banking_LoadSharedConfig(_banking, name, pDb);
}



int AB_Banking::saveSharedConfig(const char *name, GWEN_DB_NODE *db)
{
  return AB_Banking_SaveSharedConfig(_banking, name, db);
}



int AB_Banking::lockSharedConfig(const char *name)
{
  return AB_Banking_LockSharedConfig(_banking, name);
}



int AB_Banking::unlockSharedConfig(const char *name)
{
  return AB_Banking_UnlockSharedConfig(_banking, name);
}



int AB_Banking::loadSharedSubConfig(const char *name,
                                    const char *subGroup,
                                    GWEN_DB_NODE **pDb)
{
  GWEN_DB_NODE *dbShared = NULL;
  int rv;

  rv = loadSharedConfig(name, &dbShared);
  if (rv < 0) {
    DBG_ERROR(0, "Unable to load config (%d)", rv);
    GWEN_DB_Group_free(dbShared);
    return rv;
  } else {
    GWEN_DB_NODE *dbSrc;

    dbSrc = GWEN_DB_GetGroup(dbShared,
                             GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                             subGroup);
    if (dbSrc) {
      *pDb = GWEN_DB_Group_dup(dbSrc);
    } else {
      *pDb = GWEN_DB_Group_new("config");
    }
    GWEN_DB_Group_free(dbShared);

    return 0;
  }
}



int AB_Banking::saveSharedSubConfig(const char *name,
                                    const char *subGroup,
                                    GWEN_DB_NODE *dbSrc)
{
  GWEN_DB_NODE *dbShared = NULL;
  int rv;

  rv = lockSharedConfig(name);
  if (rv < 0) {
    DBG_ERROR(0, "Unable to lock config");
    return rv;
  } else {
    rv = loadSharedConfig(name, &dbShared);
    if (rv < 0) {
      DBG_ERROR(0, "Unable to load config (%d)", rv);
      unlockSharedConfig(name);
      return rv;
    } else {
      GWEN_DB_NODE *dbDst;

      dbDst = GWEN_DB_GetGroup(dbShared,
                               GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                               subGroup);
      assert(dbDst);
      if (dbSrc)
        GWEN_DB_AddGroupChildren(dbDst, dbSrc);
      rv = saveSharedConfig(name, dbShared);
      if (rv < 0) {
        DBG_ERROR(0, "Unable to store config (%d)", rv);
        unlockSharedConfig(name);
        GWEN_DB_Group_free(dbShared);
        return rv;
      }
      GWEN_DB_Group_free(dbShared);
    }

    rv = unlockSharedConfig(name);
    if (rv < 0) {
      DBG_ERROR(0, "Unable to unlock config (%d)", rv);
      return rv;
    }
  }
  return 0;
}


int AB_Banking::loadAppConfig(GWEN_DB_NODE **pDb)
{
  return AB_Banking_LoadAppConfig(_banking, pDb);
}



int AB_Banking::saveAppConfig(GWEN_DB_NODE *db)
{
  return AB_Banking_SaveAppConfig(_banking, db);
}



int AB_Banking::lockAppConfig()
{
  return AB_Banking_LockAppConfig(_banking);
}



int AB_Banking::unlockAppConfig()
{
  return AB_Banking_UnlockAppConfig(_banking);
}



int AB_Banking::loadAppSubConfig(const char *subGroup,
                                 GWEN_DB_NODE **pDb)
{
  GWEN_DB_NODE *dbApp = NULL;
  int rv;

  rv = loadAppConfig(&dbApp);
  if (rv < 0) {
    DBG_ERROR(0, "Unable to load config (%d)", rv);
    GWEN_DB_Group_free(dbApp);
    return rv;
  } else {
    GWEN_DB_NODE *dbSrc;

    dbSrc = GWEN_DB_GetGroup(dbApp,
                             GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                             subGroup);
    if (dbSrc) {
      *pDb = GWEN_DB_Group_dup(dbSrc);
    } else {
      *pDb = GWEN_DB_Group_new("config");
    }
    GWEN_DB_Group_free(dbApp);

    return 0;
  }
}



int AB_Banking::saveAppSubConfig(const char *subGroup,
                                 GWEN_DB_NODE *dbSrc)
{
  GWEN_DB_NODE *dbApp = NULL;
  int rv;

  rv = lockAppConfig();
  if (rv < 0) {
    DBG_ERROR(0, "Unable to lock config");
    return rv;
  } else {
    rv = loadAppConfig(&dbApp);
    if (rv < 0) {
      DBG_ERROR(0, "Unable to load config (%d)", rv);
      unlockAppConfig();
      return rv;
    } else {
      GWEN_DB_NODE *dbDst;

      dbDst = GWEN_DB_GetGroup(dbApp,
                               GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                               subGroup);
      assert(dbDst);
      if (dbSrc)
        GWEN_DB_AddGroupChildren(dbDst, dbSrc);
      rv = saveAppConfig(dbApp);
      if (rv < 0) {
        DBG_ERROR(0, "Unable to store config (%d)", rv);
        unlockAppConfig();
        GWEN_DB_Group_free(dbApp);
        return rv;
      }
      GWEN_DB_Group_free(dbApp);
    }

    rv = unlockAppConfig();
    if (rv < 0) {
      DBG_ERROR(0, "Unable to unlock config (%d)", rv);
      return rv;
    }
  }
  return 0;
}


int AB_Banking::beginExclUseAccount(AB_ACCOUNT *a)
{
  return AB_Banking_BeginExclUseAccount(_banking, a);
}



int AB_Banking::endExclUseAccount(AB_ACCOUNT *a, int abandon)
{
  return AB_Banking_EndExclUseAccount(_banking, a, abandon);
}



int AB_Banking::beginExclUseUser(AB_USER *u)
{
  return AB_Banking_BeginExclUseUser(_banking, u);
}



int AB_Banking::endExclUseUser(AB_USER *u, int abandon)
{
  return AB_Banking_EndExclUseUser(_banking, u, abandon);
}


void AB_Banking::setAccountAlias(AB_ACCOUNT *a, const char *alias)
{
  AB_Banking_SetAccountAlias(_banking, a, alias);
}





