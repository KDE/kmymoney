/*
    SPDX-FileCopyrightText: 2004 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2010-2019 Thomas Baumgart tbaumgart @kde.org
    SPDX-FileCopyrightText: 2016 Christian David christian-david @web.de
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KBANKING_H
#define KBANKING_H

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
class KBankingExt;
class KBAccountSettings;

#include <aqbanking/banking.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "banking.hpp"
#include "kmymoneyplugin.h"
#include "kmymoneyutils.h"
#include "mymoney/onlinejobtyped.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "onlinepluginextended.h"
#include "onlinetasks/sepa/sepaonlinetransfer.h"

/**
  * This class represents the KBanking plugin towards KMymoney.
  * All GUI related issues are handled in this object.
  */
class MyMoneyStatement;
class KBanking : public KMyMoneyPlugin::OnlinePluginExtended
{
    friend class KBankingExt;

    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::OnlinePluginExtended
                 KMyMoneyPlugin::OnlinePlugin)

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit KBanking(QObject *parent, const QVariantList &args);
#else
    explicit KBanking(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~KBanking() override;

    bool importStatement(const MyMoneyStatement& s);

    MyMoneyAccount account(const QString& key, const QString& value) const;

    void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const;

    void protocols(QStringList& protocolList) const override;

    QStringList availableJobs(QString accountId) const override;
    IonlineTaskSettings::ptr settings(QString accountId, QString taskName) override;

    void sendOnlineJob(QList<onlineJob>& jobs) override;

    void plug(KXMLGUIFactory* guiFactory) override;
    void unplug() override;

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
    void setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT_SPEC* ab_acc);

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
    QWidget* accountConfigTab(const MyMoneyAccount& acc, QString& name) override;

    /**
     * Stores the configuration data kept in the widgets created
     * in accountConfigTab() and returns them in a key value container
     * The current settings are accessible through the reference to
     * @a current.
     */
    MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;

    /**
      * Called by the application to map the KMyMoney account @a acc
      * to an AqBanking account. Calls KBanking to set up AqBanking mappings.
      * Returns the necessary settings for the plugin in @a settings and
      * @a true if the mapping was successful.
      */
    bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings) override;

    /**
     * This method translates a MyMoneyAccount to the corresponding AB_ACCOUNT object pointer.
     * If no mapped account can be detected, it returns 0.
     */
    AB_ACCOUNT_SPEC* aqbAccount(const MyMoneyAccount& acc) const;

    /**
     * This is a convenient method for aqbAccount if you have KMyMoney's account id only.
     */
    AB_ACCOUNT_SPEC* aqbAccount(const QString& accountId) const;

    /**
      * Called by the application framework to update the
      * KMyMoney account @a acc with data from the online source.
      * Store the jobs in the outbox in case @a moreAccounts is true
      */
    bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts) override;

    /**
      * Kept for backward compatibility. Use
      * updateAccount(const MyMoneyAccount& acc, bool moreAccounts) instead.
      *
      * @deprecated
      */
    bool updateAccount(const MyMoneyAccount& acc) KMYMONEY_DEPRECATED;

    /**
      * Trigger the password cache timer
      */
    void startPasswordTimer();

    bool enqueTransaction(onlineJobTyped<sepaOnlineTransfer>& job);


protected Q_SLOTS:
    void slotSettings();
    void slotImport();
    void slotClearPasswordCache();
    void executeQueue();

Q_SIGNALS:
    void queueChanged();

private:
    class Private;
    Private* const d;
    KAction*                m_configAction;
    KAction*                m_importAction;
    KBankingExt*            m_kbanking;
    QMap<QString, QString>  m_protocolConversionMap;
    KBAccountSettings*      m_accountSettings;
    int                     m_statementCount;
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
class KBankingExt : public AB_Banking
{
    friend class KBanking;

public:
    explicit KBankingExt(KBanking* parent, const char* appname, const char* fname = 0);
    virtual ~KBankingExt() {}

    int executeQueue(AB_IMEXPORTER_CONTEXT *ctx);

    int enqueueJob(AB_TRANSACTION *j);
    int dequeueJob(AB_TRANSACTION *j);
    std::list<AB_TRANSACTION*> getEnqueuedJobs();
    void transfer();

    virtual bool interactiveImport();

protected:
    int init() final override;
    int fini() final override;

    bool askMapAccount(const MyMoneyAccount& acc);
    QString mappingId(const MyMoneyObject& object) const;

    bool importAccountInfo(AB_IMEXPORTER_CONTEXT *ctx, AB_IMEXPORTER_ACCOUNTINFO *ai, uint32_t flags) final override;
    void _slToStatement(MyMoneyStatement &ks,
                        const MyMoneyAccount&,
                        const AB_SECURITY *sy);
    void _xaToStatement(MyMoneyStatement &ks,
                        const MyMoneyAccount&,
                        const AB_TRANSACTION *t);
    void clearPasswordCache();

private:
    KBanking* m_parent;
    QMap<QString, bool> m_hashMap;
    AB_TRANSACTION_LIST2 *_jobQueue;
    QSet<QString>   m_sepaKeywords;
};

#endif // KBANKING
