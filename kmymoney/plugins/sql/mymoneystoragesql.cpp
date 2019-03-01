/***************************************************************************
                          mymoneystoragesql.cpp
                          ---------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
                         : Christian Dávid <christian-david@web.de>
                         (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneystoragesql_p.h"

// ----------------------------------------------------------------------------
// System Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QInputDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include "KMessageBox"

// ----------------------------------------------------------------------------
// Project Includes

//************************ Constructor/Destructor *****************************
MyMoneyStorageSql::MyMoneyStorageSql(MyMoneyStorageMgr *storage, const QUrl &url) :
  QSqlDatabase(QUrlQuery(url).queryItemValue("driver")),
  d_ptr(new MyMoneyStorageSqlPrivate(this))
{
  Q_D(MyMoneyStorageSql);
  d->m_storage = storage;
}

MyMoneyStorageSql::~MyMoneyStorageSql()
{
  try {
    close(true);
  } catch (const MyMoneyException &e) {
    qDebug() << "Caught Exception in MMStorageSql dtor: " << e.what();
  }
  Q_D(MyMoneyStorageSql);
  delete d;
}

uint MyMoneyStorageSql::currentVersion() const
{
  Q_D(const MyMoneyStorageSql);
  return (d->m_db.currentVersion());
}

int MyMoneyStorageSql::open(const QUrl &url, int openMode, bool clear)
{
  Q_D(MyMoneyStorageSql);
  try {
    int rc = 0;
    d->m_driver = MyMoneyDbDriver::create(QUrlQuery(url).queryItemValue("driver"));
    //get the input options
    QStringList options = QUrlQuery(url).queryItemValue("options").split(',');
    d->m_loadAll = true; // force loading whole database into memory since unification of storages
                   // options.contains("loadAll")/*|| m_mode == 0*/;
    d->m_override = options.contains("override");

    // create the database connection
    // regarding the construction of the database name see the discussion on
    // https://phabricator.kde.org/D12681. In case of a local file based DB
    // driver we cut off the leading slash only in those cases, where we
    // a) have a file based DB on Windows systems and
    // b) have a server based DB.
    // so that we do not modify the absolute path on *nix based systems
    // in case of a DB based driver
    QString dbName = url.path();
    if(d->m_driver->requiresExternalFile()) {
#ifdef Q_OS_WIN
      dbName = url.path().remove(0, 1);   // remove separator slash for files on Windows
#endif
    } else {
      dbName = url.path().remove(0, 1);   // remove separator slash for server based databases
    }
    setDatabaseName(dbName);
    setHostName(url.host());
    setUserName(url.userName());
    setPassword(url.password());
    if (QUrlQuery(url).queryItemValue("driver").contains("QMYSQL")) {
      setConnectOptions("MYSQL_OPT_RECONNECT=1");
    }

    QSqlQuery query(*this);
    switch (openMode) {
      case QIODevice::ReadOnly:    // OpenDatabase menu entry (or open last file)
      case QIODevice::ReadWrite:   // Save menu entry with database open
        // this may be a sqlite file opened from the recently used list
        // but which no longer exists. In that case, open will work but create an empty file.
        // This is not what the user's after; he may accuse KMM of deleting all his data!
        if (d->m_driver->requiresExternalFile()) {
          if (!d->fileExists(dbName)) {
            rc = 1;
            break;
          }
        }
        if (!QSqlDatabase::open()) {
          d->buildError(QSqlQuery(*this), Q_FUNC_INFO, "opening database");
          rc = 1;
        } else {
          if (driverName().compare(QLatin1String("QSQLCIPHER")) == 0) {
            auto passphrase = password();
            while (true) {
              if (!passphrase.isEmpty()) {
                query.exec(QString::fromLatin1("PRAGMA cipher_version"));
                if(!query.next())
                  throw MYMONEYEXCEPTION_CSTRING("Based on empty cipher_version, libsqlcipher is not in use.");
                query.exec(QString::fromLatin1("PRAGMA key = '%1'").arg(passphrase)); // SQLCipher feature to decrypt a database
              }
              query.exec(QStringLiteral("SELECT count(*) FROM sqlite_master")); // SQLCipher recommended way to check if password is correct
              if (query.next()) {
                rc = d->createTables(); // check all tables are present, create if not
                break;
              }
              auto ok = false;
              passphrase = QInputDialog::getText(nullptr, i18n("Password"),
                                                 i18n("You're trying to open an encrypted database.\n"
                                                      "Please provide a password in order to open it."),
                                                 QLineEdit::Password, QString(), &ok);
              if (!ok) {
                QSqlDatabase::close();
                throw MYMONEYEXCEPTION_CSTRING("Bad password.");
              }
            }
          } else {
            rc = d->createTables(); // check all tables are present, create if not
          }
        }
        break;
      case QIODevice::WriteOnly:   // SaveAs Database - if exists, must be empty, if not will create
        {
          // Try to open the database.
          // If that fails, try to create the database, then try to open it again.
          d->m_newDatabase = true;

          // QSqlDatabase::open() always returns true on MS Windows
          // even if SQLite database doesn't exist
          auto isSQLiteAutocreated = false;
          if (driverName().compare(QLatin1String("QSQLITE")) == 0 ||
              driverName().compare(QLatin1String("QSQLCIPHER")) == 0) {
            if (!QFile::exists(dbName))
              isSQLiteAutocreated = true;
          }
          const auto isSuccessfullyOpened = QSqlDatabase::open();
          if (!isSuccessfullyOpened || (isSQLiteAutocreated && isSuccessfullyOpened)) {
            if (!d->createDatabase(url)) {
              rc = 1;
            } else {
              if (!QSqlDatabase::open()) {
                d->buildError(QSqlQuery(*this), Q_FUNC_INFO, "opening new database");
                rc = 1;
              } else {
                query.exec(QString::fromLatin1("PRAGMA key = '%1'").arg(password()));
                rc = d->createTables();
              }
            }
          } else {
            if (driverName().compare(QLatin1String("QSQLCIPHER")) == 0 &&
                !password().isEmpty()) {
              KMessageBox::information(nullptr, i18n("Overwriting an existing database with an encrypted database is not yet supported.\n"
                                                     "Please save your database under a new name."));
              QSqlDatabase::close();
              rc = 3;
              return rc;
            }
            rc = d->createTables();
            if (rc == 0) {
              if (clear) {
                d->clean();
              } else {
                rc = d->isEmpty();
              }
            }
          }
          break;
        }
      default:
        qWarning("%s", qPrintable(QString("%1 - unknown open mode %2").arg(Q_FUNC_INFO).arg(openMode)));
    }
    if (rc != 0) return (rc);
    // bypass logon check if we are creating a database
    if (d->m_newDatabase) return(0);
    // check if the database is locked, if not lock it
    d->readFileInfo();
    if (!d->m_logonUser.isEmpty() && (!d->m_override)) {
      d->m_error = i18n("Database apparently in use\nOpened by %1 on %2 at %3.\nOpen anyway?",
                     d->m_logonUser,
                     d->m_logonAt.date().toString(Qt::ISODate),
                     d->m_logonAt.time().toString("hh.mm.ss"));
      qDebug("%s", qPrintable(d->m_error));
      close(false);
      rc = -1; // retryable error
    } else {
      d->m_logonUser = url.userName() + '@' + url.host();
      d->m_logonAt = QDateTime::currentDateTime();
      d->writeFileInfo();
    }
    return(rc);
  } catch (const QString& s) {
    qDebug("%s", qPrintable(s));
    return (1);
  }
}

void MyMoneyStorageSql::close(bool logoff)
{
  Q_D(MyMoneyStorageSql);
  if (QSqlDatabase::isOpen()) {
    if (logoff) {
      MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
      d->m_logonUser.clear();
      d->writeFileInfo();
    }
    QSqlDatabase::close();
    QSqlDatabase::removeDatabase(connectionName());
  }
}

ulong MyMoneyStorageSql::getRecCount(const QString& table) const
{
  Q_D(const MyMoneyStorageSql);
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare(QString("SELECT COUNT(*) FROM %1;").arg(table));
  if ((!q.exec()) || (!q.next())) { // krazy:exclude=crashy
    d->buildError(q, Q_FUNC_INFO, "error retrieving record count");
    qFatal("Error retrieving record count"); // definitely shouldn't happen
  }
  return ((ulong) q.value(0).toULongLong());
}

//////////////////////////////////////////////////////////////////

bool MyMoneyStorageSql::readFile()
{
  Q_D(MyMoneyStorageSql);
  d->m_displayStatus = true;
  try {
    d->readFileInfo();
    d->readInstitutions();
    if (d->m_loadAll) {
      readPayees();
    } else {
      QList<QString> user;
      user.append(QString("USER"));
      readPayees(user);
    }
    readTags();
    d->readCurrencies();
    d->readSecurities();
    d->readAccounts();
    if (d->m_loadAll) {
      d->readTransactions();
    } else {
      if (d->m_preferred.filterSet().singleFilter.accountFilter) readTransactions(d->m_preferred);
    }
    d->readSchedules();
    d->readPrices();
    d->readReports();
    d->readBudgets();
    d->readOnlineJobs();
    //FIXME - ?? if (m_mode == 0)
    //m_storage->rebuildAccountBalances();
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    d->m_storage->setLastModificationDate(d->m_storage->lastModificationDate());
    // FIXME?? if (m_mode == 0) m_storage = NULL;
    // make sure the progress bar is not shown any longer
    d->signalProgress(-1, -1);
    d->m_displayStatus = false;
    //MyMoneySqlQuery::traceOn();
    return true;
  } catch (const QString &) {
    return false;
  }
  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
}

// The following is called from 'SaveAsDatabase'
bool MyMoneyStorageSql::writeFile()
{
  Q_D(MyMoneyStorageSql);
  // initialize record counts and hi ids
  d->m_institutions = d->m_accounts = d->m_payees = d->m_tags = d->m_transactions = d->m_splits
                                = d->m_securities = d->m_prices = d->m_currencies = d->m_schedules  = d->m_reports = d->m_kvps = d->m_budgets = 0;
  d->m_hiIdInstitutions = d->m_hiIdPayees = d->m_hiIdTags = d->m_hiIdAccounts = d->m_hiIdTransactions =
                                        d->m_hiIdSchedules = d->m_hiIdSecurities = d->m_hiIdReports = d->m_hiIdBudgets = 0;
  d->m_onlineJobs = d->m_payeeIdentifier = 0;
  d->m_displayStatus = true;
  try {
    const auto driverName = this->driverName();
    if (driverName.compare(QLatin1String("QSQLITE")) == 0 ||
        driverName.compare(QLatin1String("QSQLCIPHER")) == 0) {
      QSqlQuery query(*this);
      query.exec("PRAGMA foreign_keys = ON"); // this is needed for "ON UPDATE" and "ON DELETE" to work
    }

    MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
    d->writeInstitutions();
    d->writePayees();
    d->writeTags();
    d->writeAccounts();
    d->writeTransactions();
    d->writeSchedules();
    d->writeSecurities();
    d->writePrices();
    d->writeCurrencies();
    d->writeReports();
    d->writeBudgets();
    d->writeOnlineJobs();
    d->writeFileInfo();
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    //m_storage->setLastModificationDate(m_storage->lastModificationDate());
    // FIXME?? if (m_mode == 0) m_storage = NULL;

    // make sure the progress bar is not shown any longer
    d->signalProgress(-1, -1);
    d->m_displayStatus = false;
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    d->m_storage->setLastModificationDate(d->m_storage->lastModificationDate());
    return true;
  } catch (const QString &) {
    return false;
  }
}

QString MyMoneyStorageSql::lastError() const
{
  Q_D(const MyMoneyStorageSql);
  return d->m_error;
}

