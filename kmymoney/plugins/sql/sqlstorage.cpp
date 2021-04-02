/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrlQuery>
#include <QSqlQuery>
#include <QTimer>
#include <QFile>
#include <QSqlDriver>
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "sqlstorage.h"
#include "appinterface.h"
#include "viewinterface.h"
#include "kselectdatabasedlg.h"
#include "kgeneratesqldlg.h"
#include "mymoneyfile.h"
#include "mymoneystoragesql.h"
#include "mymoneyexception.h"
#include "mymoneystoragemgr.h"
#include "icons.h"
#include "kmymoneysettings.h"
#include "kmymoneyenums.h"

using namespace Icons;

QUrlQuery SQLStorage::convertOldUrl(const QUrl& url)
{
    const auto key = QLatin1String("driver");
    // take care and convert some old url to their new counterpart
    QUrlQuery query(url);
    if (query.queryItemValue(key) == QLatin1String("QMYSQL3")) { // fix any old urls
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QMYSQL"));
    } else if (query.queryItemValue(key) == QLatin1String("QSQLITE3")) {
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QSQLITE"));
    }
#ifdef ENABLE_SQLCIPHER
    // Reading unencrypted database with QSQLITE
    // while QSQLCIPHER is available causes crash.
    // QSQLCIPHER can read QSQLITE
    if (query.queryItemValue(key) == QLatin1String("QSQLITE")) {
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QSQLCIPHER"));
    }
#endif
    return query;
}


SQLStorage::SQLStorage(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "sqlstorage"/*must be the same as X-KDE-PluginInfo-Name*/)
{
    Q_UNUSED(args)
    const auto componentName = QLatin1String("sqlstorage");
    const auto rcFileName = QLatin1String("sqlstorage.rc");
    setComponentName(componentName, i18n("SQL storage"));

#ifdef IS_APPIMAGE
    const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
    setXMLFile(rcFilePath);

    const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
    setLocalXMLFile(localRcFilePath);
#else
    setXMLFile(rcFileName);
#endif

    createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: sqlstorage loaded");
}

SQLStorage::~SQLStorage()
{
    actionCollection()->removeAction(m_openDBaction);
    actionCollection()->removeAction(m_generateDB);

    qDebug("Plugins: sqlstorage unloaded");
}

MyMoneyStorageMgr *SQLStorage::open(const QUrl &url)
{
    if (url.scheme() != QLatin1String("sql"))
        return nullptr;

    auto storage = new MyMoneyStorageMgr;
    auto reader = std::make_unique<MyMoneyStorageSql>(storage, url);

    dbUrl = url;
    if (dbUrl.password().isEmpty()) {
        // check if a password is needed. it may be if the URL came from the last/recent file list
        QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite, dbUrl);
        if (!dialog->checkDrivers()) {
            delete dialog;
            return nullptr;
        }
        QUrlQuery query = convertOldUrl(dbUrl);
        // if we need to supply a password, then show the dialog
        // otherwise it isn't needed
        if ((query.queryItemValue("secure").toLower() == QLatin1String("yes")) && dbUrl.password().isEmpty()) {
            if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
                dbUrl = dialog->selectedURL();
            } else {
                delete dialog;
                return nullptr;
            }
        }
        delete dialog;
    }

    // create a copy that we can override temporarily
    // and use it from now on
    QUrl dbURL(dbUrl);

    bool retry = true;
    while (retry) {
        switch (reader->open(dbURL, QIODevice::ReadWrite)) {
        case 0: // opened okay
            retry = false;
            break;
        case 1: // permanent error
            KMessageBox::detailedError(nullptr,
                                       i18n("Cannot open database %1\n", dbURL.toDisplayString()),
                                       reader->lastError());
            delete storage;
            return nullptr;
        case -1: // retryable error
            if (KMessageBox::warningYesNo(nullptr, reader->lastError(), PACKAGE) == KMessageBox::No) {
                delete storage;
                return nullptr;
            } else {
                QUrlQuery query(dbURL);
                const QString optionKey = QLatin1String("options");
                QString options = query.queryItemValue(optionKey);
                if(!options.isEmpty()) {
                    options += QLatin1Char(',');
                }
                options += QLatin1String("override");
                query.removeQueryItem(QLatin1String("mode"));
                query.removeQueryItem(optionKey);
                query.addQueryItem(optionKey, options);
                dbURL.setQuery(query);
            }
            break;
        case 2: // bad password
        case 3: // unsupported operation
            delete storage;
            return nullptr;
        }
    }
    // single user mode; read some of the data into memory
    // FIXME - readFile no longer relevant?
    // tried removing it but then got no indication that loading was complete
    // also, didn't show home page
//  reader->setProgressCallback(&KMyMoneyView::progressCallback);
    if (!reader->readFile()) {
        KMessageBox::detailedError(nullptr,
                                   i18n("An unrecoverable error occurred while reading the database"),
                                   reader->lastError().toLatin1(),
                                   i18n("Database malfunction"));
        delete storage;
        return nullptr;
    }
//  reader->setProgressCallback(0);
    return storage;
}

QUrl SQLStorage::openUrl() const
{
    return dbUrl;
}

bool SQLStorage::save(const QUrl &url)
{
    auto rc = false;
    if (!appInterface()->fileOpen()) {
        KMessageBox::error(nullptr, i18n("Tried to access a file when it has not been opened"));
        return (rc);
    }
    auto writer = new MyMoneyStorageSql(MyMoneyFile::instance()->storage(), url);
    writer->open(url, QIODevice::ReadWrite);
//  writer->setProgressCallback(&KMyMoneyView::progressCallback);
    if (!writer->writeFile()) {
        KMessageBox::detailedError(nullptr,
                                   i18n("An unrecoverable error occurred while writing to the database.\n"
                                        "It may well be corrupt."),
                                   writer->lastError().toLatin1(),
                                   i18n("Database malfunction"));
        rc =  false;
    } else {
        rc = true;
    }
    writer->setProgressCallback(0);
    delete writer;
    return rc;
}

