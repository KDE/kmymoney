/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file
 * @short A C++ wrapper of the main aqbanking interface
 */

// krazy:exclude=license

#ifndef AQ_BANKING_CPP_H
#define AQ_BANKING_CPP_H


#include <aqbanking/banking.h>
#include <aqbanking/system.h>

#include <list>
#include <string>


/**
 * @brief A C++ binding for the C module @ref AB_BANKING
 *
 * This class simply is a C++ binding for the C module @ref AB_BANKING.
 * It redirects C callbacks used by AB_BANKING to virtual functions in
 * this class. It als transforms some return values inconveniant for
 * C++ into STL objects (such as "list<T>").
 *
 * @ingroup G_AB_CPP_INTERFACE
 *
 * @author Martin Preuss<martin@aquamaniac.de>
 */
class AB_Banking {
private:
  AB_BANKING *_banking;

public:
  AB_Banking(const char *appname,
          const char *fname);
  virtual ~AB_Banking();


  AB_BANKING *getCInterface();


  /**
   * See @ref AB_Banking_Init
   */
  virtual int init();

  /**
   * See @ref AB_Banking_Fini
   */
  virtual int fini();


  /**
   * See @ref AB_Banking_OnlineInit
   */
  int onlineInit();

  /**
   * See @ref AB_Banking_OnlineFini
   */
  int onlineFini();

  /**
   * Loads a backend with the given name. You can use
   * @ref AB_Banking_GetProviderDescrs to retrieve a list of available
   * backends. Such a backend can then be asked to return an account list.
   */
  AB_PROVIDER *getProvider(const char *name);


  /**
   * Returns the application name as given to @ref AB_Banking_new.
   */
  const char *getAppName();

  /**
   * Returns a list of pointers to currently known accounts.
   * Please note that the pointers in this list are still owned by
   * AqBanking, so you MUST NOT free them.
   * However, destroying the list will not free the accounts, so it is
   * safe to do that.
   */
  std::list<AB_ACCOUNT*> getAccounts();

  /**
   * This function does an account lookup based on the given unique id.
   * This id is assigned by AqBanking when an account is created.
   * The pointer returned is still owned by AqBanking, so you MUST NOT free
   * it.
   */
  AB_ACCOUNT *getAccount(uint32_t uniqueId);

  /**
   * Returns a list of pointers to currently known users.
   * Please note that the pointers in this list are still owned by
   * AqBanking, so you MUST NOT free them.
   * However, destroying the list will not free the users, so it is
   * safe to do that.
   */
  std::list<AB_USER*> getUsers();

  int getUserDataDir(GWEN_BUFFER *buf) const ;
  int getAppUserDataDir(GWEN_BUFFER *buf) const ;

  int loadAppConfig(GWEN_DB_NODE **pDb);
  int saveAppConfig(GWEN_DB_NODE *db);
  int lockAppConfig();
  int unlockAppConfig();

  int loadAppSubConfig(const char *subGroup,
		       GWEN_DB_NODE **pDb);

  int saveAppSubConfig(const char *subGroup,
		       GWEN_DB_NODE *dbSrc);


  int loadSharedConfig(const char *name, GWEN_DB_NODE **pDb);
  int saveSharedConfig(const char *name, GWEN_DB_NODE *db);
  int lockSharedConfig(const char *name);
  int unlockSharedConfig(const char *name);

  int loadSharedSubConfig(const char *name,
			  const char *subGroup,
			  GWEN_DB_NODE **pDb);

  int saveSharedSubConfig(const char *name,
			  const char *subGroup,
			  GWEN_DB_NODE *dbSrc);

  int beginExclUseAccount(AB_ACCOUNT *a);
  int endExclUseAccount(AB_ACCOUNT *a, int abandon);

  int beginExclUseUser(AB_USER *u);
  int endExclUseUser(AB_USER *u, int abandon);

  std::list<std::string> getActiveProviders();

  void setAccountAlias(AB_ACCOUNT *a, const char *alias);


  /** @name Enqueueing, Dequeueing and Executing Jobs
   *
   * Enqueued jobs are preserved across shutdowns. As soon as a job has been
   * sent to the appropriate backend it will be removed from the queue.
   * Only those jobs are saved/reloaded which have been enqueued but never
   * presented to the backend. This means after calling
   * @ref AB_Banking_ExecuteQueue only those jobs are still in the queue which
   * have not been processed (e.g. because they belonged to a second backend
   * but the user aborted while the jobs for a first backend were in process).
   */
  /*@{*/
  /**
   * This function sends all jobs in the list to their corresponding backends
   * and allows that backend to process it.
   */
  virtual int executeJobs(AB_JOB_LIST2 *jl,
			  AB_IMEXPORTER_CONTEXT *ctx);

  /*@}*/

  /**
   * Let the application import a given statement context.
   */
  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             uint32_t flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai, uint32_t flags);

};




#endif /* AQ_BANKING_CPP_H */