// --------------- SQL Transaction (commit unit) handling -----------------------------------
void MyMoneyStorageSql::startCommitUnit(const QString& callingFunction)
{
  Q_D(MyMoneyStorageSql);
  if (d->m_commitUnitStack.isEmpty()) {
    if (!transaction()) throw MYMONEYEXCEPTION(d->buildError(QSqlQuery(), callingFunction, "starting commit unit"));
  }
  d->m_commitUnitStack.push(callingFunction);
}

bool MyMoneyStorageSql::endCommitUnit(const QString& callingFunction)
{
  Q_D(MyMoneyStorageSql);
  // for now, we don't know if there were any changes made to the data so
  // we expect the data to have changed. This assumption causes some unnecessary
  // repaints of the UI here and there, but for now it's ok. If we can determine
  // that the commit() really changes the data, we can return that information
  // as value of this method.
  bool rc = true;
  if (d->m_commitUnitStack.isEmpty()) {
    throw MYMONEYEXCEPTION_CSTRING("Empty commit unit stack while trying to commit");
  }

  if (callingFunction != d->m_commitUnitStack.top())
    qDebug("%s", qPrintable(QString("%1 - %2 s/be %3").arg(Q_FUNC_INFO).arg(callingFunction).arg(d->m_commitUnitStack.top())));
  d->m_commitUnitStack.pop();
  if (d->m_commitUnitStack.isEmpty()) {
    //qDebug() << "Committing with " << QSqlQuery::refCount() << " queries";
    if (!commit()) throw MYMONEYEXCEPTION(d->buildError(QSqlQuery(), callingFunction, "ending commit unit"));
  }
  return rc;
}

void MyMoneyStorageSql::cancelCommitUnit(const QString& callingFunction)
{
  Q_D(MyMoneyStorageSql);
  if (d->m_commitUnitStack.isEmpty()) return;
  if (callingFunction != d->m_commitUnitStack.top())
    qDebug("%s", qPrintable(QString("%1 - %2 s/be %3").arg(Q_FUNC_INFO).arg(callingFunction).arg(d->m_commitUnitStack.top())));
  d->m_commitUnitStack.clear();
  if (!rollback()) throw MYMONEYEXCEPTION(d->buildError(QSqlQuery(), callingFunction, "cancelling commit unit") + ' ' + callingFunction);
}

/////////////////////////////////////////////////////////////////////
void MyMoneyStorageSql::fillStorage()
{
  Q_D(MyMoneyStorageSql);
//  if (!m_transactionListRead)  // make sure we have loaded everything
  d->readTransactions();
//  if (!m_payeeListRead)
  readPayees();
}

//------------------------------ Write SQL routines ----------------------------------------
// **** Institutions ****


void MyMoneyStorageSql::addInstitution(const MyMoneyInstitution& inst)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmInstitutions"].insertString());
  QList<MyMoneyInstitution> iList;
  iList << inst;
  d->writeInstitutionList(iList , q);
  ++d->m_institutions;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyInstitution(const MyMoneyInstitution& inst)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmInstitutions"].updateString());
  QVariantList kvpList;
  kvpList << inst.id();
  d->deleteKeyValuePairs("OFXSETTINGS", kvpList);
  QList<MyMoneyInstitution> iList;
  iList << inst;
  d->writeInstitutionList(iList , q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeInstitution(const MyMoneyInstitution& inst)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << inst.id();
  d->deleteKeyValuePairs("OFXSETTINGS", kvpList);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmInstitutions"].deleteString());
  query.bindValue(":id", inst.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting  Institution")); // krazy:exclude=crashy
  --d->m_institutions;
  d->writeFileInfo();
}

void MyMoneyStorageSql::addPayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmPayees"].insertString());
  d->writePayee(payee, query);
  ++d->m_payees;

  QVariantList identIds;
  QList<payeeIdentifier> idents = payee.payeeIdentifiers();
  // Store ids which have to be stored in the map table
  identIds.reserve(idents.count());
  foreach (payeeIdentifier ident, idents) {
      try {
        // note: this changes ident
        addPayeeIdentifier(ident);
      identIds.append(ident.idString());
    } catch (const payeeIdentifier::empty &) {
      }
  }

  if (!identIds.isEmpty()) {
    // Create lists for batch processing
    QVariantList order;
    QVariantList payeeIdList;
    order.reserve(identIds.size());
    payeeIdList.reserve(identIds.size());

    for (int i = 0; i < identIds.size(); ++i) {
      order << i;
      payeeIdList << payee.id();
    }
    query.prepare("INSERT INTO kmmPayeesPayeeIdentifier (payeeId, identifierId, userOrder) VALUES(?, ?, ?)");
    query.bindValue(0, payeeIdList);
    query.bindValue(1, identIds);
    query.bindValue(2, order);
    if (!query.execBatch())
      throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("writing payee's identifiers")); // krazy:exclude=crashy
  }

  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyPayee(MyMoneyPayee payee)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmPayees"].updateString());
  d->writePayee(payee, query);

  // Get a list of old identifiers first
  query.prepare("SELECT identifierId FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  query.bindValue(0, payee.id());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("modifying payee's identifiers (getting old values failed)")); // krazy:exclude=crashy

  QStringList oldIdentIds;
  oldIdentIds.reserve(query.numRowsAffected());
  while (query.next())
    oldIdentIds << query.value(0).toString();

  // Add new and modify old payeeIdentifiers
  foreach (payeeIdentifier ident, payee.payeeIdentifiers()) {
    if (ident.idString().isEmpty()) {
      payeeIdentifier oldIdent(ident);
      addPayeeIdentifier(ident);
      // addPayeeIdentifier could fail (throws an exception then) only remove old
      // identifier if new one is stored correctly
      payee.removePayeeIdentifier(oldIdent);
      payee.addPayeeIdentifier(ident);
    } else {
      modifyPayeeIdentifier(ident);
      payee.modifyPayeeIdentifier(ident);
      oldIdentIds.removeAll(ident.idString());
    }
  }

  // Remove identifiers which are not used anymore
  foreach (QString idToRemove, oldIdentIds) {
    payeeIdentifier ident(fetchPayeeIdentifier(idToRemove));
    removePayeeIdentifier(ident);
  }

  // Update relation table
  query.prepare("DELETE FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  query.bindValue(0, payee.id());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("modifying payee's identifiers (delete from mapping table)")); // krazy:exclude=crashy

  // Get list again because modifiyPayeeIdentifier which is used above may change the id
  QList<payeeIdentifier> idents(payee.payeeIdentifiers());

  QVariantList order;
  QVariantList payeeIdList;
  QVariantList identIdList;
  order.reserve(idents.size());
  payeeIdList.reserve(idents.size());
  identIdList.reserve(idents.size());

  {
    QList<payeeIdentifier>::const_iterator end = idents.constEnd();
    int i = 0;
    for (QList<payeeIdentifier>::const_iterator iter = idents.constBegin(); iter != end; ++iter, ++i) {
      order << i;
      payeeIdList << payee.id();
      identIdList << iter->idString();
    }
  }

  query.prepare("INSERT INTO kmmPayeesPayeeIdentifier (payeeId, userOrder, identifierId) VALUES(?, ?, ?)");
  query.bindValue(0, payeeIdList);
  query.bindValue(1, order);
  query.bindValue(2, identIdList);
  if (!query.execBatch())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("writing payee's identifiers during modify")); // krazy:exclude=crashy

  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyUserInfo(const MyMoneyPayee& payee)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmPayees"].updateString());
  d->writePayee(payee, q, true);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removePayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);

  // Get identifiers first so we know which to delete
  query.prepare("SELECT identifierId FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  query.bindValue(0, payee.id());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("removing payee's identifiers (getting old values failed)")); // krazy:exclude=crashy

  QStringList identIds;
  while (query.next())
    identIds << query.value(0).toString();

  QMap<QString, payeeIdentifier> idents = fetchPayeeIdentifiers(identIds);
  foreach (payeeIdentifier ident, idents) {
    removePayeeIdentifier(ident);
  }

  // Delete entries from mapping table
  query.prepare("DELETE FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  query.bindValue(0, payee.id());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("removing payee's identifiers (delete from mapping table)")); // krazy:exclude=crashy

  // Delete payee
  query.prepare(d->m_db.m_tables["kmmPayees"].deleteString());
  query.bindValue(":id", payee.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting  Payee")); // krazy:exclude=crashy
  --d->m_payees;

  d->writeFileInfo();
}

// **** Tags ****
void MyMoneyStorageSql::addTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmTags"].insertString());
  d->writeTag(tag, q);
  ++d->m_tags;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmTags"].updateString());
  d->writeTag(tag, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmTags"].deleteString());
  query.bindValue(":id", tag.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting  Tag")); // krazy:exclude=crashy
  --d->m_tags;
  d->writeFileInfo();
}

// **** Accounts ****
void MyMoneyStorageSql::addAccount(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmAccounts"].insertString());
  QList<MyMoneyAccount> aList;
  aList << acc;
  d->writeAccountList(aList, q);
  ++d->m_accounts;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyAccount(const MyMoneyAccount& acc)
{
  QList<MyMoneyAccount> aList;
  aList << acc;
  modifyAccountList(aList);
}

void MyMoneyStorageSql::modifyAccountList(const QList<MyMoneyAccount>& acc)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmAccounts"].updateString());
  QVariantList kvpList;
  foreach (const MyMoneyAccount& a, acc) {
    kvpList << a.id();
  }
  d->deleteKeyValuePairs("ACCOUNT", kvpList);
  d->deleteKeyValuePairs("ONLINEBANKING", kvpList);
  d->writeAccountList(acc, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeAccount(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << acc.id();
  d->deleteKeyValuePairs("ACCOUNT", kvpList);
  d->deleteKeyValuePairs("ONLINEBANKING", kvpList);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmAccounts"].deleteString());
  query.bindValue(":id", acc.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Account")); // krazy:exclude=crashy
  --d->m_accounts;
  d->writeFileInfo();
}


// **** Transactions and Splits ****
void MyMoneyStorageSql::addTransaction(const MyMoneyTransaction& tx)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  // add the transaction and splits
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmTransactions"].insertString());
  d->writeTransaction(tx.id(), tx, q, "N");
  ++d->m_transactions;
  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = d->m_storage->account(it_s.accountId());
    ++d->m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  // in the fileinfo record, update lastMod, txCount, next TxId
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyTransaction(const MyMoneyTransaction& tx)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  // remove the splits of the old tx from the count table
  QSqlQuery query(*this);
  query.prepare("SELECT accountId FROM kmmSplits WHERE transactionId = :txId;");
  query.bindValue(":txId", tx.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("retrieving old splits"));
  while (query.next()) {
    QString id = query.value(0).toString();
    --d->m_transactionCountMap[id];
  }
  // add the transaction and splits
  query.prepare(d->m_db.m_tables["kmmTransactions"].updateString());
  d->writeTransaction(tx.id(), tx, query, "N");
  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = d->m_storage->account(it_s.accountId());
    ++d->m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  //writeSplits(tx.id(), "N", tx.splits());
  // in the fileinfo record, update lastMod
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeTransaction(const MyMoneyTransaction& tx)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  d->deleteTransaction(tx.id());
  --d->m_transactions;

  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = d->m_storage->account(it_s.accountId());
    --d->m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  // in the fileinfo record, update lastModDate, txCount
  d->writeFileInfo();
}

// **** Schedules ****

void MyMoneyStorageSql::addSchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmSchedules"].insertString());
  d->writeSchedule(sched, q, true);
  ++d->m_schedules;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifySchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmSchedules"].updateString());
  d->writeSchedule(sched, q, false);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeSchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  d->deleteSchedule(sched.id());
  --d->m_schedules;
  d->writeFileInfo();
}

// **** Securities ****
void MyMoneyStorageSql::addSecurity(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmSecurities"].insertString());
  d->writeSecurity(sec, q);
  ++d->m_securities;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifySecurity(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << sec.id();
  d->deleteKeyValuePairs("SECURITY", kvpList);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmSecurities"].updateString());
  d->writeSecurity(sec, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeSecurity(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << sec.id();
  d->deleteKeyValuePairs("SECURITY", kvpList);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmSecurities"].deleteString());
  query.bindValue(":id", kvpList);
  if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Security"));
  --d->m_securities;
  d->writeFileInfo();
}

// **** Prices ****
void MyMoneyStorageSql::addPrice(const MyMoneyPrice& p)
{
  Q_D(MyMoneyStorageSql);
  if (d->m_readingPrices) return;
  // the app always calls addPrice, whether or not there is already one there
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  bool newRecord = false;
  QSqlQuery query(*this);
  QString s = d->m_db.m_tables["kmmPrices"].selectAllString(false);
  s += " WHERE fromId = :fromId AND toId = :toId AND priceDate = :priceDate;";
  query.prepare(s);
  query.bindValue(":fromId", p.from());
  query.bindValue(":toId", p.to());
  query.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("finding Price")); // krazy:exclude=crashy
  if (query.next()) {
    query.prepare(d->m_db.m_tables["kmmPrices"].updateString());
  } else {
    query.prepare(d->m_db.m_tables["kmmPrices"].insertString());
    ++d->m_prices;
    newRecord = true;
  }
  query.bindValue(":fromId", p.from());
  query.bindValue(":toId", p.to());
  query.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  query.bindValue(":price", p.rate(QString()).toString());
  const MyMoneySecurity sec = d->m_storage->security(p.to());
  query.bindValue(":priceFormatted",
              p.rate(QString()).formatMoney("", sec.pricePrecision()));
  query.bindValue(":priceSource", p.source());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("writing Price")); // krazy:exclude=crashy

  if (newRecord) d->writeFileInfo();
}

void MyMoneyStorageSql::removePrice(const MyMoneyPrice& p)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmPrices"].deleteString());
  query.bindValue(":fromId", p.from());
  query.bindValue(":toId", p.to());
  query.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Price")); // krazy:exclude=crashy
  --d->m_prices;
  d->writeFileInfo();
}

// **** Currencies ****
void MyMoneyStorageSql::addCurrency(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmCurrencies"].insertString());
  d->writeCurrency(sec, q);
  ++d->m_currencies;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyCurrency(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmCurrencies"].updateString());
  d->writeCurrency(sec, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeCurrency(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmCurrencies"].deleteString());
  query.bindValue(":ISOcode", sec.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Currency")); // krazy:exclude=crashy
  --d->m_currencies;
  d->writeFileInfo();
}

void MyMoneyStorageSql::addReport(const MyMoneyReport& rep)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmReportConfig"].insertString());
  d->writeReport(rep, q);
  ++d->m_reports;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyReport(const MyMoneyReport& rep)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmReportConfig"].updateString());
  d->writeReport(rep, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeReport(const MyMoneyReport& rep)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare("DELETE FROM kmmReportConfig WHERE id = :id");
  query.bindValue(":id", rep.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Report")); // krazy:exclude=crashy
  --d->m_reports;
  d->writeFileInfo();
}

void MyMoneyStorageSql::addBudget(const MyMoneyBudget& bud)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmBudgetConfig"].insertString());
  d->writeBudget(bud, q);
  ++d->m_budgets;
  d->writeFileInfo();
}

void MyMoneyStorageSql::modifyBudget(const MyMoneyBudget& bud)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(d->m_db.m_tables["kmmBudgetConfig"].updateString());
  d->writeBudget(bud, q);
  d->writeFileInfo();
}

void MyMoneyStorageSql::removeBudget(const MyMoneyBudget& bud)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmBudgetConfig"].deleteString());
  query.bindValue(":id", bud.id());
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting Budget")); // krazy:exclude=crashy
  --d->m_budgets;
  d->writeFileInfo();
}

void MyMoneyStorageSql::addOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare("INSERT INTO kmmOnlineJobs (id, type, jobSend, bankAnswerDate, state, locked) VALUES(:id, :type, :jobSend, :bankAnswerDate, :state, :locked);");
  d->writeOnlineJob(job, query);
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("writing onlineJob")); // krazy:exclude=crashy
  ++d->m_onlineJobs;

  try {
    // Save online task
    d->actOnOnlineJobInSQL(MyMoneyStorageSqlPrivate::SQLAction::Save, *job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
  }
}

void MyMoneyStorageSql::modifyOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneyStorageSql);
  Q_ASSERT(!job.id().isEmpty());

  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery query(*this);
  query.prepare(QLatin1String(
    "UPDATE kmmOnlineJobs SET "
    " type = :type, "
    " jobSend = :jobSend, "
    " bankAnswerDate = :bankAnswerDate, "
    " state = :state, "
    " locked = :locked "
    " WHERE id = :id"
  ));

  d->writeOnlineJob(job, query);
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("writing onlineJob")); // krazy:exclude=crashy

  try {
    // Modify online task
    d->actOnOnlineJobInSQL(MyMoneyStorageSqlPrivate::SQLAction::Modify, *job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
    // If there is no task attached this is fine as well
  }
}

void MyMoneyStorageSql::removeOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  // Remove onlineTask first, because it could have a constraint
  // which could block the removal of the onlineJob

  try {
    // Remove task
    d->actOnOnlineJobInSQL(MyMoneyStorageSqlPrivate::SQLAction::Remove, *job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
  }

  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmOnlineJobs"].deleteString());
  query.bindValue(":id", job.id());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting onlineJob")); // krazy:exclude=crashy
  --d->m_onlineJobs;
}

void MyMoneyStorageSql::addPayeeIdentifier(payeeIdentifier& ident)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  ident = payeeIdentifier(incrementPayeeIdentfierId(), ident);

  QSqlQuery q(*this);
  q.prepare("INSERT INTO kmmPayeeIdentifier (id, type) VALUES(:id, :type)");
  d->writePayeeIdentifier(ident, q);
  ++d->m_payeeIdentifier;

  try {
    d->actOnPayeeIdentifierObjectInSQL(MyMoneyStorageSqlPrivate::SQLAction::Save, ident);
  } catch (const payeeIdentifier::empty &) {
  }
}

void MyMoneyStorageSql::modifyPayeeIdentifier(const payeeIdentifier& ident)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  QSqlQuery query(*this);
  query.prepare("SELECT type FROM kmmPayeeIdentifier WHERE id = ?");
  query.bindValue(0, ident.idString());
  if (!query.exec() || !query.next())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("modifying payeeIdentifier")); // krazy:exclude=crashy

  bool typeChanged = (query.value(0).toString() != ident.iid());

  if (typeChanged) {
    // Delete old identifier if type changed
    const payeeIdentifier oldIdent(fetchPayeeIdentifier(ident.idString()));
    try {
      d->actOnPayeeIdentifierObjectInSQL(MyMoneyStorageSqlPrivate::SQLAction::Modify, oldIdent);
    } catch (const payeeIdentifier::empty &) {
      // Note: this should not happen because the ui does not offer a way to change
      // the type of an payeeIdentifier if it was not correctly loaded.
      throw MYMONEYEXCEPTION((QString::fromLatin1("Could not modify payeeIdentifier '")
        + ident.idString()
        + QLatin1String("' because type changed and could not remove identifier of old type. Maybe a plugin is missing?"))
      ); // krazy:exclude=crashy
    }
  }

  query.prepare("UPDATE kmmPayeeIdentifier SET type = :type WHERE id = :id");
  d->writePayeeIdentifier(ident, query);

  try {
    if (typeChanged)
      d->actOnPayeeIdentifierObjectInSQL(MyMoneyStorageSqlPrivate::SQLAction::Save, ident);
    else
      d->actOnPayeeIdentifierObjectInSQL(MyMoneyStorageSqlPrivate::SQLAction::Modify, ident);
  } catch (const payeeIdentifier::empty &) {
  }
}

