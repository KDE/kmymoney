/*
 * Copyright 2018       Martin Preuss <martin@libchipcard.de>
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

#include <aqbanking/banking.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



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



const char *AB_Banking::getAppName()
{
  return AB_Banking_GetAppName(_banking);
}



AB_ACCOUNT_SPEC *AB_Banking::getAccount(uint32_t uniqueId)
{
  int rv;
  AB_ACCOUNT_SPEC *as=NULL;

  rv=AB_Banking_GetAccountSpecByUniqueId(_banking, uniqueId, &as);
  if (rv<0) {
    DBG_ERROR(0, "Account spec not found (%d)", rv);
    return NULL;
  }
  return as;
}



std::list<AB_ACCOUNT_SPEC*> AB_Banking::getAccounts() {
  std::list<AB_ACCOUNT_SPEC*> accountSpecList;
  AB_ACCOUNT_SPEC_LIST *abAccountSpecList=NULL;
  int rv;

  rv=AB_Banking_GetAccountSpecList(_banking, &abAccountSpecList);
  if (rv>=0) {
    AB_ACCOUNT_SPEC *as;

    while( (as=AB_AccountSpec_List_First(abAccountSpecList)) ) {
      AB_AccountSpec_List_Del(as);
      accountSpecList.push_back(as);
      as=AB_AccountSpec_List_Next(as);
    }
  }
  AB_AccountSpec_List_free(abAccountSpecList);
  return accountSpecList;
}



int AB_Banking::getUserDataDir(GWEN_BUFFER *buf) const
{
  return AB_Banking_GetUserDataDir(_banking, buf);
}



AB_BANKING *AB_Banking::getCInterface()
{
  return _banking;
}



bool AB_Banking::importContext(AB_IMEXPORTER_CONTEXT *ctx, uint32_t flags)
{
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ai = AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (ai) {
    if (!importAccountInfo(ctx, ai, flags))
      return false;
    ai = AB_ImExporterAccountInfo_List_Next(ai);
  }

  return true;
}



bool AB_Banking::importAccountInfo(AB_IMEXPORTER_CONTEXT*,
                                   AB_IMEXPORTER_ACCOUNTINFO*,
                                   uint32_t)
{
  return false;
}



int AB_Banking::executeJobs(AB_TRANSACTION_LIST2 *jl, AB_IMEXPORTER_CONTEXT *ctx)
{
  return AB_Banking_SendCommands(_banking, jl, ctx);
}



std::list<std::string> AB_Banking::getActiveProviders() {
  std::list<std::string> stringList;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

  pdl=AB_Banking_GetProviderDescrs(_banking);
  if (pdl) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

    it=GWEN_PluginDescription_List2_First(pdl);
    if (it) {
      GWEN_PLUGIN_DESCRIPTION *pd;

      pd=GWEN_PluginDescription_List2Iterator_Data(it);
      while(pd) {
        const char *s;

        s=GWEN_PluginDescription_GetName(pd);
        if (s && *s)
          stringList.push_back(s);
        pd=GWEN_PluginDescription_List2Iterator_Next(it);
      }
      GWEN_PluginDescription_List2Iterator_free(it);
    }
    GWEN_PluginDescription_List2_freeAll(pdl);
  }

  return stringList;
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



void AB_Banking::setAccountAlias(AB_ACCOUNT_SPEC *a, const char *alias)
{
  AB_Banking_SetAccountSpecAlias(_banking, a, alias);
}