bool SQLStorage::saveAs()
{
    auto rc = false;
    QUrl oldUrl;
    // in event of it being a database, ensure that all data is read into storage for saveas
    if (appInterface()->isDatabase())
        oldUrl = appInterface()->filenameURL().isEmpty() ? appInterface()->lastOpenedURL() : appInterface()->filenameURL();

    QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::WriteOnly);
    QUrl url = oldUrl;
    if (!dialog->checkDrivers()) {
        delete dialog;
        return rc;
    }

    while (oldUrl == url && dialog->exec() == QDialog::Accepted && dialog != 0) {
        url = dialog->selectedURL();
        // If the protocol is SQL for the old and new, and the hostname and database names match
        // Let the user know that the current database cannot be saved on top of itself.
        if (url.scheme() == "sql" && oldUrl.scheme() == "sql"
                && oldUrl.host() == url.host()
                && QUrlQuery(oldUrl).queryItemValue("driver") == QUrlQuery(url).queryItemValue("driver")
                && oldUrl.path().right(oldUrl.path().length() - 1) == url.path().right(url.path().length() - 1)) {
            KMessageBox::sorry(nullptr, i18n("Cannot save to current database."));
        } else {
            try {
                rc = saveAsDatabase(url);
            } catch (const MyMoneyException &e) {
                KMessageBox::sorry(nullptr, i18n("Cannot save to current database: %1", QString::fromLatin1(e.what())));
            }
        }
    }
    delete dialog;

    if (rc) {
        //KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
        //if(p)
        appInterface()->addToRecentFiles(url);
        appInterface()->writeLastUsedFile(url.toDisplayString(QUrl::PreferLocalFile));
        appInterface()->writeFilenameURL(url);
    }
    return rc;
}

eKMyMoney::StorageType SQLStorage::storageType() const
{
    return eKMyMoney::StorageType::SQL;
}

QString SQLStorage::fileExtension() const
{
    return i18n("Database files (*.db *.sql)");
}

void SQLStorage::createActions()
{
    m_openDBaction = actionCollection()->addAction("open_database");
    m_openDBaction->setText(i18n("Open database..."));
    m_openDBaction->setIcon(Icons::get(Icon::OpenDatabase));
    connect(m_openDBaction, &QAction::triggered, this, &SQLStorage::slotOpenDatabase);

    m_generateDB = actionCollection()->addAction("tools_generate_sql");
    m_generateDB->setText(i18n("Generate Database SQL"));
    connect(m_generateDB, &QAction::triggered, this, &SQLStorage::slotGenerateSql);
}

void SQLStorage::slotOpenDatabase()
{
    QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite);
    if (!dialog->checkDrivers()) {
        delete dialog;
        return;
    }

    if (dialog->exec() == QDialog::Accepted && dialog != 0) {
        auto url = dialog->selectedURL();
        QUrl newurl = url;
        if ((newurl.scheme() == QLatin1String("sql"))) {
            QUrlQuery query = convertOldUrl(newurl);
            newurl.setQuery(query);

            // check if a password is needed. it may be if the URL came from the last/recent file list
            dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite, newurl);
            if (!dialog->checkDrivers()) {
                delete dialog;
                return;
            }
            // if we need to supply a password, then show the dialog
            // otherwise it isn't needed
            if ((query.queryItemValue("secure").toLower() == QLatin1String("yes")) && newurl.password().isEmpty()) {
                if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
                    newurl = dialog->selectedURL();
                } else {
                    delete dialog;
                    return;
                }
            }
            delete dialog;
        }

        appInterface()->slotFileOpenRecent(newurl);
    }
    delete dialog;
}

void SQLStorage::slotGenerateSql()
{
    QPointer<KGenerateSqlDlg> editor = new KGenerateSqlDlg(nullptr);
    editor->setObjectName("Generate Database SQL");
    editor->exec();
    delete editor;
}

bool SQLStorage::saveAsDatabase(const QUrl &url)
{
    auto writer = new MyMoneyStorageSql(MyMoneyFile::instance()->storage(), url);
    auto canWrite = false;
    switch (writer->open(url, QIODevice::WriteOnly)) {
    case 0:
        canWrite = true;
        break;
    case -1: // dbase already has data, see if he wants to clear it out
        if (KMessageBox::warningContinueCancel(nullptr,
                                               i18n("Database contains data which must be removed before using Save As.\n"
                                                       "Do you wish to continue?"), "Database not empty") == KMessageBox::Continue) {
            if (writer->open(url, QIODevice::WriteOnly, true) == 0)
                canWrite = true;
        } else {
            delete writer;
            return false;
        }
        break;
    case 2: // bad password
    case 3: // unsupported operation
        delete writer;
        return false;
    }
    if (canWrite) {
        delete writer;
        save(url);
        return true;
    } else {
        KMessageBox::detailedError(nullptr,
                                   i18n("Cannot open or create database %1.\n"
                                        "Retry Save As Database and click Help"
                                        " for further info.", url.toDisplayString()), writer->lastError());
        delete writer;
        return false;
    }
}

K_PLUGIN_FACTORY_WITH_JSON(SQLStorageFactory, "sqlstorage.json", registerPlugin<SQLStorage>();)

#include "sqlstorage.moc"