void MyMoneyStorageSql::removePayeeIdentifier(const payeeIdentifier& ident)
{
  Q_D(MyMoneyStorageSql);
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  // Remove first, the table could have a contraint which prevents removal
  // of row in kmmPayeeIdentifier
  try {
    d->actOnPayeeIdentifierObjectInSQL(MyMoneyStorageSqlPrivate::SQLAction::Remove, ident);
  } catch (const payeeIdentifier::empty &) {
  }

  QSqlQuery query(*this);
  query.prepare(d->m_db.m_tables["kmmPayeeIdentifier"].deleteString());
  query.bindValue(":id", ident.idString());
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("deleting payeeIdentifier")); // krazy:exclude=crashy
  --d->m_payeeIdentifier;
}

// **** Key/value pairs ****

//******************************** read SQL routines **************************************

/*void MyMoneyStorageSql::setVersion (const QString& version)
{
  m_dbVersion = version.section('.', 0, 0).toUInt();
  m_minorVersion = version.section('.', 1, 1).toUInt();
  // Okay, I made a cockup by forgetting to include a fixversion in the database
  // design, so we'll use the minor version as fix level (similar to VERSION
  // and FIXVERSION in XML file format). A second mistake was setting minor version to 1
  // in the first place, so we need to subtract one on reading and add one on writing (sigh)!!
  m_storage->setFileFixVersion( m_minorVersion - 1);
}*/


QMap<QString, MyMoneyInstitution> MyMoneyStorageSql::fetchInstitutions(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int institutionsNb = (idList.isEmpty() ? d->m_institutions : idList.size());
  d->signalProgress(0, institutionsNb, QObject::tr("Loading institutions..."));
  int progress = 0;
  QMap<QString, MyMoneyInstitution> iList;
  ulong lastId = 0;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmInstitutions"];
  QSqlQuery sq(*const_cast <MyMoneyStorageSql*>(this));
  sq.prepare("SELECT id from kmmAccounts where institutionId = :id");
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString(t.selectAllString(false));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  if (! idList.empty()) {
    queryString += " WHERE";
    for (int i = 0; i < idList.count(); ++i)
      queryString += QString(" id = :id%1 OR").arg(i);
    queryString = queryString.left(queryString.length() - 2);
  }
  if (forUpdate)
    queryString += d->m_driver->forUpdateString();

  queryString += ';';

  query.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      query.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!query.exec()) throw MYMONEYEXCEPTION(d->buildError(query, Q_FUNC_INFO, QString::fromLatin1("reading Institution"))); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int managerCol = t.fieldNumber("manager");
  int routingCodeCol = t.fieldNumber("routingCode");
  int addressStreetCol = t.fieldNumber("addressStreet");
  int addressCityCol = t.fieldNumber("addressCity");
  int addressZipcodeCol = t.fieldNumber("addressZipcode");
  int telephoneCol = t.fieldNumber("telephone");

  while (query.next()) {
    MyMoneyInstitution inst;
    QString iid = GETSTRING(idCol);
    inst.setName(GETSTRING(nameCol));
    inst.setManager(GETSTRING(managerCol));
    inst.setSortcode(GETSTRING(routingCodeCol));
    inst.setStreet(GETSTRING(addressStreetCol));
    inst.setCity(GETSTRING(addressCityCol));
    inst.setPostcode(GETSTRING(addressZipcodeCol));
    inst.setTelephone(GETSTRING(telephoneCol));
    // get list of subaccounts
    sq.bindValue(":id", iid);
    if (!sq.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Institution AccountList")); // krazy:exclude=crashy
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    foreach (const QString& it, aList)
    inst.addAccountId(it);

    iList[iid] = MyMoneyInstitution(iid, inst);
    ulong id = MyMoneyUtils::extractId(iid);
    if (id > lastId)
      lastId = id;

    d->signalProgress(++progress, 0);
  }
  return iList;
}

