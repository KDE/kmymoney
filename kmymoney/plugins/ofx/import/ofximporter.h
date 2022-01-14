/*
    SPDX-FileCopyrightText: 2005 Ace Jones acejones @users.sourceforge.net
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart tbaumgart @kde.org
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef OFXIMPORTER_H
#define OFXIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Library Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "mymoneykeyvaluecontainer.h"

/**
@author Ace Jones
*/
class MyMoneyAccount;
class MyMoneyStatement;
class OFXImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin, public KMyMoneyPlugin::OnlinePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::ImporterPlugin)
    Q_INTERFACES(KMyMoneyPlugin::OnlinePlugin)

public:
    explicit OFXImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~OFXImporter() override;

    /**
      * This method returns the list of the MIME types that this plugin handles,
      * i.e. {"application/x-ofx", "application/vnd.intu.qfx", "application/x-ofc"}.
      *
      * @return QStringList List of MIME types
      */
    QStringList formatMimeTypes() const override;

    /**
      * This method returns whether this plugin is able to import
      * the OFX-formatted files.
      *
      * @param filename Fully-qualified pathname to a file
      *
      * @return bool Whether the indicated file is importable by this plugin
      */
    bool isMyFormat(const QString& filename) const override;

    /**
      * Import a file
      *
      * @param filename File to import
      *
      * @return bool Whether the import was successful.
      */
    bool import(const QString& filename) override;

    /**
      * Returns the error result of the last import
      *
      * @return QString English-language name of the error encountered in the
      *  last import, or QString() if it was successful.
      *
      */
    QString lastError() const override;

    /**
      * Returns a pointer to a widget that will be added as tab to
      * the account edit dialog. @sa KNewAccountDlg. The data of the
      * current account is passed as const reference @a acc. @a name references
      * a QString that will receive the name of the tab to be shown in the dialog.
      */
    QWidget* accountConfigTab(const MyMoneyAccount& acc, QString& name) override;

    /**
      * Retrieves the online banking settings and updates the password in the KDE wallet.
      * The caller has the choice to pass a MyMoneyKeyValueContainer with the @a current
      * settings. Only those are modified that are used by the plugin.
      */
    MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;

    MyMoneyAccount account(const QString& key, const QString& value) const;

    void protocols(QStringList& protocolList) const override;

    bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings) override;
    bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts) override;

protected Q_SLOTS:
    void slotImportFile();
    void slotImportFile(const QString& url);

protected:
    void createActions();
    void addnew();
    MyMoneyStatement& back();
    bool isValid() const;
    void setValid();
    void addInfo(const QString& _msg);
    void addWarning(const QString& _msg);
    void addError(const QString& _msg);
    const QStringList& infos() const;       // krazy:exclude=spelling
    const QStringList& warnings() const;
    const QStringList& errors() const;

    bool storeStatements(const QList<MyMoneyStatement> &statements);
    QStringList importStatement(const MyMoneyStatement &s);


    static int ofxTransactionCallback(struct OfxTransactionData, void*);
    static int ofxStatementCallback(struct OfxStatementData, void*);
    static int ofxAccountCallback(struct OfxAccountData, void*);
    static int ofxStatusCallback(struct OfxStatusData, void*);
    static int ofxSecurityCallback(struct OfxSecurityData, void*);

private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
