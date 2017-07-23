/***************************************************************************
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2010  Thomas Baumgart ipwizard@users.sourceforge.net        *
 *   Copyright 2016  Christian David christian-david@web.de                *
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
#ifndef MYMONEYBANKING_H
#define MYMONEYBANKING_H

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE & Library Includes

class KAction;
class QBanking;
class KMyMoneyBanking;
class KBAccountSettings;

#include <aqbanking/banking.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "onlinepluginextended.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

#include "mymoney/onlinejobtyped.h"
#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"

#include "banking.hpp"

/**
  * This class represents the KBanking plugin towards KMymoney.
  * All GUI related issues are handled in this object.
  */
class KBankingPlugin : public KMyMoneyPlugin::OnlinePluginExtended
{
  friend class KMyMoneyBanking;

  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.kbanking" FILE "kbanking.json")

public:
  KBankingPlugin();
  virtual ~KBankingPlugin();

  bool importStatement(const MyMoneyStatement& s);

  const MyMoneyAccount& account(const QString& key, const QString& value) const;

  void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const;

  void protocols(QStringList& protocolList) const;

  QStringList availableJobs(QString accountId);
  IonlineTaskSettings::ptr settings(QString accountId, QString taskName);

  void sendOnlineJob(QList<onlineJob>& jobs);

  virtual void plug() override;
  virtual void unplug() override;

private:
  /**
    * creates the action objects available through the application menus
    */
  void createActions();

  /**
    * creates the context menu
    */
  void createContextMenu();

  /**
    * checks whether a given KMyMoney account with id @p id is
    * already mapped or not.
    *
    * @param acc reference to KMyMoney account object
    * @retval false account is not mapped to an AqBanking account
    * @retval true account is mapped to an AqBanking account
    */
  bool accountIsMapped(const MyMoneyAccount& acc);

  /**
   * sets up the reference string consisting out of BLZ and account number
   * in the KMyMoney object so that we can find it later on when importing data.
   */
  void setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT* ab_acc);

  /**
   * Returns the value of the parameter @a s with all leading 0's stripped.
   */
  QString stripLeadingZeroes(const QString& s) const;

  /**
   * Prefills the protocol conversion list to allow mapping
   * of AqBanking internal names to external names
   */
  void loadProtocolConversion();

  /**
   * Creates an additional tab widget for the account edit dialog
   * to represent the necessary parameters for online banking
   * through AqBanking.
   */
  QWidget* accountConfigTab(const MyMoneyAccount& acc, QString& name);

  /**
   * Stores the configuration data kept in the widgets created
   * in accountConfigTab() and returns them in a key value container
   * The current settings are accessible through the reference to
   * @a current.
   */
  MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current);

  /**
    * Called by the application to map the KMyMoney account @a acc
    * to an AqBanking account. Calls KBanking to set up AqBanking mappings.
    * Returns the necessary settings for the plugin in @a settings and
    * @a true if the mapping was successful.
    */
  bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings);

  /**
   * This method translates a MyMoneyAccount to the corresponding AB_ACCOUNT object pointer.
   * If no mapped account can be detected, it returns 0.
   */
  AB_ACCOUNT* aqbAccount(const MyMoneyAccount& acc) const;

  /**
   * This is a convenient method for aqbAccount if you have KMyMoney's account id only.
   */
  AB_ACCOUNT* aqbAccount(const QString& accountId) const;

  /**
    * Called by the application framework to update the
    * KMyMoney account @a acc with data from the online source.
    * Store the jobs in the outbox in case @a moreAccounts is true
    */
  bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts);

  /**
    * Kept for backward compatibility. Use
    * updateAccount(const MyMoneyAccount& acc, bool moreAccounts) instead.
    *
    * @deprecated
    */
  bool updateAccount(const MyMoneyAccount& acc);

  /**
    * Trigger the password cache timer
    */
  void startPasswordTimer();

  bool enqueTransaction(onlineJobTyped<sepaOnlineTransfer>& job);


protected slots:
  void slotSettings();
  void slotImport();
  void slotClearPasswordCache();
  void executeQueue();

signals:
  void queueChanged();

private:
  class Private;
  Private* const d;
  KAction*              m_configAction;
  KAction*              m_importAction;
  KMyMoneyBanking*      m_kbanking;
  QMap<QString, QString> m_protocolConversionMap;
  KBAccountSettings* m_accountSettings;

  /**
   * @brief @ref onlineJob "onlineJobs" which are executed at the moment
   * Key is onlineJob->id(). This container is used during execution of jobs.
   */
  QMap<QString, onlineJob> m_onlineJobQueue;
};

/**
  * This class is the special implementation to glue the AB_Banking class
  * with the KMyMoneyPlugin structure.
  */
class KMyMoneyBanking : public AB_Banking
{
  friend class KBankingPlugin;

public:
  KMyMoneyBanking(KBankingPlugin* parent, const char* appname, const char* fname = 0);
  virtual ~KMyMoneyBanking() {};

  int executeQueue(AB_IMEXPORTER_CONTEXT *ctx);

  int enqueueJob(AB_JOB *j);
  int dequeueJob(AB_JOB *j);
  std::list<AB_JOB*> getEnqueuedJobs();
  void transfer();

  virtual bool interactiveImport();

protected:
  int init();
  int fini();

  bool askMapAccount(const MyMoneyAccount& acc);
  QString mappingId(const MyMoneyObject& object) const;

  bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai, uint32_t flags);
  const AB_ACCOUNT_STATUS* _getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai);
  void _xaToStatement(MyMoneyStatement &ks,
                      const MyMoneyAccount&,
                      const AB_TRANSACTION *t);
  void clearPasswordCache();

private:
  KBankingPlugin* m_parent;
  QMap<QString, bool> m_hashMap;
  AB_JOB_LIST2 *_jobQueue;
  QSet<QString>   m_sepaKeywords;
};

#endif
