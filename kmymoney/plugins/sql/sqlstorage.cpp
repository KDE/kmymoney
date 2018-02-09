/***************************************************************************
                            sqlstorage.cpp
                             -------------------

    copyright            : (C) 2018 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sqlstorage.h"

#include <memory>
#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrlQuery>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "appinterface.h"
#include "viewinterface.h"
#include "kselectdatabasedlg.h"
#include "kgeneratesqldlg.h"
#include "mymoneyfile.h"
#include "mymoneystoragesql.h"
#include "mymoneyexception.h"
//#include "mymoneystoragemgr.h"
#include "icons.h"
#include "kmymoneyglobalsettings.h"

using namespace Icons;

SQLStorage::SQLStorage(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "sqlstorage"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args)
  setComponentName("sqlstorage", i18n("SQL storage"));
  setXMLFile("sqlstorage.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("Plugins: sqlstorage loaded");
}

SQLStorage::~SQLStorage()
{
  qDebug("Plugins: sqlstorage unloaded");
}

void SQLStorage::injectExternalSettings(KMyMoneySettings* p)
{
  KMyMoneyGlobalSettings::injectExternalSettings(p);
}

bool SQLStorage::open(MyMoneyStorageMgr *storage, const QUrl &url)
{
  auto reader = std::make_unique<MyMoneyStorageSql>(storage, url);

  QUrl dbURL(url);
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
        return false;
      case -1: // retryable error
        if (KMessageBox::warningYesNo(nullptr, reader->lastError(), PACKAGE) == KMessageBox::No) {
          return false;
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
    return false;
  }
//  reader->setProgressCallback(0);
  return true;
}

bool SQLStorage::save(const QUrl &url)
{
  return saveDatabase(url);
}

QString SQLStorage::formatName() const
{
  return QStringLiteral("SQL");
}

void SQLStorage::createActions()
{
  m_openDBaction = actionCollection()->addAction("open_database");
  m_openDBaction->setText(i18n("Open database..."));
  m_openDBaction->setIcon(Icons::get(Icon::SVNUpdate));
  connect(m_openDBaction, &QAction::triggered, this, &SQLStorage::slotOpenDatabase);

  m_saveAsDBaction = actionCollection()->addAction("saveas_database");
  m_saveAsDBaction->setText(i18n("Save as database..."));
  m_saveAsDBaction->setIcon(Icons::get(Icon::FileArchiver));
  connect(m_saveAsDBaction, &QAction::triggered, this, &SQLStorage::slotSaveAsDatabase);

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
      const QString key = QLatin1String("driver");
      // take care and convert some old url to their new counterpart
      QUrlQuery query(newurl);
      if (query.queryItemValue(key) == QLatin1String("QMYSQL3")) { // fix any old urls
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QMYSQL"));
      }
      if (query.queryItemValue(key) == QLatin1String("QSQLITE3")) {
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QSQLITE"));
      }
      newurl.setQuery(query);

      if (query.queryItemValue(key) == QLatin1String("QSQLITE")) {
        newurl.setUserInfo(QString());
        newurl.setHost(QString());
      }
      // check if a password is needed. it may be if the URL came from the last/recent file list
      QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite, newurl);
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

void SQLStorage::slotSaveAsDatabase()
{
  bool rc = false;
  QUrl oldUrl;
  // in event of it being a database, ensure that all data is read into storage for saveas
  if (viewInterface()->isDatabase())
    oldUrl = appInterface()->filenameURL().isEmpty() ? appInterface()->lastOpenedURL() : appInterface()->filenameURL();

  QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::WriteOnly);
  QUrl url = oldUrl;
  if (!dialog->checkDrivers()) {
    delete dialog;
    return;
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
        KMessageBox::sorry(nullptr, i18n("Cannot save to current database: %1", e.what()));
      }
    }
  }
  delete dialog;

  if (rc) {
    //KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
    //if(p)
    appInterface()->addToRecentFiles(url);
    appInterface()->writeLastUsedFile(url.toDisplayString(QUrl::PreferLocalFile));
  }
  appInterface()->autosaveTimer()->stop();
  appInterface()->updateCaption();
  return;
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
  }
  delete writer;
  if (canWrite) {
    saveDatabase(url);
    return true;
  } else {
    KMessageBox::detailedError(nullptr,
                               i18n("Cannot open or create database %1.\n"
                                    "Retry Save As Database and click Help"
                                    " for further info.", url.toDisplayString()), writer->lastError());
    return false;
  }
}

bool SQLStorage::saveDatabase(const QUrl &url)
{
  auto rc = false;
  if (!viewInterface()->fileOpen()) {
    KMessageBox::error(nullptr, i18n("Tried to access a file when it has not been opened"));
    return (rc);
  }
  auto writer = new MyMoneyStorageSql(MyMoneyFile::instance()->storage(), url);
  writer->open(url, QIODevice::WriteOnly);
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

K_PLUGIN_FACTORY_WITH_JSON(SQLStorageFactory, "sqlstorage.json", registerPlugin<SQLStorage>();)

#include "sqlstorage.moc"
