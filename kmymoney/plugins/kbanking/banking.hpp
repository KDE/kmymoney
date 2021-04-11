/*
    SPDX-FileCopyrightText: 2018 Martin Preuss <martin@libchipcard.de>
    SPDX-FileCopyrightText: 2004-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** @file
 * @short A C++ wrapper of the main aqbanking interface
 */

// krazy:excludeall=license

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
  std::list<AB_ACCOUNT_SPEC*> getAccounts();

  /**
   * This function does an account lookup based on the given unique id.
   * This id is assigned by AqBanking when an account is created.
   * The pointer returned is still owned by AqBanking, so you MUST NOT free
   * it.
   */
  AB_ACCOUNT_SPEC *getAccount(uint32_t uniqueId);

  std::list<std::string> getActiveProviders();

  int getUserDataDir(GWEN_BUFFER *buf) const ;

  void setAccountAlias(AB_ACCOUNT_SPEC *a, const char *alias);

  /**
   * Provide interface to setup ZKA FinTS registration
   */
  void registerFinTs(const char* regKey, const char* version) const;

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
  virtual int executeJobs(AB_TRANSACTION_LIST2 *jl,
                          AB_IMEXPORTER_CONTEXT *ctx);

  /*@}*/

  /**
   * Let the application import a given statement context.
   */
  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             uint32_t flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_CONTEXT *ctx, AB_IMEXPORTER_ACCOUNTINFO *ai, uint32_t flags);

};




#endif /* AQ_BANKING_CPP_H */