QMap<QString, MyMoneyInstitution> MyMoneyStorageSql::fetchInstitutions() const
{
  return fetchInstitutions(QStringList(), false);
}

void MyMoneyStorageSql::readPayees(const QString& id)
{
  QList<QString> list;
  list.append(id);
  readPayees(list);
}

void MyMoneyStorageSql::readPayees(const QList<QString>& pid)
{
  Q_D(MyMoneyStorageSql);
  try {
    d->m_storage->loadPayees(fetchPayees(pid));
  } catch (const MyMoneyException &) {
  }
//  if (pid.isEmpty()) m_payeeListRead = true;
}

void MyMoneyStorageSql::readPayees()
{
  readPayees(QList<QString>());
}

QMap<QString, MyMoneyPayee> MyMoneyStorageSql::fetchPayees(const QStringList& idList, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (d->m_displayStatus) {
    int payeesNb = (idList.isEmpty() ? d->m_payees : idList.size());
    d->signalProgress(0, payeesNb, QObject::tr("Loading payees..."));
  }

  int progress = 0;
  QMap<QString, MyMoneyPayee> pList;

  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString = QLatin1String("SELECT kmmPayees.id AS id, kmmPayees.name AS name, kmmPayees.reference AS reference, "
    " kmmPayees.email AS email, kmmPayees.addressStreet AS addressStreet, kmmPayees.addressCity AS addressCity, kmmPayees.addressZipcode AS addressZipcode, "
    " kmmPayees.addressState AS addressState, kmmPayees.telephone AS  telephone, kmmPayees.notes AS notes, "
    " kmmPayees.defaultAccountId AS defaultAccountId, kmmPayees.matchData AS matchData, kmmPayees.matchIgnoreCase AS matchIgnoreCase, "
    " kmmPayees.matchKeys AS matchKeys, "
    " kmmPayeesPayeeIdentifier.identifierId AS identId "
    " FROM ( SELECT * FROM kmmPayees ");

  if (!idList.isEmpty()) {
    // Create WHERE clause if needed
    queryString += QLatin1String(" WHERE id IN (");
    queryString += QString("?, ").repeated(idList.length());
    queryString.chop(2);   // remove ", " from end
    queryString += QLatin1Char(')');
  }

  queryString += QLatin1String(
    " ) kmmPayees "
    " LEFT OUTER JOIN kmmPayeesPayeeIdentifier ON kmmPayees.Id = kmmPayeesPayeeIdentifier.payeeId "
    // The order is used below
    " ORDER BY kmmPayees.id, kmmPayeesPayeeIdentifier.userOrder;");

  query.prepare(queryString);

  if (!idList.isEmpty()) {
    // Bind values
    QStringList::const_iterator end = idList.constEnd();
    for (QStringList::const_iterator iter = idList.constBegin(); iter != end; ++iter) {
      query.addBindValue(*iter);
    }
  }

  if (!query.exec())
    throw MYMONEYEXCEPTION(d->buildError(query, Q_FUNC_INFO, QString::fromLatin1("reading Payee"))); // krazy:exclude=crashy
  const QSqlRecord record = query.record();
  const int idCol = record.indexOf("id");
  const int nameCol = record.indexOf("name");
  const int referenceCol = record.indexOf("reference");
  const int emailCol = record.indexOf("email");
  const int addressStreetCol = record.indexOf("addressStreet");
  const int addressCityCol = record.indexOf("addressCity");
  const int addressZipcodeCol = record.indexOf("addressZipcode");
  const int addressStateCol = record.indexOf("addressState");
  const int telephoneCol = record.indexOf("telephone");
  const int notesCol = record.indexOf("notes");
  const int defaultAccountIdCol = record.indexOf("defaultAccountId");
  const int matchDataCol = record.indexOf("matchData");
  const int matchIgnoreCaseCol = record.indexOf("matchIgnoreCase");
  const int matchKeysCol = record.indexOf("matchKeys");
  const int identIdCol = record.indexOf("identId");

  if (query.next()) {
    while (query.isValid()) {
      QString pid;
      MyMoneyPayee payee;
      uint type;
      bool ignoreCase;
      QString matchKeys;
      pid = GETSTRING(idCol);
      payee.setName(GETSTRING(nameCol));
      payee.setReference(GETSTRING(referenceCol));
      payee.setEmail(GETSTRING(emailCol));
      payee.setAddress(GETSTRING(addressStreetCol));
      payee.setCity(GETSTRING(addressCityCol));
      payee.setPostcode(GETSTRING(addressZipcodeCol));
      payee.setState(GETSTRING(addressStateCol));
      payee.setTelephone(GETSTRING(telephoneCol));
      payee.setNotes(GETSTRING(notesCol));
      payee.setDefaultAccountId(GETSTRING(defaultAccountIdCol));
      type = GETINT(matchDataCol);
      ignoreCase = (GETSTRING(matchIgnoreCaseCol) == "Y");
      matchKeys = GETSTRING(matchKeysCol);

      payee.setMatchData(static_cast<eMyMoney::Payee::MatchType>(type), ignoreCase, matchKeys);

      // Get payeeIdentifier ids
      QStringList identifierIds;
      do {
        identifierIds.append(GETSTRING(identIdCol));
      } while (query.next() && GETSTRING(idCol) == pid);   // as long as the payeeId is unchanged

      // Fetch and save payeeIdentifier
      if (!identifierIds.isEmpty()) {
        QList< ::payeeIdentifier > identifier = fetchPayeeIdentifiers(identifierIds).values();
        payee.resetPayeeIdentifiers(identifier);
      }

      if (pid == "USER")
        d->m_storage->setUser(payee);
      else
        pList[pid] = MyMoneyPayee(pid, payee);

      if (d->m_displayStatus)
        d->signalProgress(++progress, 0);
    }
  }
  return pList;
}

QMap<QString, MyMoneyPayee> MyMoneyStorageSql::fetchPayees() const
{
  return fetchPayees(QStringList(), false);
}

void MyMoneyStorageSql::readTags(const QString& id)
{
  QList<QString> list;
  list.append(id);
  readTags(list);
}

void MyMoneyStorageSql::readTags(const QList<QString>& pid)
{
  Q_D(MyMoneyStorageSql);
  try {
    d->m_storage->loadTags(fetchTags(pid));
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyStorageSql::readTags()
{
  readTags(QList<QString>());
}

QMap<QString, onlineJob> MyMoneyStorageSql::fetchOnlineJobs(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  Q_UNUSED(forUpdate);
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (d->m_displayStatus)
    d->signalProgress(0, idList.isEmpty() ? d->m_onlineJobs : idList.size(), QObject::tr("Loading online banking data..."));

  // Create query
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    query.prepare("SELECT id, type, jobSend, bankAnswerDate, state, locked FROM kmmOnlineJobs;");
  } else {
    QString queryIdSet = QString("?, ").repeated(idList.length());
    queryIdSet.chop(2);
    query.prepare(QLatin1String("SELECT id, type, jobSend, bankAnswerDate, state, locked FROM kmmOnlineJobs WHERE id IN (") + queryIdSet + QLatin1String(");"));

    QStringList::const_iterator end = idList.constEnd();
    for (QStringList::const_iterator iter = idList.constBegin(); iter != end; ++iter) {
      query.addBindValue(*iter);
    }
  }
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading onlineJobs")); // krazy:exclude=crashy

  // Create onlineJobs
  int progress = 0;
  QMap<QString, onlineJob> jobList;

  while (query.next()) {
    const QString& id = query.value(0).toString();
    onlineTask *const task = d->createOnlineTaskObject(query.value(1).toString(), id, *this);
    onlineJob job = onlineJob(task, id);
    job.setJobSend(query.value(2).toDateTime());
    eMyMoney::OnlineJob::sendingState state;
    const QString stateString = query.value(4).toString();
    if (stateString == "acceptedByBank")
      state = eMyMoney::OnlineJob::sendingState::acceptedByBank;
    else if (stateString == "rejectedByBank")
      state = eMyMoney::OnlineJob::sendingState::rejectedByBank;
    else if (stateString == "abortedByUser")
      state = eMyMoney::OnlineJob::sendingState::abortedByUser;
    else if (stateString == "sendingError")
      state = eMyMoney::OnlineJob::sendingState::sendingError;
    else // includes: stateString == "noBankAnswer"
      state = eMyMoney::OnlineJob::sendingState::noBankAnswer;

    job.setBankAnswer(state, query.value(4).toDateTime());
    job.setLock(query.value(5).toString() == QLatin1String("Y") ? true : false);
    jobList.insert(job.id(), job);
    if (d->m_displayStatus)
      d->signalProgress(++progress, 0);
  }
  return jobList;
}

QMap<QString, onlineJob> MyMoneyStorageSql::fetchOnlineJobs() const
{
  return fetchOnlineJobs(QStringList(), false);
}

payeeIdentifier MyMoneyStorageSql::fetchPayeeIdentifier(const QString& id) const
{
  QMap<QString, payeeIdentifier> list = fetchPayeeIdentifiers(QStringList(id));
  QMap<QString, payeeIdentifier>::const_iterator iter = list.constFind(id);
  if (iter == list.constEnd())
    throw MYMONEYEXCEPTION(QString::fromLatin1("payeeIdentifier with id '%1' not found").arg(id)); // krazy:exclude=crashy
  return *iter;
}

QMap< QString, payeeIdentifier > MyMoneyStorageSql::fetchPayeeIdentifiers(const QStringList& idList) const
{
  Q_D(const MyMoneyStorageSql);
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  // Create query
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    query.prepare("SELECT id, type FROM kmmPayeeIdentifier;");
  } else {
    QString queryIdSet = QString("?, ").repeated(idList.length());
    queryIdSet.chop(2);   // remove ", " from end
    query.prepare(QLatin1String("SELECT id, type FROM kmmPayeeIdentifier WHERE id IN (") + queryIdSet + QLatin1String(");"));

    QStringList::const_iterator end = idList.constEnd();
    for (QStringList::const_iterator iter = idList.constBegin(); iter != end; ++iter) {
      query.addBindValue(*iter);
    }
  }
  if (!query.exec())
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading payee identifiers")); // krazy:exclude=crashy

  QMap<QString, payeeIdentifier> identList;

  while (query.next()) {
    const auto id = query.value(0).toString();
    identList.insert(id, d->createPayeeIdentifierObject(*this, query.value(1).toString(), id));
  }

  return identList;
}

QMap< QString, payeeIdentifier > MyMoneyStorageSql::fetchPayeeIdentifiers() const
{
  return fetchPayeeIdentifiers(QStringList());
}

QMap<QString, MyMoneyTag> MyMoneyStorageSql::fetchTags(const QStringList& idList, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (d->m_displayStatus) {
    int tagsNb = (idList.isEmpty() ? d->m_tags : idList.size());
    d->signalProgress(0, tagsNb, QObject::tr("Loading tags..."));
  } else {
//    if (m_tagListRead) return;
  }
  int progress = 0;
  QMap<QString, MyMoneyTag> taList;
  //ulong lastId;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmTags"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    query.prepare(t.selectAllString());
  } else {
    QString whereClause = " where (";
    QString itemConnector = "";
    foreach (const QString& it, idList) {
      whereClause.append(QString("%1id = '%2'").arg(itemConnector).arg(it));
      itemConnector = " or ";
    }
    whereClause += ')';
    query.prepare(t.selectAllString(false) + whereClause);
  }
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Tag")); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int notesCol = t.fieldNumber("notes");
  int tagColorCol = t.fieldNumber("tagColor");
  int closedCol = t.fieldNumber("closed");

  while (query.next()) {
    QString pid;
    MyMoneyTag tag;
    pid = GETSTRING(idCol);
    tag.setName(GETSTRING(nameCol));
    tag.setNotes(GETSTRING(notesCol));
    tag.setClosed((GETSTRING(closedCol) == "Y"));
    tag.setTagColor(QColor(GETSTRING(tagColorCol)));
    taList[pid] = MyMoneyTag(pid, tag);
    if (d->m_displayStatus) d->signalProgress(++progress, 0);
  }
  return taList;
}

