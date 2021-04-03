/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SQLSTORAGE_H
#define SQLSTORAGE_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyStorageMgr;
class QUrlQuery;

class SQLStorage : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
    explicit SQLStorage(QObject *parent, const QVariantList &args);
    ~SQLStorage() override;

    QAction *m_openDBaction;
    QAction *m_saveAsDBaction;
    QAction *m_generateDB;

    MyMoneyStorageMgr *open(const QUrl &url) override;
    bool save(const QUrl &url) override;
    bool saveAs() override;
    eKMyMoney::StorageType storageType() const override;
    QString fileExtension() const override;
    QUrl openUrl() const override;

protected:
    void createActions();

private:
    /**
     * Saves the data into permanent storage on a new or empty SQL database.
     *
     * @param url The pseudo URL of the database
     *
     * @retval false save operation failed
     * @retval true save operation was successful
     */
    bool saveAsDatabase(const QUrl &url);

    QUrlQuery convertOldUrl(const QUrl& url);

    /**
     * The full url (incl. password) used to open a database
     */
    QUrl dbUrl;

private Q_SLOTS:
    void slotOpenDatabase();
    void slotGenerateSql();
};

#endif