QMap<QString, MyMoneyTag> MyMoneyStorageSql::fetchTags() const
{
  return fetchTags(QStringList(), false);
}

QMap<QString, MyMoneyAccount> MyMoneyStorageSql::fetchAccounts(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int accountsNb = (idList.isEmpty() ? d->m_accounts : idList.size());
  d->signalProgress(0, accountsNb, QObject::tr("Loading accounts..."));
  int progress = 0;
  QMap<QString, MyMoneyAccount> accList;
  QStringList kvpAccountList(idList);

  const MyMoneyDbTable& t = d->m_db.m_tables["kmmAccounts"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QSqlQuery sq(*const_cast <MyMoneyStorageSql*>(this));

  QString childQueryString = "SELECT id, parentId FROM kmmAccounts WHERE ";
  QString queryString(t.selectAllString(false));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  if (! idList.empty()) {
    queryString += " WHERE id IN (";
    childQueryString += " parentId IN (";
    QString inString;
    for (int i = 0; i < idList.count(); ++i) {
      inString += QString(":id%1, ").arg(i);
    }
    inString = inString.left(inString.length() - 2) + ')';

    queryString += inString;
    childQueryString += inString;
  } else {
    childQueryString += " NOT parentId IS NULL";
  }

  queryString += " ORDER BY id";
  childQueryString += " ORDER BY parentid, id";

  if (forUpdate) {
    queryString += d->m_driver->forUpdateString();
    childQueryString += d->m_driver->forUpdateString();
  }

  query.prepare(queryString);
  sq.prepare(childQueryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      query.bindValue(QString(":id%1").arg(i), *bindVal);
      sq.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Account")); // krazy:exclude=crashy
  if (!sq.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading subAccountList")); // krazy:exclude=crashy

  // Reserve enough space for all values. Approximate it with the size of the
  // idList in case the db doesn't support reporting the size of the
  // resultset to the caller.
  //FIXME: this is for if/when there is a QHash conversion
  //accList.reserve(q.size() > 0 ? q.size() : idList.size());

  static const int idCol = t.fieldNumber("id");
  static const int institutionIdCol = t.fieldNumber("institutionId");
  static const int parentIdCol = t.fieldNumber("parentId");
  static const int lastReconciledCol = t.fieldNumber("lastReconciled");
  static const int lastModifiedCol = t.fieldNumber("lastModified");
  static const int openingDateCol = t.fieldNumber("openingDate");
  static const int accountNumberCol = t.fieldNumber("accountNumber");
  static const int accountTypeCol = t.fieldNumber("accountType");
  static const int accountNameCol = t.fieldNumber("accountName");
  static const int descriptionCol = t.fieldNumber("description");
  static const int currencyIdCol = t.fieldNumber("currencyId");
  static const int balanceCol = t.fieldNumber("balance");
  static const int transactionCountCol = t.fieldNumber("transactionCount");

  while (query.next()) {
    QString aid;
    MyMoneyAccount acc;

    aid = GETSTRING(idCol);
    acc.setInstitutionId(GETSTRING(institutionIdCol));
    acc.setParentAccountId(GETSTRING(parentIdCol));
    acc.setLastReconciliationDate(GETDATE_D(lastReconciledCol));
    acc.setLastModified(GETDATE_D(lastModifiedCol));
    acc.setOpeningDate(GETDATE_D(openingDateCol));
    acc.setNumber(GETSTRING(accountNumberCol));
    acc.setAccountType(static_cast<Account::Type>(GETINT(accountTypeCol)));
    acc.setName(GETSTRING(accountNameCol));
    acc.setDescription(GETSTRING(descriptionCol));
    acc.setCurrencyId(GETSTRING(currencyIdCol));
    acc.setBalance(MyMoneyMoney(GETSTRING(balanceCol)));
    const_cast <MyMoneyStorageSql*>(this)->d_func()->m_transactionCountMap[aid] = (ulong) GETULL(transactionCountCol);

    // Process any key value pair
    if (idList.empty())
      kvpAccountList.append(aid);

    accList.insert(aid, MyMoneyAccount(aid, acc));
    if (acc.value("PreferredAccount") == "Yes") {
      const_cast <MyMoneyStorageSql*>(this)->d_func()->m_preferred.addAccount(aid);
    }
    d->signalProgress(++progress, 0);
  }

  QMap<QString, MyMoneyAccount>::Iterator it_acc;
  QMap<QString, MyMoneyAccount>::Iterator accListEnd = accList.end();
  while (sq.next()) {
    it_acc = accList.find(sq.value(1).toString());
    if (it_acc != accListEnd && it_acc.value().id() == sq.value(1).toString()) {
      while (sq.isValid() && it_acc != accListEnd
             && it_acc.value().id() == sq.value(1).toString()) {
        it_acc.value().addAccountId(sq.value(0).toString());
        if (!sq.next())
          break;
      }
      sq.previous();
    }
  }

  //TODO: There should be a better way than this.  What's below is O(n log n) or more,
  // where it may be able to be done in O(n), if things are just right.
  // The operator[] call in the loop is the most expensive call in this function, according
  // to several profile runs.
  QHash <QString, MyMoneyKeyValueContainer> kvpResult = d->readKeyValuePairs("ACCOUNT", kvpAccountList);
  QHash <QString, MyMoneyKeyValueContainer>::const_iterator kvp_end = kvpResult.constEnd();
  for (QHash <QString, MyMoneyKeyValueContainer>::const_iterator it_kvp = kvpResult.constBegin();
       it_kvp != kvp_end; ++it_kvp) {
    accList[it_kvp.key()].setPairs(it_kvp.value().pairs());
  }

  kvpResult = d->readKeyValuePairs("ONLINEBANKING", kvpAccountList);
  kvp_end = kvpResult.constEnd();
  for (QHash <QString, MyMoneyKeyValueContainer>::const_iterator it_kvp = kvpResult.constBegin();
       it_kvp != kvp_end; ++it_kvp) {
    accList[it_kvp.key()].setOnlineBankingSettings(it_kvp.value());
  }

  return accList;
}

QMap<QString, MyMoneyAccount> MyMoneyStorageSql::fetchAccounts() const
{
  return fetchAccounts(QStringList(), false);
}

QMap<QString, MyMoneyMoney> MyMoneyStorageSql::fetchBalance(const QStringList& idList, const QDate& date) const
{
  Q_D(const MyMoneyStorageSql);
  QMap<QString, MyMoneyMoney> returnValue;
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString = "SELECT action, shares, accountId, postDate "
                        "FROM kmmSplits WHERE txType = 'N'";

  if (idList.count() > 0) {
    queryString += "AND accountId in (";

    for (int i = 0; i < idList.count(); ++i) {
      queryString += QString(":id%1, ").arg(i);
    }
    queryString = queryString.left(queryString.length() - 2) + ')';
  }

  // SQLite stores dates as YYYY-MM-DDTHH:mm:ss with 0s for the time part. This makes
  // the <= operator misbehave when the date matches. To avoid this, add a day to the
  // requested date and use the < operator.
  if (date.isValid() && !date.isNull())
    queryString += QString(" AND postDate < '%1'").arg(date.addDays(1).toString(Qt::ISODate));

  queryString += " ORDER BY accountId, postDate;";
  //DBG(queryString);
  query.prepare(queryString);

  int i = 0;
  foreach (const QString& bindVal, idList) {
    query.bindValue(QString(":id%1").arg(i), bindVal);
    ++i;
  }

  if (!query.exec()) // krazy:exclude=crashy
    throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("fetching balance"));
  QString id;
  QString oldId;
  MyMoneyMoney temp;
  while (query.next()) {
    id = query.value(2).toString();
    // If the old ID does not match the new ID, then the account being summed has changed.
    // Write the balance into the returnValue map and update the oldId to the current one.
    if (id != oldId) {
      if (!oldId.isEmpty()) {
        returnValue.insert(oldId, temp);
        temp = 0;
      }
      oldId = id;
    }
    if (MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares) == query.value(0).toString())
      temp *= MyMoneyMoney(query.value(1).toString());
    else
      temp += MyMoneyMoney(query.value(1).toString());
  }
  // Do not forget the last id in the list.
  returnValue.insert(id, temp);

  // Return the map.
  return returnValue;
}

void MyMoneyStorageSql::readTransactions(const MyMoneyTransactionFilter& filter)
{
  Q_D(MyMoneyStorageSql);
  try {
    d->m_storage->loadTransactions(fetchTransactions(filter));
  } catch (const MyMoneyException &) {
    throw;
  }
}

QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions(const QString& tidList, const QString& dateClause, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
//  if (m_transactionListRead) return; // all list already in memory
  if (d->m_displayStatus) {
    int transactionsNb = (tidList.isEmpty() ? d->m_transactions : tidList.size());
    d->signalProgress(0, transactionsNb, QObject::tr("Loading transactions..."));
  }
  int progress = 0;
//  m_payeeList.clear();
  QString whereClause = " WHERE txType = 'N' ";
  if (! tidList.isEmpty()) {
    whereClause += " AND id IN " + tidList;
  }
  if (!dateClause.isEmpty()) whereClause += " and " + dateClause;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmTransactions"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  query.prepare(t.selectAllString(false) + whereClause + " ORDER BY id;");
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Transaction")); // krazy:exclude=crashy
  const MyMoneyDbTable& ts = d->m_db.m_tables["kmmSplits"];
  whereClause = " WHERE txType = 'N' ";
  if (! tidList.isEmpty()) {
    whereClause += " AND transactionId IN " + tidList;
  }
  if (!dateClause.isEmpty()) whereClause += " and " + dateClause;
  QSqlQuery qs(*const_cast <MyMoneyStorageSql*>(this));
  QString splitQuery = ts.selectAllString(false) + whereClause
                       + " ORDER BY transactionId, splitId;";
  qs.prepare(splitQuery);
  if (!qs.exec()) throw MYMONEYEXCEPTION(d->buildError(qs, Q_FUNC_INFO, "reading Splits")); // krazy:exclude=crashy
  QString splitTxId = "ZZZ";
  MyMoneySplit s;
  if (qs.next()) {
    splitTxId = qs.value(0).toString();
    s = d->readSplit(qs);
  } else {
    splitTxId = "ZZZ";
  }
  QMap <QString, MyMoneyTransaction> txMap;
  QStringList txList;
  int idCol = t.fieldNumber("id");
  int postDateCol = t.fieldNumber("postDate");
  int memoCol = t.fieldNumber("memo");
  int entryDateCol = t.fieldNumber("entryDate");
  int currencyIdCol = t.fieldNumber("currencyId");
  int bankIdCol = t.fieldNumber("bankId");

  while (query.next()) {
    MyMoneyTransaction tx;
    QString txId = GETSTRING(idCol);
    tx.setPostDate(GETDATE_D(postDateCol));
    tx.setMemo(GETSTRING(memoCol));
    tx.setEntryDate(GETDATE_D(entryDateCol));
    tx.setCommodity(GETSTRING(currencyIdCol));
    tx.setBankID(GETSTRING(bankIdCol));

    // skip all splits while the transaction id of the split is less than
    // the transaction id of the current transaction. Don't forget to check
    // for the ZZZ flag for the end of the list.
    while (txId < splitTxId && splitTxId != "ZZZ") {
      if (qs.next()) {
        splitTxId = qs.value(0).toString();
        s = d->readSplit(qs);
      } else {
        splitTxId = "ZZZ";
      }
    }

    // while the split transaction id matches the current transaction id,
    // add the split to the current transaction. Set the ZZZ flag if
    // all splits for this transaction have been read.
    while (txId == splitTxId) {
      tx.addSplit(s);
      if (qs.next()) {
        splitTxId = qs.value(0).toString();
        s = d->readSplit(qs);
      } else {
        splitTxId = "ZZZ";
      }
    }

    // Process any key value pair
    if (! txId.isEmpty()) {
      txList.append(txId);
      tx = MyMoneyTransaction(txId, tx);
      txMap.insert(tx.uniqueSortKey(), tx);
    }
  }

  // get the kvps
  QHash <QString, MyMoneyKeyValueContainer> kvpMap = d->readKeyValuePairs("TRANSACTION", txList);
  QMap<QString, MyMoneyTransaction>::Iterator txMapEnd = txMap.end();
  for (QMap<QString, MyMoneyTransaction>::Iterator i = txMap.begin();
       i != txMapEnd; ++i) {
    i.value().setPairs(kvpMap[i.value().id()].pairs());

    if (d->m_displayStatus) d->signalProgress(++progress, 0);
  }

  if ((tidList.isEmpty()) && (dateClause.isEmpty())) {
    //qDebug("setting full list read");
  }
  return txMap;
}

QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions(const QString& tidList) const
{
  return fetchTransactions(tidList, QString(), false);
}

QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions() const
{
  return fetchTransactions(QString(), QString(), false);
}

QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions(const MyMoneyTransactionFilter& filter) const
{
  Q_D(const MyMoneyStorageSql);
  // analyze the filter
//  if (m_transactionListRead) return; // all list already in memory
  // if the filter is restricted to certain accounts/categories
  // check if we already have them all in memory
  QStringList accounts;
  QString inQuery;
  filter.accounts(accounts);
  filter.categories(accounts);
//  QStringList::iterator it;
//  bool allAccountsLoaded = true;
//  for (it = accounts.begin(); it != accounts.end(); ++it) {
//    if (m_accountsLoaded.find(*it) == m_accountsLoaded.end()) {
//      allAccountsLoaded = false;
//      break;
//    }
//  }
//  if (allAccountsLoaded) return;
  /* Some filter combinations do not lend themselves to implementation
  * in SQL, or are likely to require such extensive reading of the database
  * as to make it easier to just read everything into memory.  */
  bool canImplementFilter = true;
  MyMoneyMoney m1, m2;
  if (filter.amountFilter(m1, m2)) {
    d->alert("Amount Filter Set");
    canImplementFilter = false;
  }
  QString n1, n2;
  if (filter.numberFilter(n1, n2)) {
    d->alert("Number filter set");
    canImplementFilter = false;
  }
  int t1;
  if (filter.firstType(t1)) {
    d->alert("Type filter set");
    canImplementFilter = false;
  }
//  int s1;
//  if (filter.firstState(s1)) {
//    alert("State filter set");
//    canImplementFilter = false;
//  }
  QRegExp t2;
  if (filter.textFilter(t2)) {
    d->alert("text filter set");
    canImplementFilter = false;
  }
  MyMoneyTransactionFilter::FilterSet s = filter.filterSet();
  if (s.singleFilter.validityFilter) {
    d->alert("Validity filter set");
    canImplementFilter = false;
  }
  if (!canImplementFilter) {
    QMap<QString, MyMoneyTransaction> transactionList =  fetchTransactions();

    std::remove_if(transactionList.begin(), transactionList.end(), FilterFail(filter));
    return transactionList;
  }

  bool splitFilterActive = false; // the split filter is active if we are selecting on fields in the split table
  // get start and end dates
  QDate start = filter.fromDate();
  QDate end = filter.toDate();
  // not entirely sure if the following is correct, but at best, saves a lot of reads, at worst
  // it only causes us to read a few more transactions that strictly necessary (I think...)
  if (start == MyMoneyStorageSqlPrivate::m_startDate) start = QDate();
  bool txFilterActive = ((start != QDate()) || (end != QDate())); // and this for fields in the transaction table

  QString whereClause = "";
  QString subClauseconnector = " where txType = 'N' and ";
  // payees
  QStringList payees;
  if (filter.payees(payees)) {
    QString itemConnector = "payeeId in (";
    QString payeesClause = "";
    foreach (const QString& it, payees) {
      payeesClause.append(QString("%1'%2'")
                          .arg(itemConnector).arg(it));
      itemConnector = ", ";
    }
    if (!payeesClause.isEmpty()) {
      whereClause += subClauseconnector + payeesClause + ')';
      subClauseconnector = " and ";
    }
    splitFilterActive = true;
  }

  //tags
  QStringList tags;
  if (filter.tags(tags)) {
    QString itemConnector = "splitId in ( SELECT splitId from kmmTagSplits where kmmTagSplits.transactionId = kmmSplits.transactionId and tagId in (";
    QString tagsClause = "";
    foreach (const QString& it, tags) {
      tagsClause.append(QString("%1'%2'")
                        .arg(itemConnector).arg(it));
      itemConnector = ", ";
    }
    if (!tagsClause.isEmpty()) {
      whereClause += subClauseconnector + tagsClause + ')';
      subClauseconnector = " and ";
    }
    splitFilterActive = true;
  }

  // accounts and categories
  if (!accounts.isEmpty()) {
    splitFilterActive = true;
    QString itemConnector = "accountId in (";
    QString accountsClause = "";
    foreach (const QString& it, accounts) {
      accountsClause.append(QString("%1 '%2'")
                            .arg(itemConnector).arg(it));
      itemConnector = ", ";
    }
    if (!accountsClause.isEmpty()) {
      whereClause += subClauseconnector + accountsClause + ')';
      subClauseconnector = " and (";
    }
  }

  // split states
  QList <int> splitStates;
  if (filter.states(splitStates)) {
    splitFilterActive = true;
    QString itemConnector = " reconcileFlag IN (";
    QString statesClause = "";
    foreach (int it, splitStates) {
      statesClause.append(QString(" %1 '%2'").arg(itemConnector)
                          .arg(d->splitState(TransactionFilter::State(it))));
      itemConnector = ',';
    }
    if (!statesClause.isEmpty()) {
      whereClause += subClauseconnector + statesClause + ')';
      subClauseconnector = " and (";
    }
  }
  // I've given up trying to work out the logic. we keep getting the wrong number of close brackets
  int obc = whereClause.count('(');
  int cbc = whereClause.count(')');
  if (cbc > obc) {
    qDebug() << "invalid where clause " << whereClause;
    qFatal("aborting");
  }
  while (cbc < obc) {
    whereClause.append(')');
    cbc++;
  }
  // if the split filter is active, but the where clause and the date filter is empty
  // it means we already have all the transactions for the specified filter
  // in memory, so just exit
  if ((splitFilterActive) && (whereClause.isEmpty()) && (!txFilterActive)) {
    qDebug("all transactions already in storage");
    return fetchTransactions();
  }

  // if we have neither a split filter, nor a tx (date) filter
  // it's effectively a read all
  if ((!splitFilterActive) && (!txFilterActive)) {
    //qDebug("reading all transactions");
    return fetchTransactions();
  }
  // build a date clause for the transaction table
  QString dateClause;
  QString connector = "";
  if (end != QDate()) {
    dateClause = QString("(postDate < '%1')").arg(end.addDays(1).toString(Qt::ISODate));
    connector = " and ";
  }
  if (start != QDate()) {
    dateClause += QString("%1 (postDate >= '%2')").arg(connector).arg(start.toString(Qt::ISODate));
  }
  // now get a list of transaction ids
  // if we have only a date filter, we need to build the list from the tx table
  // otherwise we need to build from the split table
  if (splitFilterActive) {
    inQuery = QString("(select distinct transactionId from kmmSplits %1)").arg(whereClause);
  } else {
    inQuery = QString("(select distinct id from kmmTransactions where %1)").arg(dateClause);
    txFilterActive = false; // kill off the date filter now
  }

  return fetchTransactions(inQuery, dateClause);
  //FIXME: if we have an accounts-only filter, recalc balances on loaded accounts
}

ulong MyMoneyStorageSql::transactionCount(const QString& aid) const
{
  Q_D(const MyMoneyStorageSql);
  if (aid.isEmpty())
    return d->m_transactions;
  else
    return d->m_transactionCountMap[aid];
}

QHash<QString, ulong> MyMoneyStorageSql::transactionCountMap() const
{
  Q_D(const MyMoneyStorageSql);
  return d->m_transactionCountMap;
}

bool MyMoneyStorageSql::isReferencedByTransaction(const QString& id) const
{
  Q_D(const MyMoneyStorageSql);
  //FIXME-ALEX should I add sub query for kmmTagSplits here?
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare("SELECT COUNT(*) FROM kmmTransactions "
            "INNER JOIN kmmSplits ON kmmTransactions.id = kmmSplits.transactionId "
            "WHERE kmmTransactions.currencyId = :ID OR kmmSplits.payeeId = :ID "
            "OR kmmSplits.accountId = :ID OR kmmSplits.costCenterId = :ID");
  q.bindValue(":ID", id);
  if ((!q.exec()) || (!q.next())) { // krazy:exclude=crashy
    d->buildError(q, Q_FUNC_INFO, "error retrieving reference count");
    qFatal("Error retrieving reference count"); // definitely shouldn't happen
  }
  return (0 != q.value(0).toULongLong());
}

QMap<QString, MyMoneySchedule> MyMoneyStorageSql::fetchSchedules(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int schedulesNb = (idList.isEmpty() ? d->m_schedules : idList.size());
  d->signalProgress(0, schedulesNb, QObject::tr("Loading schedules..."));
  int progress = 0;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmSchedules"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QMap<QString, MyMoneySchedule> sList;
  //ulong lastId = 0;
  const MyMoneyDbTable& ts = d->m_db.m_tables["kmmSplits"];
  QSqlQuery qs(*const_cast <MyMoneyStorageSql*>(this));
  qs.prepare(ts.selectAllString(false) + " WHERE transactionId = :id ORDER BY splitId;");
  QSqlQuery sq(*const_cast <MyMoneyStorageSql*>(this));
  sq.prepare("SELECT payDate from kmmSchedulePaymentHistory where schedId = :id");

  QString queryString(t.selectAllString(false));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  if (! idList.empty()) {
    queryString += " WHERE";
    for (int i = 0; i < idList.count(); ++i)
      queryString += QString(" id = :id%1 OR").arg(i);
    queryString = queryString.left(queryString.length() - 2);
  }
  queryString += " ORDER BY id";

  if (forUpdate)
    queryString += d->m_driver->forUpdateString();

  query.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      query.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Schedules")); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int typeCol = t.fieldNumber("type");
  int occurrenceCol = t.fieldNumber("occurence"); // krazy:exclude=spelling
  int occurrenceMultiplierCol = t.fieldNumber("occurenceMultiplier"); // krazy:exclude=spelling
  int paymentTypeCol = t.fieldNumber("paymentType");
  int startDateCol = t.fieldNumber("startDate");
  int endDateCol = t.fieldNumber("endDate");
  int fixedCol = t.fieldNumber("fixed");
  int lastDayInMonthCol = t.fieldNumber("lastDayInMonth");
  int autoEnterCol = t.fieldNumber("autoEnter");
  int lastPaymentCol = t.fieldNumber("lastPayment");
  int weekendOptionCol = t.fieldNumber("weekendOption");
  int nextPaymentDueCol = t.fieldNumber("nextPaymentDue");

  while (query.next()) {
    MyMoneySchedule s;
    QString boolChar;

    QString sId = GETSTRING(idCol);
    s.setName(GETSTRING(nameCol));
    s.setType(static_cast<Schedule::Type>(GETINT(typeCol)));
    s.setOccurrencePeriod(static_cast<Schedule::Occurrence>(GETINT(occurrenceCol)));
    s.setOccurrenceMultiplier(GETINT(occurrenceMultiplierCol));
    s.setPaymentType(static_cast<Schedule::PaymentType>(GETINT(paymentTypeCol)));
    s.setStartDate(GETDATE_D(startDateCol));
    s.setEndDate(GETDATE_D(endDateCol));
    boolChar = GETSTRING(fixedCol); s.setFixed(boolChar == "Y");
    boolChar = GETSTRING(lastDayInMonthCol); s.setLastDayInMonth(boolChar == "Y");
    boolChar = GETSTRING(autoEnterCol); s.setAutoEnter(boolChar == "Y");
    s.setLastPayment(GETDATE_D(lastPaymentCol));
    s.setWeekendOption(static_cast<Schedule::WeekendOption>(GETINT(weekendOptionCol)));
    QDate nextPaymentDue = GETDATE_D(nextPaymentDueCol);

    // convert simple occurrence to compound occurrence
    int mult = s.occurrenceMultiplier();
    Schedule::Occurrence occ = s.occurrencePeriod();
    MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
    s.setOccurrencePeriod(occ);
    s.setOccurrenceMultiplier(mult);
    // now assign the id to the schedule
    MyMoneySchedule _s(sId, s);
    s = _s;
    // read the associated transaction
//    m_payeeList.clear();

    const MyMoneyDbTable& transactionTable = d->m_db.m_tables["kmmTransactions"];
    QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
    q.prepare(transactionTable.selectAllString(false) + " WHERE id = :id;");
    q.bindValue(":id", s.id());
    if (!q.exec()) throw MYMONEYEXCEPTION(d->buildError(q, Q_FUNC_INFO, QString("reading Scheduled Transaction"))); // krazy:exclude=crashy
    QSqlRecord rec = q.record();
    if (!q.next()) throw MYMONEYEXCEPTION(d->buildError(q, Q_FUNC_INFO, QString("retrieving scheduled transaction")));
    MyMoneyTransaction tx(s.id(), MyMoneyTransaction());
    // we cannot use the GET.... macros here as they are bound to the query variable
    tx.setPostDate(d->getDate(q.value(transactionTable.fieldNumber("postDate")).toString()));
    tx.setMemo(q.value(transactionTable.fieldNumber("memo")).toString());
    tx.setEntryDate(d->getDate(q.value(transactionTable.fieldNumber("entryDate")).toString()));
    tx.setCommodity(q.value(transactionTable.fieldNumber("currencyId")).toString());
    tx.setBankID(q.value(transactionTable.fieldNumber("bankId")).toString());

    qs.bindValue(":id", s.id());
    if (!qs.exec()) throw MYMONEYEXCEPTION(d->buildError(qs, Q_FUNC_INFO, "reading Scheduled Splits")); // krazy:exclude=crashy
    while (qs.next()) {
      MyMoneySplit sp(d->readSplit(qs));
      tx.addSplit(sp);
    }
//    if (!m_payeeList.isEmpty())
//      readPayees(m_payeeList);
    // Process any key value pair
    tx.setPairs(d->readKeyValuePairs("TRANSACTION", s.id()).pairs());

    // If the transaction doesn't have a post date, setTransaction will reject it.
    // The old way of handling things was to store the next post date in the schedule object
    // and set the transaction post date to QDate().
    // For compatibility, if this is the case, copy the next post date from the schedule object
    // to the transaction object post date.
    if (!tx.postDate().isValid()) {
      tx.setPostDate(nextPaymentDue);
    }

    s.setTransaction(tx);

    // read in the recorded payments
    sq.bindValue(":id", s.id());
    if (!sq.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading schedule payment history")); // krazy:exclude=crashy
    while (sq.next()) s.recordPayment(sq.value(0).toDate());

    sList[s.id()] = s;

    //FIXME: enable when schedules have KVPs.
    //  s.setPairs(readKeyValuePairs("SCHEDULE", s.id()).pairs());

    //ulong id = MyMoneyUtils::extractId(s.id().data());
    //if(id > lastId)
    //  lastId = id;

    d->signalProgress(++progress, 0);
  }
  return sList;
}

QMap<QString, MyMoneySchedule> MyMoneyStorageSql::fetchSchedules() const
{
  return fetchSchedules(QStringList(), false);
}

QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchSecurities(const QStringList& /*idList*/, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
  d->signalProgress(0, d->m_securities, QObject::tr("Loading securities..."));
  int progress = 0;
  QMap<QString, MyMoneySecurity> sList;
  ulong lastId = 0;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmSecurities"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  query.prepare(t.selectAllString(false) + " ORDER BY id;");
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Securities")); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int symbolCol = t.fieldNumber("symbol");
  int typeCol = t.fieldNumber("type");
  int roundingMethodCol = t.fieldNumber("roundingMethod");
  int smallestAccountFractionCol = t.fieldNumber("smallestAccountFraction");
  int pricePrecisionCol = t.fieldNumber("pricePrecision");
  int tradingCurrencyCol = t.fieldNumber("tradingCurrency");
  int tradingMarketCol = t.fieldNumber("tradingMarket");

  while (query.next()) {
    MyMoneySecurity e;
    QString eid;
    eid = GETSTRING(idCol);
    e.setName(GETSTRING(nameCol));
    e.setTradingSymbol(GETSTRING(symbolCol));
    e.setSecurityType(static_cast<Security::Type>(GETINT(typeCol)));
    e.setRoundingMethod(static_cast<AlkValue::RoundingMethod>(GETINT(roundingMethodCol)));
    int saf = GETINT(smallestAccountFractionCol);
    int pp = GETINT(pricePrecisionCol);
    e.setTradingCurrency(GETSTRING(tradingCurrencyCol));
    e.setTradingMarket(GETSTRING(tradingMarketCol));

    if (e.tradingCurrency().isEmpty())
      e.setTradingCurrency(d->m_storage->pairs()["kmm-baseCurrency"]);
    if (saf == 0)
      saf = 100;
    if (pp == 0 || pp > 10)
      pp = 4;
    e.setSmallestAccountFraction(saf);
    e.setPricePrecision(pp);

    // Process any key value pairs
    e.setPairs(d->readKeyValuePairs("SECURITY", eid).pairs());
    //tell the storage objects we have a new security object.

    // FIXME: Adapt to new interface make sure, to take care of the currencies as well
    //   see MyMoneyStorageXML::readSecurites()
    MyMoneySecurity security(eid, e);
    sList[security.id()] = security;

    ulong id = MyMoneyUtils::extractId(security.id());
    if (id > lastId)
      lastId = id;

    d->signalProgress(++progress, 0);
  }
  return sList;
}

QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchSecurities() const
{
  return fetchSecurities(QStringList(), false);
}

MyMoneyPrice MyMoneyStorageSql::fetchSinglePrice(const QString& fromId, const QString& toId, const QDate& date_, bool exactDate, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmPrices"];

  static const int priceDateCol = t.fieldNumber("priceDate");
  static const int priceCol = t.fieldNumber("price");
  static const int priceSourceCol = t.fieldNumber("priceSource");

  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  // See balance query for why the date logic seems odd.
  QString queryString = t.selectAllString(false) +
                        " WHERE fromId = :fromId  AND toId = :toId AND priceDate < :priceDate ";

  if (exactDate)
    queryString += "AND priceDate > :exactDate ";

  queryString += "ORDER BY priceDate DESC;";

  query.prepare(queryString);

  QDate date(date_);

  if (!date.isValid())
    date = QDate::currentDate();

  query.bindValue(":fromId", fromId);
  query.bindValue(":toId", toId);
  query.bindValue(":priceDate", date.addDays(1).toString(Qt::ISODate));

  if (exactDate)
    query.bindValue(":exactDate", date.toString(Qt::ISODate));

  if (! query.exec()) return MyMoneyPrice(); // krazy:exclude=crashy

  if (query.next()) {

    return MyMoneyPrice(fromId,
                        toId,
                        GETDATE_D(priceDateCol),
                        MyMoneyMoney(GETSTRING(priceCol)),
                        GETSTRING(priceSourceCol));
  }

  return MyMoneyPrice();
}

MyMoneyPriceList MyMoneyStorageSql::fetchPrices(const QStringList& fromIdList, const QStringList& toIdList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int pricesNb = (fromIdList.isEmpty() ? d->m_prices : fromIdList.size());
  d->signalProgress(0, pricesNb, QObject::tr("Loading prices..."));
  int progress = 0;
  const_cast <MyMoneyStorageSql*>(this)->d_func()->m_readingPrices = true;
  MyMoneyPriceList pList;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmPrices"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString = t.selectAllString(false);

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  if (! fromIdList.empty()) {
    queryString += " WHERE (";
    for (int i = 0; i < fromIdList.count(); ++i) {
      queryString += QString(" fromId = :fromId%1 OR").arg(i);
    }
    queryString = queryString.left(queryString.length() - 2) + ')';
  }
  if (! toIdList.empty()) {
    queryString += " AND (";
    for (int i = 0; i < toIdList.count(); ++i) {
      queryString += QString(" toId = :toId%1 OR").arg(i);
    }
    queryString = queryString.left(queryString.length() - 2) + ')';
  }

  if (forUpdate)
    queryString += d->m_driver->forUpdateString();

  queryString += ';';

  query.prepare(queryString);

  if (! fromIdList.empty()) {
    QStringList::ConstIterator bindVal = fromIdList.constBegin();
    for (int i = 0; bindVal != fromIdList.constEnd(); ++i, ++bindVal) {
      query.bindValue(QString(":fromId%1").arg(i), *bindVal);
    }
  }
  if (! toIdList.empty()) {
    QStringList::ConstIterator bindVal = toIdList.constBegin();
    for (int i = 0; bindVal != toIdList.constEnd(); ++i, ++bindVal) {
      query.bindValue(QString(":toId%1").arg(i), *bindVal);
    }
  }

  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Prices")); // krazy:exclude=crashy
  static const int fromIdCol = t.fieldNumber("fromId");
  static const int toIdCol = t.fieldNumber("toId");
  static const int priceDateCol = t.fieldNumber("priceDate");
  static const int priceCol = t.fieldNumber("price");
  static const int priceSourceCol = t.fieldNumber("priceSource");

  while (query.next()) {
    QString from = GETSTRING(fromIdCol);
    QString to = GETSTRING(toIdCol);
    QDate date = GETDATE_D(priceDateCol);

    pList [MyMoneySecurityPair(from, to)].insert(date, MyMoneyPrice(from, to, date, MyMoneyMoney(GETSTRING(priceCol)), GETSTRING(priceSourceCol)));
    d->signalProgress(++progress, 0);
  }
  const_cast <MyMoneyStorageSql*>(this)->d_func()->m_readingPrices = false;

  return pList;
}

MyMoneyPriceList MyMoneyStorageSql::fetchPrices() const
{
  return fetchPrices(QStringList(), QStringList(), false);
}

QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchCurrencies(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int currenciesNb = (idList.isEmpty() ? d->m_currencies : idList.size());
  d->signalProgress(0, currenciesNb, QObject::tr("Loading currencies..."));
  int progress = 0;
  QMap<QString, MyMoneySecurity> cList;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmCurrencies"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));

  QString queryString(t.selectAllString(false));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  if (! idList.empty()) {
    queryString += " WHERE";
    for (int i = 0; i < idList.count(); ++i)
      queryString += QString(" isocode = :id%1 OR").arg(i);
    queryString = queryString.left(queryString.length() - 2);
  }

  queryString += " ORDER BY ISOcode";

  if (forUpdate)
    queryString += d->m_driver->forUpdateString();

  queryString += ';';

  query.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.end(); ++i, ++bindVal) {
      query.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading Currencies")); // krazy:exclude=crashy
  int ISOcodeCol = t.fieldNumber("ISOcode");
  int nameCol = t.fieldNumber("name");
  int typeCol = t.fieldNumber("type");
  int symbol1Col = t.fieldNumber("symbol1");
  int symbol2Col = t.fieldNumber("symbol2");
  int symbol3Col = t.fieldNumber("symbol3");
  int smallestCashFractionCol = t.fieldNumber("smallestCashFraction");
  int smallestAccountFractionCol = t.fieldNumber("smallestAccountFraction");
  int pricePrecisionCol = t.fieldNumber("pricePrecision");

  while (query.next()) {
    QString id;
    MyMoneySecurity c;
    QChar symbol[3];
    id = GETSTRING(ISOcodeCol);
    c.setName(GETSTRING(nameCol));
    c.setSecurityType(static_cast<Security::Type>(GETINT(typeCol)));
    symbol[0] = QChar(GETINT(symbol1Col));
    symbol[1] = QChar(GETINT(symbol2Col));
    symbol[2] = QChar(GETINT(symbol3Col));
    c.setSmallestCashFraction(GETINT(smallestCashFractionCol));
    c.setSmallestAccountFraction(GETINT(smallestAccountFractionCol));
    c.setPricePrecision(GETINT(pricePrecisionCol));
    c.setTradingSymbol(QString(symbol, 3).trimmed());

    cList[id] = MyMoneySecurity(id, c);

    d->signalProgress(++progress, 0);
  }
  return cList;
}

QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchCurrencies() const
{
  return fetchCurrencies(QStringList(), false);
}

QMap<QString, MyMoneyReport> MyMoneyStorageSql::fetchReports(const QStringList& /*idList*/, bool /*forUpdate*/) const
{
  Q_D(const MyMoneyStorageSql);
  d->signalProgress(0, d->m_reports, QObject::tr("Loading reports..."));
  int progress = 0;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmReportConfig"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  query.prepare(t.selectAllString(true));
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading reports")); // krazy:exclude=crashy
  int xmlCol = t.fieldNumber("XML");
  QMap<QString, MyMoneyReport> rList;
  while (query.next()) {
    QDomDocument dom;
    dom.setContent(GETSTRING(xmlCol), false);

    QDomNode child = dom.firstChild();
    child = child.firstChild();
    auto report = MyMoneyXmlContentHandler2::readReport(child.toElement());
    rList[report.id()] = report;

    d->signalProgress(++progress, 0);
  }
  return rList;
}

QMap<QString, MyMoneyReport> MyMoneyStorageSql::fetchReports() const
{
  return fetchReports(QStringList(), false);
}

QMap<QString, MyMoneyBudget> MyMoneyStorageSql::fetchBudgets(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  int budgetsNb = (idList.isEmpty() ? d->m_budgets : idList.size());
  d->signalProgress(0, budgetsNb, QObject::tr("Loading budgets..."));
  int progress = 0;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmBudgetConfig"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString(t.selectAllString(false));
  if (! idList.empty()) {
    queryString += " WHERE id = '" + idList.join("' OR id = '") + '\'';
  }
  if (forUpdate)
    queryString += d->m_driver->forUpdateString();

  queryString += ';';

  query.prepare(queryString);
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading budgets")); // krazy:exclude=crashy
  QMap<QString, MyMoneyBudget> budgets;
  int xmlCol = t.fieldNumber("XML");
  while (query.next()) {
    QDomDocument dom;
    dom.setContent(GETSTRING(xmlCol), false);

    QDomNode child = dom.firstChild();
    child = child.firstChild();
    auto budget = MyMoneyXmlContentHandler2::readBudget(child.toElement());
    budgets.insert(budget.id(), budget);
    d->signalProgress(++progress, 0);
  }
  return budgets;
}

QMap<QString, MyMoneyBudget> MyMoneyStorageSql::fetchBudgets() const
{
  return fetchBudgets(QStringList(), false);
}

ulong MyMoneyStorageSql::getNextBudgetId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdBudgets>(QLatin1String("kmmBudgetConfig"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextAccountId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdAccounts>(QLatin1String("kmmAccounts"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextInstitutionId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdInstitutions>(QLatin1String("kmmInstitutions"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextPayeeId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdPayees>(QLatin1String("kmmPayees"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextTagId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdTags>(QLatin1String("kmmTags"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextReportId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdReports>(QLatin1String("kmmReportConfig"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextScheduleId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdSchedules>(QLatin1String("kmmSchedules"), QLatin1String("id"), 3);
}

ulong MyMoneyStorageSql::getNextSecurityId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdSecurities>(QLatin1String("kmmSecurities"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextTransactionId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdTransactions>(QLatin1String("kmmTransactions"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextOnlineJobId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdOnlineJobs>(QLatin1String("kmmOnlineJobs"), QLatin1String("id"), 1);
}

ulong MyMoneyStorageSql::getNextPayeeIdentifierId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdPayeeIdentifier>(QLatin1String("kmmPayeeIdentifier"), QLatin1String("id"), 5);
}

ulong MyMoneyStorageSql::getNextCostCenterId() const
{
  Q_D(const MyMoneyStorageSql);
  return d->getNextId<&MyMoneyStorageSqlPrivate::m_hiIdCostCenter>(QLatin1String("kmmCostCenterIdentifier"), QLatin1String("id"), 5);
}

ulong MyMoneyStorageSql::incrementBudgetId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdBudgets = getNextBudgetId() + 1;
  return (d->m_hiIdBudgets - 1);
}

/**
 * @warning This method uses getNextAccountId() internally. The database is not informed which can cause issues
 * when the database is accessed concurrently. Then maybe a single id is used twice but the RDBMS will detect the
 * issue and KMyMoney crashes. This issue can only occur when two instances of KMyMoney access the same database.
 * But in this unlikly case MyMoneyStorageSql will have a lot more issues, I think.
 */
ulong MyMoneyStorageSql::incrementAccountId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdAccounts = getNextAccountId() + 1;
  return (d->m_hiIdAccounts - 1);
}

ulong MyMoneyStorageSql::incrementInstitutionId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdInstitutions = getNextInstitutionId() + 1;
  return (d->m_hiIdInstitutions - 1);
}

ulong MyMoneyStorageSql::incrementPayeeId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdPayees = getNextPayeeId() + 1;
  return (d->m_hiIdPayees - 1);
}

ulong MyMoneyStorageSql::incrementTagId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdTags = getNextTagId() + 1;
  return (d->m_hiIdTags - 1);
}

ulong MyMoneyStorageSql::incrementReportId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdReports = getNextReportId() + 1;
  return (d->m_hiIdReports - 1);
}

ulong MyMoneyStorageSql::incrementScheduleId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdSchedules = getNextScheduleId() + 1;
  return (d->m_hiIdSchedules - 1);
}

ulong MyMoneyStorageSql::incrementSecurityId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdSecurities = getNextSecurityId() + 1;
  return (d->m_hiIdSecurities - 1);
}

ulong MyMoneyStorageSql::incrementTransactionId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdTransactions = getNextTransactionId() + 1;
  return (d->m_hiIdTransactions - 1);
}

ulong MyMoneyStorageSql::incrementOnlineJobId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdOnlineJobs = getNextOnlineJobId() + 1;
  return (d->m_hiIdOnlineJobs - 1);
}

ulong MyMoneyStorageSql::incrementPayeeIdentfierId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdPayeeIdentifier = getNextPayeeIdentifierId() + 1;
  return (d->m_hiIdPayeeIdentifier - 1);
}

ulong MyMoneyStorageSql::incrementCostCenterId()
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdCostCenter = getNextCostCenterId() + 1;
  return (d->m_hiIdCostCenter - 1);
}

void MyMoneyStorageSql::loadAccountId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdAccounts = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadTransactionId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdTransactions = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadPayeeId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdPayees = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadTagId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdTags = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadInstitutionId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdInstitutions = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadScheduleId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdSchedules = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadSecurityId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdSecurities = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadReportId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdReports = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadBudgetId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdBudgets = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadOnlineJobId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdOnlineJobs = id;
  d->writeFileInfo();
}

void MyMoneyStorageSql::loadPayeeIdentifierId(ulong id)
{
  Q_D(MyMoneyStorageSql);
  d->m_hiIdPayeeIdentifier = id;
  d->writeFileInfo();
}

//****************************************************

void MyMoneyStorageSql::setProgressCallback(void(*callback)(int, int, const QString&))
{
  Q_D(MyMoneyStorageSql);
  d->m_progressCallback = callback;
}

void MyMoneyStorageSql::readFile(QIODevice* s, MyMoneyStorageMgr* storage)
{
  Q_UNUSED(s); Q_UNUSED(storage)
}

void MyMoneyStorageSql::writeFile(QIODevice* s, MyMoneyStorageMgr* storage)
{
  Q_UNUSED(s); Q_UNUSED(storage)
}

// **************************** Error display routine *******************************


QDate MyMoneyStorageSqlPrivate::m_startDate = QDate(1900, 1, 1);

void MyMoneyStorageSql::setStartDate(const QDate& startDate)
{
  MyMoneyStorageSqlPrivate::m_startDate = startDate;
}

QMap< QString, MyMoneyCostCenter > MyMoneyStorageSql::fetchCostCenters(const QStringList& idList, bool forUpdate) const
{
  Q_D(const MyMoneyStorageSql);
  Q_UNUSED(forUpdate);

  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (d->m_displayStatus) {
    int costCenterNb = (idList.isEmpty() ? 100 : idList.size());
    d->signalProgress(0, costCenterNb, QObject::tr("Loading cost center..."));
  }
  int progress = 0;
  QMap<QString, MyMoneyCostCenter> costCenterList;
  //ulong lastId;
  const MyMoneyDbTable& t = d->m_db.m_tables["kmmCostCenter"];
  QSqlQuery query(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    query.prepare(t.selectAllString());
  } else {
    QString whereClause = " where (";
    QString itemConnector = "";
    foreach (const QString& it, idList) {
      whereClause.append(QString("%1id = '%2'").arg(itemConnector).arg(it));
      itemConnector = " or ";
    }
    whereClause += ')';
    query.prepare(t.selectAllString(false) + whereClause);
  }
  if (!query.exec()) throw MYMONEYEXCEPTIONSQL_D(QString::fromLatin1("reading CostCenter")); // krazy:exclude=crashy
  const int idCol = t.fieldNumber("id");
  const int nameCol = t.fieldNumber("name");

  while (query.next()) {
    MyMoneyCostCenter costCenter;
    QString pid = GETSTRING(idCol);
    costCenter.setName(GETSTRING(nameCol));
    costCenterList[pid] = MyMoneyCostCenter(pid, costCenter);
    if (d->m_displayStatus) d->signalProgress(++progress, 0);
  }
  return costCenterList;
}

QMap< QString, MyMoneyCostCenter > MyMoneyStorageSql::fetchCostCenters() const
{
  return fetchCostCenters(QStringList(), false);
}
