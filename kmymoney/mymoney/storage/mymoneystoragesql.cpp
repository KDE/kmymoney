/***************************************************************************
                          mymoneystoragesql.cpp
                          ---------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
                         : Christian Dávid <christian-david@web.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneystoragesql.h"

// ----------------------------------------------------------------------------
// System Includes
#include <algorithm>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QIODevice>
#include <QUrlQuery>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QSqlRecord>
#include <QMap>
#include <QFile>
#include <QVariant>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
// TODO: port KF5 (needed for payeeidentifier plugin)
//#include <KServiceTypeTrader>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneystorage.h"
#include "imymoneyserialize.h"
#include "kmymoneystorageplugin.h"
#include "onlinejobadministration.h"
#include "payeeidentifier/payeeidentifierloader.h"
#include "onlinetasks/interfaces/tasks/onlinetask.h"
#include "mymoneycostcenter.h"
#include "mymoneyexception.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneybudget.h"
#include "mymoneyutils.h"
#include "payeeidentifier/payeeidentifierdata.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

// subclass QSqlQuery for performance tracing
MyMoneySqlQuery::MyMoneySqlQuery(MyMoneyStorageSql*  db)
    : QSqlQuery(*db)
{
}

MyMoneySqlQuery::~MyMoneySqlQuery()
{
}

bool MyMoneySqlQuery::exec()
{
  qDebug() << "start sql:" << lastQuery();
  bool rc = QSqlQuery::exec();
  qDebug() << "end sql:" << QSqlQuery::executedQuery();
  qDebug() << "***Query returned:" << rc << ", row count:" << numRowsAffected();
  return (rc);
}

bool MyMoneySqlQuery::exec(const QString & query)
{
  qDebug() << "start sql:" << query;
  bool rc = QSqlQuery::exec(query);
  qDebug() << "end sql:" << QSqlQuery::executedQuery();
  qDebug() << "***Query returned:" << rc << ", row count:" << numRowsAffected();
  return rc;
}

bool MyMoneySqlQuery::prepare(const QString & query)
{
  return (QSqlQuery::prepare(query));
}

//*****************************************************************************
MyMoneyDbTransaction::MyMoneyDbTransaction(MyMoneyStorageSql& db, const QString& name)
    : m_db(db), m_name(name)
{
  db.startCommitUnit(name);
}

MyMoneyDbTransaction::~MyMoneyDbTransaction()
{
  if (std::uncaught_exception()) {
    m_db.cancelCommitUnit(m_name);
  } else {
    m_db.endCommitUnit(m_name);
  }
}

//************************ Constructor/Destructor *****************************
MyMoneyStorageSql::MyMoneyStorageSql(IMyMoneySerialize *storage, const QUrl &url)
    : QSqlDatabase(QUrlQuery(url).queryItemValue("driver")),
    m_hiIdInstitutions(0),
    m_hiIdPayees(0),
    m_hiIdTags(0),
    m_hiIdAccounts(0),
    m_hiIdTransactions(0),
    m_hiIdSchedules(0),
    m_hiIdSecurities(0),
    m_hiIdReports(0),
    m_hiIdBudgets(0),
    m_hiIdOnlineJobs(0),
    m_hiIdPayeeIdentifier(0)
{
  m_storage = storage;

  m_storagePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
  m_dbVersion = 0;
  m_progressCallback = 0;
  m_displayStatus = false;
  m_readingPrices = false;
  m_newDatabase = false;
  m_loadAll = false;
  m_override = false;
  m_preferred.setReportAllSplits(false);
}
MyMoneyStorageSql::~MyMoneyStorageSql()
{
  try {
    close(true);
  } catch (const MyMoneyException& e) {
    qDebug() << "Caught Exception in MMStorageSql dtor: " << e.what();
  }
}

int MyMoneyStorageSql::open(const QUrl &url, int openMode, bool clear)
{
  try {
    int rc = 0;
    m_driver = MyMoneyDbDriver::create(QUrlQuery(url).queryItemValue("driver"));
    //get the input options
    QStringList options = QUrlQuery(url).queryItemValue("options").split(',');
    m_loadAll = options.contains("loadAll")/*|| m_mode == 0*/;
    m_override = options.contains("override");

    // create the database connection
    QString dbName = url.path();
    setDatabaseName(dbName);
    setHostName(url.host());
    setUserName(url.userName());
    setPassword(url.password());
    setConnectOptions("MYSQL_OPT_RECONNECT=1");
    switch (openMode) {
      case QIODevice::ReadOnly:    // OpenDatabase menu entry (or open last file)
      case QIODevice::ReadWrite:   // Save menu entry with database open
        // this may be a sqlite file opened from the recently used list
        // but which no longer exists. In that case, open will work but create an empty file.
        // This is not what the user's after; he may accuse KMM of deleting all his data!
        if (m_driver->requiresExternalFile()) {
          if (!fileExists(dbName)) {
            rc = 1;
            break;
          }
        }
        if (!QSqlDatabase::open()) {
          buildError(QSqlQuery(*this), Q_FUNC_INFO, "opening database");
          rc = 1;
        } else {
          rc = createTables(); // check all tables are present, create if not
        }
        break;
      case QIODevice::WriteOnly:   // SaveAs Database - if exists, must be empty, if not will create
        // Try to open the database.
        // If that fails, try to create the database, then try to open it again.
        m_newDatabase = true;
        if (!QSqlDatabase::open()) {
          if (!createDatabase(url)) {
            rc = 1;
          } else {
            if (!QSqlDatabase::open()) {
              buildError(QSqlQuery(*this), Q_FUNC_INFO, "opening new database");
              rc = 1;
            } else {
              rc = createTables();
            }
          }
        } else {
          rc = createTables();
          if (rc == 0) {
            if (clear) {
              clean();
            } else {
              rc = isEmpty();
            }
          }
        }
        break;
      default:
        qWarning("%s", qPrintable(QString("%1 - unknown open mode %2").arg(Q_FUNC_INFO).arg(openMode)));
    }
    if (rc != 0) return (rc);
    // bypass logon check if we are creating a database
    if (m_newDatabase) return(0);
    // check if the database is locked, if not lock it
    readFileInfo();
    if (!m_logonUser.isEmpty() && (!m_override)) {
      m_error = i18n("Database apparently in use\nOpened by %1 on %2 at %3.\nOpen anyway?",
                     m_logonUser,
                     m_logonAt.date().toString(Qt::ISODate),
                     m_logonAt.time().toString("hh.mm.ss"));
      qDebug("%s", qPrintable(m_error));
      close(false);
      rc = -1; // retryable error
    } else {
      m_logonUser = url.userName() + '@' + url.host();
      m_logonAt = QDateTime::currentDateTime();
      writeFileInfo();
    }
    return(rc);
  } catch (const QString& s) {
    qDebug("%s", qPrintable(s));
    return (1);
  }
}

void MyMoneyStorageSql::close(bool logoff)
{
  if (QSqlDatabase::isOpen()) {
    if (logoff) {
      MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
      m_logonUser.clear();
      writeFileInfo();
    }
    QSqlDatabase::close();
    QSqlDatabase::removeDatabase(connectionName());
  }
}

bool MyMoneyStorageSql::fileExists(const QString& dbName)
{
  QFile f(dbName);
  if (!f.exists()) {
    m_error = i18n("SQLite file %1 does not exist", dbName);
    return (false);
  }
  return (true);
}

bool MyMoneyStorageSql::createDatabase(const QUrl &url)
{
  int rc = true;
  if (!m_driver->requiresCreation()) return(true); // not needed for sqlite
  QString dbName = url.path().right(url.path().length() - 1); // remove separator slash
  if (!m_driver->canAutocreate()) {
    m_error = i18n("Automatic database creation for type %1 is not currently implemented.\n"
                   "Please create database %2 manually", driverName(), dbName);
    return (false);
  }
  // create the database (only works for mysql and postgre at present)
  { // for this code block, see QSqlDatabase API re removeDatabase
    QSqlDatabase maindb = QSqlDatabase::addDatabase(driverName(), "main");
    maindb.setDatabaseName(m_driver->defaultDbName());
    maindb.setHostName(url.host());
    maindb.setUserName(url.userName());
    maindb.setPassword(url.password());
    if (!maindb.open()) {
      throw MYMONEYEXCEPTION(QString("opening database %1 in function %2")
                             .arg(maindb.databaseName()).arg(Q_FUNC_INFO));
    } else {
      QSqlQuery qm(maindb);
      QString qs = m_driver->createDbString(dbName) + ';';
      if (!qm.exec(qs)) { // krazy:exclude=crashy
        buildError(qm, Q_FUNC_INFO,
                   i18n("Error in create database %1; do you have create permissions?", dbName), &maindb);
        rc = false;
      }
      maindb.close();
    }
  }
  QSqlDatabase::removeDatabase("main");
  return (rc);
}

int MyMoneyStorageSql::upgradeDb()
{
  //signalProgress(0, 1, QObject::tr("Upgrading database..."));
  QSqlQuery q(*this);
  q.prepare("SELECT version FROM kmmFileInfo;");
  if (!q.exec() || !q.next()) { // krazy:exclude=crashy
    if (!m_newDatabase) {
      buildError(q, Q_FUNC_INFO, "Error retrieving file info (version)");
      return(1);
    } else {
      m_dbVersion = m_db.currentVersion();
      m_storage->setFileFixVersion(m_storage->currentFixVersion());
      QSqlQuery q(*this);
      q.prepare("UPDATE kmmFileInfo SET version = :version, \
                fixLevel = :fixLevel;");
      q.bindValue(":version", m_dbVersion);
      q.bindValue(":fixLevel", m_storage->currentFixVersion());
      if (!q.exec()) { // krazy:exclude=crashy
        buildError(q, Q_FUNC_INFO, "Error updating file info(version)");
        return(1);
      }
      return (0);
    }
  }
  // prior to dbv6, 'version' format was 'dbversion.fixLevel+1'
  // as of dbv6, these are separate fields
  QString version = q.value(0).toString();
  if (version.contains('.')) {
    m_dbVersion = q.value(0).toString().section('.', 0, 0).toUInt();
    m_storage->setFileFixVersion(q.value(0).toString().section('.', 1, 1).toUInt() - 1);
  } else {
    m_dbVersion = version.toUInt();
    q.prepare("SELECT fixLevel FROM kmmFileInfo;");
    if (!q.exec() || !q.next()) { // krazy:exclude=crashy
      buildError(q, Q_FUNC_INFO, "Error retrieving file info (fixLevel)");
      return(1);
    }
    m_storage->setFileFixVersion(q.value(0).toUInt());
  }

  if (m_dbVersion == m_db.currentVersion())
    return 0;

  int rc = 0;

  // Drop VIEWs
  QStringList lowerTables = tables(QSql::AllTables);
  for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
    (*i) = (*i).toLower();
  }

  for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
    if (lowerTables.contains(tt.key().toLower())) {
      if (!q.exec("DROP VIEW " + tt.value().name() + ';')) // krazy:exclude=crashy
        throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("dropping view %1").arg(tt.key())));
    }
  }

  while ((m_dbVersion < m_db.currentVersion()) && (rc == 0)) {
    switch (m_dbVersion) {
      case 0:
        if ((rc = upgradeToV1()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 1:
        if ((rc = upgradeToV2()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 2:
        if ((rc = upgradeToV3()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 3:
        if ((rc = upgradeToV4()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 4:
        if ((rc = upgradeToV5()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 5:
        if ((rc = upgradeToV6()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 6:
        if ((rc = upgradeToV7()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 7:
        if ((rc = upgradeToV8()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 8:
        if ((rc = upgradeToV9()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 9:
        if ((rc = upgradeToV10()) != 0) return (1);
        ++m_dbVersion;
        break;
      case 10:
        if ((rc = upgradeToV11()) != 0) return (1);
        ++m_dbVersion;
        break;
      default:
        qWarning("Unknown version number in database - %d", m_dbVersion);
    }
  }

  // restore VIEWs
  lowerTables = tables(QSql::AllTables);
  for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
    (*i) = (*i).toLower();
  }

  for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
    if (!lowerTables.contains(tt.key().toLower())) {
      if (!q.exec(tt.value().createString())) // krazy:exclude=crashy
        throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO,
                                          QString("creating view %1").arg(tt.key())));
    }
  }

  // write updated version to DB
  //setVersion(QString("%1.%2").arg(m_dbVersion).arg(m_minorVersion))
  q.prepare(QString("UPDATE kmmFileInfo SET version = :version;"));
  q.bindValue(":version", m_dbVersion);
  if (!q.exec()) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "Error updating db version");
    return (1);
  }
  //signalProgress(-1,-1);
  return (0);
}

int MyMoneyStorageSql::upgradeToV1()
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  // change kmmSplits pkey to (transactionId, splitId)
  if (!q.exec("ALTER TABLE kmmSplits ADD PRIMARY KEY (transactionId, splitId);")) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "Error updating kmmSplits pkey");
    return (1);
  }
  // change kmmSplits alter checkNumber varchar(32)
  if (!q.exec(m_db.m_tables["kmmSplits"].modifyColumnString(m_driver, "checkNumber", // krazy:exclude=crashy
              MyMoneyDbColumn("checkNumber", "varchar(32)")))) {
    buildError(q, Q_FUNC_INFO, "Error expanding kmmSplits.checkNumber");
    return (1);
  }
  // change kmmSplits add postDate datetime
  if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
    return (1);
  // initialize it to same value as transaction (do it the long way round)
  q.prepare("SELECT id, postDate FROM kmmTransactions WHERE txType = 'N';");
  if (!q.exec()) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "Error priming kmmSplits.postDate");
    return (1);
  }
  QMap<QString, QDateTime> tids;
  while (q.next()) tids[q.value(0).toString()] = q.value(1).toDateTime();
  QMap<QString, QDateTime>::ConstIterator it;
  for (it = tids.constBegin(); it != tids.constEnd(); ++it) {
    q.prepare("UPDATE kmmSplits SET postDate=:postDate WHERE transactionId = :id;");
    q.bindValue(":postDate", it.value().toString(Qt::ISODate));
    q.bindValue(":id", it.key());
    if (!q.exec()) { // krazy:exclude=crashy
      buildError(q, Q_FUNC_INFO, "priming kmmSplits.postDate");
      return(1);
    }
  }
  // add index to kmmKeyValuePairs to (kvpType,kvpId)
  QStringList list;
  list << "kvpType" << "kvpId";
  if (!q.exec(MyMoneyDbIndex("kmmKeyValuePairs", "kmmKVPtype_id", list, false).generateDDL(m_driver) + ';')) {
    buildError(q, Q_FUNC_INFO, "Error adding kmmKeyValuePairs index");
    return (1);
  }
  // add index to kmmSplits to (accountId, txType)
  list.clear();
  list << "accountId" << "txType";
  if (!q.exec(MyMoneyDbIndex("kmmSplits", "kmmSplitsaccount_type", list, false).generateDDL(m_driver) + ';')) {
    buildError(q, Q_FUNC_INFO, "Error adding kmmSplits index");
    return (1);
  }
  // change kmmSchedulePaymentHistory pkey to (schedId, payDate)
  if (!q.exec("ALTER TABLE kmmSchedulePaymentHistory ADD PRIMARY KEY (schedId, payDate);")) {
    buildError(q, Q_FUNC_INFO, "Error updating kmmSchedulePaymentHistory pkey");
    return (1);
  }
  // change kmmPrices pkey to (fromId, toId, priceDate)
  if (!q.exec("ALTER TABLE kmmPrices ADD PRIMARY KEY (fromId, toId, priceDate);")) {
    buildError(q, Q_FUNC_INFO, "Error updating kmmPrices pkey");
    return (1);
  }
  // change kmmReportConfig pkey to (name)
  // There wasn't one previously, so no need to drop it.
  if (!q.exec("ALTER TABLE kmmReportConfig ADD PRIMARY KEY (name);")) {
    buildError(q, Q_FUNC_INFO, "Error updating kmmReportConfig pkey");
    return (1);
  }
  // change kmmFileInfo add budgets, hiBudgetId unsigned bigint
  // change kmmFileInfo add logonUser
  // change kmmFileInfo add logonAt datetime
  if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
    return (1);
  // change kmmAccounts add transactionCount unsigned bigint as last field
  if (!alterTable(m_db.m_tables["kmmAccounts"], m_dbVersion))
    return (1);
  // calculate the transaction counts. the application logic defines an account's tx count
  // in such a way as to count multiple splits in a tx which reference the same account as one.
  // this is the only way I can think of to do this which will work in sqlite too.
  // inefficient, but it only gets done once...
  // get a list of all accounts so we'll get a zero value for those without txs
  q.prepare("SELECT id FROM kmmAccounts");
  if (!q.exec()) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "Error retrieving accounts for transaction counting");
    return(1);
  }
  while (q.next()) {
    m_transactionCountMap[q.value(0).toString()] = 0;
  }
  q.prepare("SELECT accountId, transactionId FROM kmmSplits WHERE txType = 'N' ORDER BY 1, 2");
  if (!q.exec()) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "Error retrieving splits for transaction counting");
    return(1);
  }
  QString lastAcc, lastTx;
  while (q.next()) {
    QString thisAcc = q.value(0).toString();
    QString thisTx = q.value(1).toString();
    if ((thisAcc != lastAcc) || (thisTx != lastTx)) ++m_transactionCountMap[thisAcc];
    lastAcc = thisAcc;
    lastTx = thisTx;
  }
  QHash<QString, unsigned long>::ConstIterator itm;
  q.prepare("UPDATE kmmAccounts SET transactionCount = :txCount WHERE id = :id;");
  for (itm = m_transactionCountMap.constBegin(); itm != m_transactionCountMap.constEnd(); ++itm) {
    q.bindValue(":txCount", QString::number(itm.value()));
    q.bindValue(":id", itm.key());
    if (!q.exec()) { // krazy:exclude=crashy
      buildError(q, Q_FUNC_INFO, "Error updating transaction count");
      return (1);
    }
  }
  m_transactionCountMap.clear();
  return (0);
}

int MyMoneyStorageSql::upgradeToV2()
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  // change kmmSplits add price, priceFormatted fields
  if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
    return (1);
  return (0);
}

int MyMoneyStorageSql::upgradeToV3()
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  // kmmSchedules - add occurenceMultiplier
  // The default value is given here to populate the column.
  if (!q.exec("ALTER TABLE kmmSchedules ADD COLUMN " +
              MyMoneyDbIntColumn("occurenceMultiplier",
                                 MyMoneyDbIntColumn::SMALL, false, false, true)
              .generateDDL(m_driver) + " DEFAULT 0;")) {
    buildError(q, Q_FUNC_INFO, "Error adding kmmSchedules.occurenceMultiplier");
    return (1);
  }
  //The default is less than any useful value, so as each schedule is hit, it will update
  //itself to the appropriate value.
  return 0;
}

int MyMoneyStorageSql::upgradeToV4()
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  // kmmSplits - add index on transactionId + splitId
  QStringList list;
  list << "transactionId" << "splitId";
  if (!q.exec(MyMoneyDbIndex("kmmSplits", "kmmTx_Split", list, false).generateDDL(m_driver) + ';')) {
    buildError(q, Q_FUNC_INFO, "Error adding kmmSplits index on (transactionId, splitId)");
    return (1);
  }
  return 0;
}

int MyMoneyStorageSql::upgradeToV5()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  // kmmSplits - add bankId
  if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
    return (1);
  //kmmPayees - add columns "notes" "defaultAccountId" "matchData" "matchIgnoreCase" "matchKeys";
  if (!alterTable(m_db.m_tables["kmmPayees"], m_dbVersion))
    return (1);
  // kmmReportConfig - drop primary key on name since duplicate names are allowed
  if (!alterTable(m_db.m_tables["kmmReportConfig"], m_dbVersion))
    return (1);
  //}
  return 0;
}

int MyMoneyStorageSql::upgradeToV6()
{
  startCommitUnit(Q_FUNC_INFO);
  QSqlQuery q(*this);
  // kmmFileInfo - add fixLevel
  if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
    return (1);
  // upgrade Mysql to InnoDB transaction-safe engine
  // the following is not a good way to test for mysql - think of a better way
  if (!m_driver->tableOptionString().isEmpty()) {
    for (QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin(); tt != m_db.tableEnd(); ++tt) {
      if (!q.exec(QString("ALTER TABLE %1 ENGINE = InnoDB;").arg(tt.value().name()))) {
        buildError(q, Q_FUNC_INFO, "Error updating to InnoDB");
        return (1);
      }
    }
  }

  // the alterTable function really doesn't work too well
  // with adding a new column which is also to be primary key
  // so add the column first
  if (!q.exec("ALTER TABLE kmmReportConfig ADD COLUMN " +
              MyMoneyDbColumn("id", "varchar(32)").generateDDL(m_driver) + ';')) {
    buildError(q, Q_FUNC_INFO, "adding id to report table");
    return(1);
  }
  QMap<QString, MyMoneyReport> reportList = fetchReports();
  // the V5 database allowed lots of duplicate reports with no
  // way to distinguish between them. The fetchReports call
  // will have effectively removed all duplicates
  // so we now delete from the db and re-write them
  if (!q.exec("DELETE FROM kmmReportConfig;")) {
    buildError(q, Q_FUNC_INFO, "Error deleting reports");
    return (1);
  }
  // add unique id to reports table
  if (!alterTable(m_db.m_tables["kmmReportConfig"], m_dbVersion))
    return(1);

  QMap<QString, MyMoneyReport>::const_iterator it_r;
  for (it_r = reportList.constBegin(); it_r != reportList.constEnd(); ++it_r) {
    MyMoneyReport r = *it_r;
    q.prepare(m_db.m_tables["kmmReportConfig"].insertString());
    writeReport(*it_r, q);
  }

  m_storage->loadReportId(m_hiIdReports);
  endCommitUnit(Q_FUNC_INFO);
  return 0;
}

int MyMoneyStorageSql::upgradeToV7()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);

  // add tags support
  // kmmFileInfo - add tags and hiTagId
  if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
    return (1);

  m_tags = 0;
  return 0;
}

int MyMoneyStorageSql::upgradeToV8()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);

  // Added onlineJobs and payeeIdentifier
  if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
    return (1);

  return 0;
}

int MyMoneyStorageSql::upgradeToV9()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);

  QSqlQuery q(*this);
  // kmmSplits - add bankId
  if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
    return (1);

  return 0;
}

int MyMoneyStorageSql::upgradeToV10()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);

  QSqlQuery q(*this);
  if (!alterTable(m_db.m_tables["kmmPayeesPayeeIdentifier"], m_dbVersion))
    return (1);
  if (!alterTable(m_db.m_tables["kmmAccountsPayeeIdentifier"], m_dbVersion))
    return (1);

  return 0;
}

int MyMoneyStorageSql::upgradeToV11()
{
  MyMoneyDbTransaction dbtrans(*this, Q_FUNC_INFO);

  QSqlQuery q(*this);
  // add column roundingMethodCol to kmmSecurities
  if (!alterTable(m_db.m_tables["kmmSecurities"], m_dbVersion))
    return (1);
  // add column pricePrecision to kmmCurrencies
  if (!alterTable(m_db.m_tables["kmmCurrencies"], m_dbVersion))
    return (1);

  return 0;
}

bool MyMoneyStorageSql::alterTable(const MyMoneyDbTable& t, int fromVersion)
{
  const int toVersion = fromVersion + 1;

  QString tempTableName = t.name();
  tempTableName.replace("kmm", "kmmtmp");
  QSqlQuery q(*this);
  // drop primary key if it has one (and driver supports it)
  if (t.hasPrimaryKey(fromVersion)) {
    QString dropString = m_driver->dropPrimaryKeyString(t.name());
    if (!dropString.isEmpty()) {
      if (!q.exec(dropString)) {
        buildError(q, Q_FUNC_INFO, QString("Error dropping old primary key from %1").arg(t.name()));
        return false;
      }
    }
  }
  for (MyMoneyDbTable::index_iterator i = t.indexBegin(); i != t.indexEnd(); ++i) {
    QString indexName = t.name() + '_' + i->name() + "_idx";
    if (!q.exec(m_driver->dropIndexString(t.name(), indexName))) {
      buildError(q, Q_FUNC_INFO, QString("Error dropping index from %1").arg(t.name()));
      return false;
    }
  }
  if (!q.exec(QString("ALTER TABLE " + t.name() + " RENAME TO " + tempTableName + ';'))) {
    buildError(q, Q_FUNC_INFO, QString("Error renaming table %1").arg(t.name()));
    return false;
  }
  createTable(t, toVersion);
  if (getRecCount(tempTableName) > 0) {
    q.prepare(QString("INSERT INTO " + t.name() + " (" + t.columnList(fromVersion) +
                        ") SELECT " + t.columnList(fromVersion) + " FROM " + tempTableName + ';'));
    if (!q.exec()) { // krazy:exclude=crashy
        buildError(q, Q_FUNC_INFO, QString("Error inserting into new table %1").arg(t.name()));
        return false;
    }
  }
  if (!q.exec(QString("DROP TABLE " + tempTableName + ';'))) {
    buildError(q, Q_FUNC_INFO, QString("Error dropping old table %1").arg(t.name()));
    return false;
  }
  return true;
}

long unsigned MyMoneyStorageSql::getRecCount(const QString& table) const
{
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare(QString("SELECT COUNT(*) FROM %1;").arg(table));
  if ((!q.exec()) || (!q.next())) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "error retrieving record count");
    qFatal("Error retrieving record count"); // definitely shouldn't happen
  }
  return ((unsigned long) q.value(0).toULongLong());
}

int MyMoneyStorageSql::createTables()
{
  // check tables, create if required
  // convert everything to lower case, since SQL standard is case insensitive
  // table and column names (when not delimited), but some DBMSs disagree.
  QStringList lowerTables = tables(QSql::AllTables);
  for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
    (*i) = (*i).toLower();
  }

  for (QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin(); tt != m_db.tableEnd(); ++tt) {
    if (!lowerTables.contains(tt.key().toLower())) {
      createTable(tt.value());
    }
  }

  QSqlQuery q(*this);
  for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
    if (!lowerTables.contains(tt.key().toLower())) {
      if (!q.exec(tt.value().createString())) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("creating view %1").arg(tt.key())));
    }
  }

  // The columns to store version info changed with version 6. Prior versions are not supported here but an error is prevented and
  // an old behaviour is used: call upgradeDb().
  m_dbVersion = m_db.currentVersion();
  if (m_dbVersion >= 6) {
    q.prepare(QLatin1String("INSERT INTO kmmFileInfo (version, fixLevel) VALUES(?,?);"));
    q.bindValue(0, m_dbVersion);
    q.bindValue(1, m_storage->fileFixVersion());
    if (!q.exec())
      throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("Saving database version")));
  }

  return upgradeDb();
}

void MyMoneyStorageSql::createTable(const MyMoneyDbTable& t, int version)
{
// create the tables
  QStringList ql = t.generateCreateSQL(m_driver, version).split('\n', QString::SkipEmptyParts);
  QSqlQuery q(*this);
  foreach (const QString& i, ql) {
    if (!q.exec(i)) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("creating table/index %1").arg(t.name())));
  }
}

int MyMoneyStorageSql::isEmpty()
{
  // check all tables are empty
  QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin();
  int recordCount = 0;
  QSqlQuery q(*this);
  while ((tt != m_db.tableEnd()) && (recordCount == 0)) {
    q.prepare(QString("select count(*) from %1;").arg((*tt).name()));
    if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "getting record count")); // krazy:exclude=crashy
    if (!q.next()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "retrieving record count"));
    recordCount += q.value(0).toInt();
    ++tt;
  }

  if (recordCount != 0) {
    return (-1); // not empty
  } else {
    return (0);
  }
}

void MyMoneyStorageSql::clean()
{
// delete all existing records
  QMap<QString, MyMoneyDbTable>::ConstIterator it = m_db.tableBegin();
  QSqlQuery q(*this);
  while (it != m_db.tableEnd()) {
    q.prepare(QString("DELETE from %1;").arg(it.key()));
    if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("cleaning database"))); // krazy:exclude=crashy
    ++it;
  }
}

//////////////////////////////////////////////////////////////////

bool MyMoneyStorageSql::readFile()
{
  m_displayStatus = true;
  try {
    readFileInfo();
    readInstitutions();
    if (m_loadAll) {
      readPayees();
    } else {
      QList<QString> user;
      user.append(QString("USER"));
      readPayees(user);
    }
    readTags();
    readCurrencies();
    readSecurities();
    readAccounts();
    if (m_loadAll) {
      readTransactions();
    } else {
      if (m_preferred.filterSet().singleFilter.accountFilter) readTransactions(m_preferred);
    }
    readSchedules();
    readPrices();
    readReports();
    readBudgets();
    //FIXME - ?? if (m_mode == 0)
    //m_storage->rebuildAccountBalances();
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    m_storage->setLastModificationDate(m_storage->lastModificationDate());
    // FIXME?? if (m_mode == 0) m_storage = NULL;
    // make sure the progress bar is not shown any longer
    signalProgress(-1, -1);
    m_displayStatus = false;
    //MyMoneySqlQuery::traceOn();
    return true;
  } catch (const QString &) {
    return false;
  }
}

// The following is called from 'SaveAsDatabase'
bool MyMoneyStorageSql::writeFile()
{
  // initialize record counts and hi ids
  m_institutions = m_accounts = m_payees = m_tags = m_transactions = m_splits
                                = m_securities = m_prices = m_currencies = m_schedules  = m_reports = m_kvps = m_budgets = 0;
  m_hiIdInstitutions = m_hiIdPayees = m_hiIdTags = m_hiIdAccounts = m_hiIdTransactions =
                                        m_hiIdSchedules = m_hiIdSecurities = m_hiIdReports = m_hiIdBudgets = 0;
  m_onlineJobs = m_payeeIdentifier = 0;
  m_displayStatus = true;
  try {
    MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
    writeInstitutions();
    writePayees();
    writeTags();
    writeAccounts();
    writeTransactions();
    writeSchedules();
    writeSecurities();
    writePrices();
    writeCurrencies();
    writeReports();
    writeBudgets();
    writeOnlineJobs();
    writeFileInfo();
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    //m_storage->setLastModificationDate(m_storage->lastModificationDate());
    // FIXME?? if (m_mode == 0) m_storage = NULL;

    // make sure the progress bar is not shown any longer
    signalProgress(-1, -1);
    m_displayStatus = false;
    return true;
  } catch (const QString &) {
    return false;
  }
}

long unsigned MyMoneyStorageSql::highestNumberFromIdString(QString tableName, QString tableField, int prefixLength)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);

  if (!q.exec(m_driver->highestNumberFromIdString(tableName, tableField, prefixLength)) || !q.next())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("retrieving highest ID number"))); // krazy:exclude=crashy

  return q.value(0).toULongLong();
}

// --------------- SQL Transaction (commit unit) handling -----------------------------------
void MyMoneyStorageSql::startCommitUnit(const QString& callingFunction)
{
  if (m_commitUnitStack.isEmpty()) {
    if (!transaction()) throw MYMONEYEXCEPTION(buildError(QSqlQuery(), callingFunction, "starting commit unit") + ' ' + callingFunction);
  }
  m_commitUnitStack.push(callingFunction);
}

bool MyMoneyStorageSql::endCommitUnit(const QString& callingFunction)
{
  // for now, we don't know if there were any changes made to the data so
  // we expect the data to have changed. This assumption causes some unnecessary
  // repaints of the UI here and there, but for now it's ok. If we can determine
  // that the commit() really changes the data, we can return that information
  // as value of this method.
  bool rc = true;
  if (m_commitUnitStack.isEmpty()) {
    throw MYMONEYEXCEPTION("Empty commit unit stack while trying to commit");
  }

  if (callingFunction != m_commitUnitStack.top())
    qDebug("%s", qPrintable(QString("%1 - %2 s/be %3").arg(Q_FUNC_INFO).arg(callingFunction).arg(m_commitUnitStack.top())));
  m_commitUnitStack.pop();
  if (m_commitUnitStack.isEmpty()) {
    //qDebug() << "Committing with " << QSqlQuery::refCount() << " queries";
    if (!commit()) throw MYMONEYEXCEPTION(buildError(QSqlQuery(), callingFunction, "ending commit unit"));
  }
  return rc;
}

void MyMoneyStorageSql::cancelCommitUnit(const QString& callingFunction)
{
  if (m_commitUnitStack.isEmpty()) return;
  if (callingFunction != m_commitUnitStack.top())
    qDebug("%s", qPrintable(QString("%1 - %2 s/be %3").arg(Q_FUNC_INFO).arg(callingFunction).arg(m_commitUnitStack.top())));
  m_commitUnitStack.clear();
  if (!rollback()) throw MYMONEYEXCEPTION(buildError(QSqlQuery(), callingFunction, "cancelling commit unit") + ' ' + callingFunction);
}

/////////////////////////////////////////////////////////////////////
void MyMoneyStorageSql::fillStorage()
{
//  if (!m_transactionListRead)  // make sure we have loaded everything
  readTransactions();
//  if (!m_payeeListRead)
  readPayees();
}

//------------------------------ Write SQL routines ----------------------------------------
// **** Institutions ****
void MyMoneyStorageSql::writeInstitutions()
{
  // first, get a list of what's on the database
  // anything not in the list needs to be inserted
  // anything which is will be updated and removed from the list
  // anything left over at the end will need to be deleted
  // this is an expensive and inconvenient way to do things; find a better way
  // one way would be to build the lists when reading the db
  // unfortunately this object does not persist between read and write
  // it would also be nice if we could tell which objects had been updated since we read them in
  QList<QString> dbList;
  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmInstitutions;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Institution list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  const QList<MyMoneyInstitution> list = m_storage->institutionList();
  QList<MyMoneyInstitution> insertList;
  QList<MyMoneyInstitution> updateList;
  QSqlQuery q2(*this);
  q.prepare(m_db.m_tables["kmmInstitutions"].updateString());
  q2.prepare(m_db.m_tables["kmmInstitutions"].insertString());
  signalProgress(0, list.count(), "Writing Institutions...");
  foreach (const MyMoneyInstitution& i, list) {
    if (dbList.contains(i.id())) {
      dbList.removeAll(i.id());
      updateList << i;
    } else {
      insertList << i;
    }
    signalProgress(++m_institutions, 0);
  }
  if (!insertList.isEmpty())
    writeInstitutionList(insertList, q2);

  if (!updateList.isEmpty())
    writeInstitutionList(updateList, q);

  if (!dbList.isEmpty()) {
    QVariantList deleteList;
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      deleteList << it;
    }
    q.prepare("DELETE FROM kmmInstitutions WHERE id = :id");
    q.bindValue(":id", deleteList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Institution"));

    deleteKeyValuePairs("OFXSETTINGS", deleteList);
  }
}

void MyMoneyStorageSql::addInstitution(const MyMoneyInstitution& inst)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmInstitutions"].insertString());
  QList<MyMoneyInstitution> iList;
  iList << inst;
  writeInstitutionList(iList , q);
  ++m_institutions;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyInstitution(const MyMoneyInstitution& inst)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmInstitutions"].updateString());
  QVariantList kvpList;
  kvpList << inst.id();
  deleteKeyValuePairs("OFXSETTINGS", kvpList);
  QList<MyMoneyInstitution> iList;
  iList << inst;
  writeInstitutionList(iList , q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeInstitution(const MyMoneyInstitution& inst)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << inst.id();
  deleteKeyValuePairs("OFXSETTINGS", kvpList);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmInstitutions"].deleteString());
  q.bindValue(":id", inst.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting  Institution"))); // krazy:exclude=crashy
  --m_institutions;
  writeFileInfo();
}

void MyMoneyStorageSql::writeInstitutionList(const QList<MyMoneyInstitution>& iList, QSqlQuery& q)
{
  QVariantList idList;
  QVariantList nameList;
  QVariantList managerList;
  QVariantList routingCodeList;
  QVariantList addressStreetList;
  QVariantList addressCityList;
  QVariantList addressZipcodeList;
  QVariantList telephoneList;
  QList<QMap<QString, QString> > kvpPairsList;

  foreach (const MyMoneyInstitution& i, iList) {
    idList << i.id();
    nameList << i.name();
    managerList << i.manager();
    routingCodeList << i.sortcode();
    addressStreetList << i.street();
    addressCityList << i.city();
    addressZipcodeList << i.postcode();
    telephoneList << i.telephone();
    kvpPairsList << i.pairs();
  }

  q.bindValue(":id", idList);
  q.bindValue(":name", nameList);
  q.bindValue(":manager", managerList);
  q.bindValue(":routingCode", routingCodeList);
  q.bindValue(":addressStreet", addressStreetList);
  q.bindValue(":addressCity", addressCityList);
  q.bindValue(":addressZipcode", addressZipcodeList);
  q.bindValue(":telephone", telephoneList);

  if (!q.execBatch())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Institution")));
  writeKeyValuePairs("OFXSETTINGS", idList, kvpPairsList);
  // Set m_hiIdInstitutions to 0 to force recalculation the next time it is requested
  m_hiIdInstitutions = 0;
}

void MyMoneyStorageSql::writePayees()
{
  // first, get a list of what's on the database (see writeInstitutions)

  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmPayees;");
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Payee list")); // krazy:exclude=crashy

  QList<QString> dbList;
  dbList.reserve(q.numRowsAffected());
  while (q.next())
    dbList.append(q.value(0).toString());

  QList<MyMoneyPayee> list = m_storage->payeeList();
  MyMoneyPayee user(QString("USER"), m_storage->user());
  list.prepend(user);
  signalProgress(0, list.count(), "Writing Payees...");

  Q_FOREACH(const MyMoneyPayee& it, list) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      modifyPayee(it);
    } else {
      addPayee(it);
    }
    signalProgress(++m_payees, 0);
  }

  if (!dbList.isEmpty()) {
    QMap<QString, MyMoneyPayee> payeesToDelete = fetchPayees(dbList, true);
    Q_FOREACH(const MyMoneyPayee& payee, payeesToDelete) {
      removePayee(payee);
    }
  }
}

void MyMoneyStorageSql::addPayee(const MyMoneyPayee& payee)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPayees"].insertString());
  writePayee(payee, q);
  ++m_payees;

  QVariantList identIds;
  QList<payeeIdentifier> idents = payee.payeeIdentifiers();
  // Store ids which have to be stored in the map table
  identIds.reserve(idents.count());
  foreach (payeeIdentifier ident, idents) {
      try {
        // note: this changes ident
        addPayeeIdentifier(ident);
      identIds.append(ident.idString());
    } catch (payeeIdentifier::empty&) {
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
    q.prepare("INSERT INTO kmmPayeesPayeeIdentifier (payeeId, identifierId, userOrder) VALUES(?, ?, ?)");
    q.bindValue(0, payeeIdList);
    q.bindValue(1, identIds);
    q.bindValue(2, order);
    if (!q.execBatch())
      throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing payee's identifiers"))); // krazy:exclude=crashy
  }

  writeFileInfo();
}

void MyMoneyStorageSql::modifyPayee(MyMoneyPayee payee)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPayees"].updateString());
  writePayee(payee, q);

  // Get a list of old identifiers first
  q.prepare("SELECT identifierId FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  q.bindValue(0, payee.id());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("modifying payee's identifiers (getting old values failed)"))); // krazy:exclude=crashy

  QStringList oldIdentIds;
  oldIdentIds.reserve(q.numRowsAffected());
  while (q.next())
    oldIdentIds << q.value(0).toString();

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
  q.prepare("DELETE FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  q.bindValue(0, payee.id());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("modifying payee's identifiers (delete from mapping table)"))); // krazy:exclude=crashy

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

  q.prepare("INSERT INTO kmmPayeesPayeeIdentifier (payeeId, userOrder, identifierId) VALUES(?, ?, ?)");
  q.bindValue(0, payeeIdList);
  q.bindValue(1, order);
  q.bindValue(2, identIdList);
  if (!q.execBatch())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing payee's identifiers during modify"))); // krazy:exclude=crashy

  writeFileInfo();
}

void MyMoneyStorageSql::modifyUserInfo(const MyMoneyPayee& payee)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPayees"].updateString());
  writePayee(payee, q, true);
  writeFileInfo();
}

void MyMoneyStorageSql::removePayee(const MyMoneyPayee& payee)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);

  // Get identifiers first so we know which to delete
  q.prepare("SELECT identifierId FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  q.bindValue(0, payee.id());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("removing payee's identifiers (getting old values failed)"))); // krazy:exclude=crashy

  QStringList identIds;
  while (q.next())
    identIds << q.value(0).toString();

  QMap<QString, payeeIdentifier> idents = fetchPayeeIdentifiers(identIds);
  foreach (payeeIdentifier ident, idents) {
    removePayeeIdentifier(ident);
  }

  // Delete entries from mapping table
  q.prepare("DELETE FROM kmmPayeesPayeeIdentifier WHERE payeeId = ?");
  q.bindValue(0, payee.id());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("removing payee's identifiers (delete from mapping table)"))); // krazy:exclude=crashy

  // Delete payee
  q.prepare(m_db.m_tables["kmmPayees"].deleteString());
  q.bindValue(":id", payee.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting  Payee"))); // krazy:exclude=crashy
  --m_payees;

  writeFileInfo();
}

void MyMoneyStorageSql::writePayee(const MyMoneyPayee& p, QSqlQuery& q, bool isUserInfo)
{
  if (isUserInfo) {
    q.bindValue(":id", "USER");
  } else {
    q.bindValue(":id", p.id());
  }
  q.bindValue(":name", p.name());
  q.bindValue(":reference", p.reference());
  q.bindValue(":email", p.email());
  q.bindValue(":addressStreet", p.address());
  q.bindValue(":addressCity", p.city());
  q.bindValue(":addressZipcode", p.postcode());
  q.bindValue(":addressState", p.state());
  q.bindValue(":telephone", p.telephone());
  q.bindValue(":notes", p.notes());
  q.bindValue(":defaultAccountId", p.defaultAccountId());
  bool ignoreCase;
  QString matchKeys;
  MyMoneyPayee::payeeMatchType type = p.matchData(ignoreCase, matchKeys);
  q.bindValue(":matchData", static_cast<unsigned int>(type));

  if (ignoreCase)
    q.bindValue(":matchIgnoreCase", "Y");
  else
    q.bindValue(":matchIgnoreCase", "N");

  q.bindValue(":matchKeys", matchKeys);
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Payee"))); // krazy:exclude=crashy

  if (!isUserInfo)
    m_hiIdPayees = 0;
}

// **** Tags ****
void MyMoneyStorageSql::writeTags()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmTags;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Tag list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  QList<MyMoneyTag> list = m_storage->tagList();
  signalProgress(0, list.count(), "Writing Tags...");
  QSqlQuery q2(*this);
  q.prepare(m_db.m_tables["kmmTags"].updateString());
  q2.prepare(m_db.m_tables["kmmTags"].insertString());
  foreach (const MyMoneyTag& it, list) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      writeTag(it, q);
    } else {
      writeTag(it, q2);
    }
    signalProgress(++m_tags, 0);
  }

  if (!dbList.isEmpty()) {
    QVariantList deleteList;
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      deleteList << it;
    }
    q.prepare(m_db.m_tables["kmmTags"].deleteString());
    q.bindValue(":id", deleteList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Tag"));
    m_tags -= q.numRowsAffected();
  }
}

void MyMoneyStorageSql::addTag(const MyMoneyTag& tag)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmTags"].insertString());
  writeTag(tag, q);
  ++m_tags;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyTag(const MyMoneyTag& tag)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmTags"].updateString());
  writeTag(tag, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeTag(const MyMoneyTag& tag)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmTags"].deleteString());
  q.bindValue(":id", tag.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting  Tag"))); // krazy:exclude=crashy
  --m_tags;
  writeFileInfo();
}

void MyMoneyStorageSql::writeTag(const MyMoneyTag& ta, QSqlQuery& q)
{
  q.bindValue(":id", ta.id());
  q.bindValue(":name", ta.name());
  q.bindValue(":tagColor", ta.tagColor().name());
  if (ta.isClosed()) q.bindValue(":closed", "Y");
  else q.bindValue(":closed", "N");
  q.bindValue(":notes", ta.notes());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Tag"))); // krazy:exclude=crashy
  m_hiIdTags = 0;
}

// **** Accounts ****
void MyMoneyStorageSql::writeAccounts()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmAccounts;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Account list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  QList<MyMoneyAccount> list;
  m_storage->accountList(list);
  unsigned progress = 0;
  signalProgress(0, list.count(), "Writing Accounts...");
  if (dbList.isEmpty()) { // new table, insert standard accounts
    q.prepare(m_db.m_tables["kmmAccounts"].insertString());
  } else {
    q.prepare(m_db.m_tables["kmmAccounts"].updateString());
  }
  // Attempt to write the standard accounts. For an empty db, this will fail.
  try {
    QList<MyMoneyAccount> stdList;
    stdList << m_storage->asset();
    stdList << m_storage->liability();
    stdList << m_storage->expense();
    stdList << m_storage->income();
    stdList << m_storage->equity();
    writeAccountList(stdList, q);
    m_accounts += stdList.size();
  } catch (const MyMoneyException &) {
    // If the above failed, assume that the database is empty and create
    // the standard accounts by hand before writing them.
    MyMoneyAccount acc_l;
    acc_l.setAccountType(Account::Liability);
    acc_l.setName("Liability");
    MyMoneyAccount liability(STD_ACC_LIABILITY, acc_l);

    MyMoneyAccount acc_a;
    acc_a.setAccountType(Account::Asset);
    acc_a.setName("Asset");
    MyMoneyAccount asset(STD_ACC_ASSET, acc_a);

    MyMoneyAccount acc_e;
    acc_e.setAccountType(Account::Expense);
    acc_e.setName("Expense");
    MyMoneyAccount expense(STD_ACC_EXPENSE, acc_e);

    MyMoneyAccount acc_i;
    acc_i.setAccountType(Account::Income);
    acc_i.setName("Income");
    MyMoneyAccount income(STD_ACC_INCOME, acc_i);

    MyMoneyAccount acc_q;
    acc_q.setAccountType(Account::Equity);
    acc_q.setName("Equity");
    MyMoneyAccount equity(STD_ACC_EQUITY, acc_q);

    QList<MyMoneyAccount> stdList;
    stdList << asset;
    stdList << liability;
    stdList << expense;
    stdList << income;
    stdList << equity;
    writeAccountList(stdList, q);
    m_accounts += stdList.size();
  }

  QSqlQuery q2(*this);
  q.prepare(m_db.m_tables["kmmAccounts"].updateString());
  q2.prepare(m_db.m_tables["kmmAccounts"].insertString());
  QList<MyMoneyAccount> updateList;
  QList<MyMoneyAccount> insertList;
  // Update the accounts that exist; insert the ones that do not.
  foreach (const MyMoneyAccount& it, list) {
    m_transactionCountMap[it.id()] = m_storagePtr->transactionCount(it.id());
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      updateList << it;
    } else {
      insertList << it;
    }
    signalProgress(++progress, 0);
    ++m_accounts;
  }

  writeAccountList(updateList, q);
  writeAccountList(insertList, q2);

  // Delete the accounts that are in the db but no longer in memory.
  if (!dbList.isEmpty()) {
    QVariantList kvpList;

    q.prepare("DELETE FROM kmmAccounts WHERE id = :id");
    foreach (const QString& it, dbList) {
      if (!m_storagePtr->isStandardAccount(it)) {
        kvpList << it;
      }
    }
    q.bindValue(":id", kvpList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Account"));

    deleteKeyValuePairs("ACCOUNT", kvpList);
    deleteKeyValuePairs("ONLINEBANKING", kvpList);
  }
}

void MyMoneyStorageSql::addAccount(const MyMoneyAccount& acc)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmAccounts"].insertString());
  QList<MyMoneyAccount> aList;
  aList << acc;
  writeAccountList(aList, q);
  ++m_accounts;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyAccount(const MyMoneyAccount& acc)
{
  QList<MyMoneyAccount> aList;
  aList << acc;
  modifyAccountList(aList);
}

void MyMoneyStorageSql::modifyAccountList(const QList<MyMoneyAccount>& acc)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmAccounts"].updateString());
  QVariantList kvpList;
  foreach (const MyMoneyAccount& a, acc) {
    kvpList << a.id();
  }
  deleteKeyValuePairs("ACCOUNT", kvpList);
  deleteKeyValuePairs("ONLINEBANKING", kvpList);
  writeAccountList(acc, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeAccount(const MyMoneyAccount& acc)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << acc.id();
  deleteKeyValuePairs("ACCOUNT", kvpList);
  deleteKeyValuePairs("ONLINEBANKING", kvpList);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmAccounts"].deleteString());
  q.bindValue(":id", acc.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Account"))); // krazy:exclude=crashy
  --m_accounts;
  writeFileInfo();
}

void MyMoneyStorageSql::writeAccountList(const QList<MyMoneyAccount>& accList, QSqlQuery& q)
{
  //MyMoneyMoney balance = m_storagePtr->balance(acc.id(), QDate());

  QVariantList idList;
  QVariantList institutionIdList;
  QVariantList parentIdList;
  QVariantList lastReconciledList;
  QVariantList lastModifiedList;
  QVariantList openingDateList;
  QVariantList accountNumberList;
  QVariantList accountTypeList;
  QVariantList accountTypeStringList;
  QVariantList isStockAccountList;
  QVariantList accountNameList;
  QVariantList descriptionList;
  QVariantList currencyIdList;
  QVariantList balanceList;
  QVariantList balanceFormattedList;
  QVariantList transactionCountList;

  QList<QMap<QString, QString> > pairs;
  QList<QMap<QString, QString> > onlineBankingPairs;

  foreach (const MyMoneyAccount& a, accList) {
    idList << a.id();
    institutionIdList << a.institutionId();
    parentIdList << a.parentAccountId();
    if (a.lastReconciliationDate() == QDate())
      lastReconciledList << a.lastReconciliationDate();
    else
      lastReconciledList << a.lastReconciliationDate().toString(Qt::ISODate);
    lastModifiedList << a.lastModified();
    if (a.openingDate() == QDate())
      openingDateList << a.openingDate();
    else
      openingDateList << a.openingDate().toString(Qt::ISODate);
    accountNumberList << a.number();
    accountTypeList << (int)a.accountType();
    accountTypeStringList << MyMoneyAccount::accountTypeToString(a.accountType());
    if (a.accountType() == Account::Stock)
      isStockAccountList << "Y";
    else
      isStockAccountList << "N";
    accountNameList << a.name();
    descriptionList << a.description();
    currencyIdList << a.currencyId();
    // This section attempts to get the balance from the database, if possible
    // That way, the balance fields are kept in sync. If that fails, then
    // It is assumed that the account actually knows its correct balance.

    //FIXME: Using exceptions for branching always feels like a kludge.
    //       Look for a better way.
    try {
      MyMoneyMoney bal = m_storagePtr->balance(a.id(), QDate());
      balanceList << bal.toString();
      balanceFormattedList << bal.formatMoney("", -1, false);
    } catch (const MyMoneyException &) {
      balanceList << a.balance().toString();
      balanceFormattedList << a.balance().formatMoney("", -1, false);
    }
    transactionCountList << quint64(m_transactionCountMap[a.id()]);

    //MMAccount inherits from KVPContainer AND has a KVPContainer member
    //so handle both
    pairs << a.pairs();
    onlineBankingPairs << a.onlineBankingSettings().pairs();
  }

  q.bindValue(":id", idList);
  q.bindValue(":institutionId", institutionIdList);
  q.bindValue(":parentId", parentIdList);
  q.bindValue(":lastReconciled", lastReconciledList);
  q.bindValue(":lastModified", lastModifiedList);
  q.bindValue(":openingDate", openingDateList);
  q.bindValue(":accountNumber", accountNumberList);
  q.bindValue(":accountType", accountTypeList);
  q.bindValue(":accountTypeString", accountTypeStringList);
  q.bindValue(":isStockAccount", isStockAccountList);
  q.bindValue(":accountName", accountNameList);
  q.bindValue(":description", descriptionList);
  q.bindValue(":currencyId", currencyIdList);
  q.bindValue(":balance", balanceList);
  q.bindValue(":balanceFormatted", balanceFormattedList);
  q.bindValue(":transactionCount", transactionCountList);

  if (!q.execBatch())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Account")));

  //Add in Key-Value Pairs for accounts.
  writeKeyValuePairs("ACCOUNT", idList, pairs);
  writeKeyValuePairs("ONLINEBANKING", idList, onlineBankingPairs);
  m_hiIdAccounts = 0;
}

// **** Transactions and Splits ****
void MyMoneyStorageSql::writeTransactions()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmTransactions WHERE txType = 'N';");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Transaction list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QList<MyMoneyTransaction> list;
  m_storage->transactionList(list, filter);
  signalProgress(0, list.count(), "Writing Transactions...");
  QList<MyMoneyTransaction>::ConstIterator it;
  QSqlQuery q2(*this);
  q.prepare(m_db.m_tables["kmmTransactions"].updateString());
  q2.prepare(m_db.m_tables["kmmTransactions"].insertString());
  foreach (const MyMoneyTransaction& it, list) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      writeTransaction(it.id(), it, q, "N");
    } else {
      writeTransaction(it.id(), it, q2, "N");
    }
    signalProgress(++m_transactions, 0);
  }

  if (!dbList.isEmpty()) {
    foreach (const QString& it, dbList) {
      deleteTransaction(it);
    }
  }
}

void MyMoneyStorageSql::addTransaction(const MyMoneyTransaction& tx)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  // add the transaction and splits
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmTransactions"].insertString());
  writeTransaction(tx.id(), tx, q, "N");
  ++m_transactions;
  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = m_storagePtr->account(it_s.accountId());
    ++m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  // in the fileinfo record, update lastMod, txCount, next TxId
  writeFileInfo();
}

void MyMoneyStorageSql::modifyTransaction(const MyMoneyTransaction& tx)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  // remove the splits of the old tx from the count table
  QSqlQuery q(*this);
  q.prepare("SELECT accountId FROM kmmSplits WHERE transactionId = :txId;");
  q.bindValue(":txId", tx.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "retrieving old splits"));
  while (q.next()) {
    QString id = q.value(0).toString();
    --m_transactionCountMap[id];
  }
  // add the transaction and splits
  q.prepare(m_db.m_tables["kmmTransactions"].updateString());
  writeTransaction(tx.id(), tx, q, "N");
  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = m_storagePtr->account(it_s.accountId());
    ++m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  //writeSplits(tx.id(), "N", tx.splits());
  // in the fileinfo record, update lastMod
  writeFileInfo();
}

void MyMoneyStorageSql::removeTransaction(const MyMoneyTransaction& tx)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  deleteTransaction(tx.id());
  --m_transactions;

  QList<MyMoneyAccount> aList;
  // for each split account, update lastMod date, balance, txCount
  foreach (const MyMoneySplit& it_s, tx.splits()) {
    MyMoneyAccount acc = m_storagePtr->account(it_s.accountId());
    --m_transactionCountMap[acc.id()];
    aList << acc;
  }
  modifyAccountList(aList);
  // in the fileinfo record, update lastModDate, txCount
  writeFileInfo();
}

void MyMoneyStorageSql::deleteTransaction(const QString& id)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  QVariantList idList;
  idList << id;
  q.prepare("DELETE FROM kmmSplits WHERE transactionId = :transactionId;");
  q.bindValue(":transactionId", idList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Splits"));

  q.prepare("DELETE FROM kmmKeyValuePairs WHERE kvpType = 'SPLIT' "
            "AND kvpId LIKE '?%'");
  q.bindValue(1, idList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Splits KVP"));

  m_splits -= q.numRowsAffected();
  deleteKeyValuePairs("TRANSACTION", idList);
  q.prepare(m_db.m_tables["kmmTransactions"].deleteString());
  q.bindValue(":id", idList);
  if (!q.execBatch())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Transaction"));
}

void MyMoneyStorageSql::writeTransaction(const QString& txId, const MyMoneyTransaction& tx, QSqlQuery& q, const QString& type)
{
  q.bindValue(":id", txId);
  q.bindValue(":txType", type);
  q.bindValue(":postDate", tx.postDate().toString(Qt::ISODate));
  q.bindValue(":memo", tx.memo());
  q.bindValue(":entryDate", tx.entryDate().toString(Qt::ISODate));
  q.bindValue(":currencyId", tx.commodity());
  q.bindValue(":bankId", tx.bankID());

  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Transaction"))); // krazy:exclude=crashy

  m_txPostDate = tx.postDate(); // FIXME: TEMP till Tom puts date in split object
  QList<MyMoneySplit> splitList = tx.splits();
  writeSplits(txId, type, splitList);

  //Add in Key-Value Pairs for transactions.
  QVariantList idList;
  idList << txId;
  deleteKeyValuePairs("TRANSACTION", idList);
  QList<QMap<QString, QString> > pairs;
  pairs << tx.pairs();
  writeKeyValuePairs("TRANSACTION", idList, pairs);
  m_hiIdTransactions = 0;
}

void MyMoneyStorageSql::writeSplits(const QString& txId, const QString& type, const QList<MyMoneySplit>& splitList)
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<unsigned int> dbList;
  QList<MyMoneySplit> insertList;
  QList<MyMoneySplit> updateList;
  QList<int> insertIdList;
  QList<int> updateIdList;
  QSqlQuery q(*this);
  q.prepare("SELECT splitId FROM kmmSplits where transactionId = :id;");
  q.bindValue(":id", txId);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Split list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toUInt());

  QList<MyMoneySplit>::ConstIterator it;
  unsigned int i = 0;
  QSqlQuery q2(*this);
  q.prepare(m_db.m_tables["kmmSplits"].updateString());
  q2.prepare(m_db.m_tables["kmmSplits"].insertString());
  for (it = splitList.constBegin(), i = 0; it != splitList.constEnd(); ++it, ++i) {
    if (dbList.contains(i)) {
      dbList.removeAll(i);
      updateList << *it;
      updateIdList << i;
    } else {
      ++m_splits;
      insertList << *it;
      insertIdList << i;
    }
  }

  if (!insertList.isEmpty()) {
    writeSplitList(txId, insertList, type, insertIdList, q2);
    writeTagSplitsList(txId, insertList, insertIdList);
  }

  if (!updateList.isEmpty()) {
    writeSplitList(txId, updateList, type, updateIdList, q);
    deleteTagSplitsList(txId, updateIdList);
    writeTagSplitsList(txId, updateList, updateIdList);
  }

  if (!dbList.isEmpty()) {
    QVector<QVariant> txIdList(dbList.count(), txId);
    QVariantList splitIdList;
    q.prepare("DELETE FROM kmmSplits WHERE transactionId = :txId AND splitId = :splitId");
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (int it, dbList) {
      splitIdList << it;
    }
    q.bindValue(":txId", txIdList.toList());
    q.bindValue(":splitId", splitIdList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Splits"));
  }
}

void MyMoneyStorageSql::deleteTagSplitsList(const QString& txId, const QList<int>& splitIdList)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList iList;
  QVariantList transactionIdList;

  // qCopy segfaults here, so do it with a hand-rolled loop
  foreach (int it_s, splitIdList) {
    iList << it_s;
    transactionIdList << txId;
  }
  QSqlQuery q(*this);
  q.prepare("DELETE FROM kmmTagSplits WHERE transactionId = :transactionId AND splitId = :splitId");
  q.bindValue(":splitId", iList);
  q.bindValue(":transactionId", transactionIdList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting tagSplits")));
}

void MyMoneyStorageSql::writeTagSplitsList
(const QString& txId,
 const QList<MyMoneySplit>& splitList,
 const QList<int>& splitIdList)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList tagIdList;
  QVariantList txIdList;
  QVariantList splitIdList_TagSplits;
  QVariantList tagSplitsIdList;

  int i = 0, l = 0;
  foreach (const MyMoneySplit& s, splitList) {
    for (l = 0; l < s.tagIdList().size(); ++l) {
      tagIdList << s.tagIdList()[l];
      splitIdList_TagSplits << splitIdList[i];
      txIdList << txId;
    }
    i++;
  }
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmTagSplits"].insertString());
  q.bindValue(":tagId", tagIdList);
  q.bindValue(":splitId", splitIdList_TagSplits);
  q.bindValue(":transactionId", txIdList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing tagSplits")));
}

void MyMoneyStorageSql::writeSplitList
(const QString& txId,
 const QList<MyMoneySplit>& splitList,
 const QString& type,
 const QList<int>& splitIdList,
 QSqlQuery& q)
{
  QVariantList txIdList;
  QVariantList typeList;
  QVariantList payeeIdList;
  QVariantList reconcileDateList;
  QVariantList actionList;
  QVariantList reconcileFlagList;
  QVariantList valueList;
  QVariantList valueFormattedList;
  QVariantList sharesList;
  QVariantList sharesFormattedList;
  QVariantList priceList;
  QVariantList priceFormattedList;
  QVariantList memoList;
  QVariantList accountIdList;
  QVariantList costCenterIdList;
  QVariantList checkNumberList;
  QVariantList postDateList;
  QVariantList bankIdList;
  QVariantList kvpIdList;
  QList<QMap<QString, QString> > kvpPairsList;

  int i = 0;
  foreach (const MyMoneySplit& s, splitList) {
    txIdList << txId;
    typeList << type;
    payeeIdList << s.payeeId();
    if (s.reconcileDate() == QDate())
      reconcileDateList << s.reconcileDate();
    else
      reconcileDateList << s.reconcileDate().toString(Qt::ISODate);
    actionList << s.action();
    reconcileFlagList << (int)s.reconcileFlag();
    valueList << s.value().toString();
    valueFormattedList << s.value().formatMoney("", -1, false).replace(QChar(','), QChar('.'));
    sharesList << s.shares().toString();
    MyMoneyAccount acc = m_storagePtr->account(s.accountId());
    MyMoneySecurity sec = m_storagePtr->security(acc.currencyId());
    sharesFormattedList << s.price().
    formatMoney("", MyMoneyMoney::denomToPrec(sec.smallestAccountFraction()), false).
    replace(QChar(','), QChar('.'));
    MyMoneyMoney price = s.actualPrice();
    if (!price.isZero()) {
      priceList << price.toString();
      priceFormattedList << price.formatMoney
      ("", sec.pricePrecision(), false)
      .replace(QChar(','), QChar('.'));
    } else {
      priceList << QString();
      priceFormattedList << QString();
    }
    memoList << s.memo();
    accountIdList << s.accountId();
    costCenterIdList << s.costCenterId();
    checkNumberList << s.number();
    postDateList << m_txPostDate.toString(Qt::ISODate); // FIXME: when Tom puts date into split object
    bankIdList << s.bankID();

    kvpIdList << QString(txId + QString::number(splitIdList[i]));
    kvpPairsList << s.pairs();
    ++i;
  }

  q.bindValue(":transactionId", txIdList);
  q.bindValue(":txType", typeList);
  QVariantList iList;
  // qCopy segfaults here, so do it with a hand-rolled loop
  foreach (int it_s, splitIdList) {
    iList << it_s;
  }

  q.bindValue(":splitId", iList);
  q.bindValue(":payeeId", payeeIdList);
  q.bindValue(":reconcileDate", reconcileDateList);
  q.bindValue(":action", actionList);
  q.bindValue(":reconcileFlag", reconcileFlagList);
  q.bindValue(":value", valueList);
  q.bindValue(":valueFormatted", valueFormattedList);
  q.bindValue(":shares", sharesList);
  q.bindValue(":sharesFormatted", sharesFormattedList);
  q.bindValue(":price", priceList);
  q.bindValue(":priceFormatted", priceFormattedList);
  q.bindValue(":memo", memoList);
  q.bindValue(":accountId", accountIdList);
  q.bindValue(":costCenterId", costCenterIdList);
  q.bindValue(":checkNumber", checkNumberList);
  q.bindValue(":postDate", postDateList);
  q.bindValue(":bankId", bankIdList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Split")));
  deleteKeyValuePairs("SPLIT", kvpIdList);
  writeKeyValuePairs("SPLIT", kvpIdList, kvpPairsList);
}

// **** Schedules ****
void MyMoneyStorageSql::writeSchedules()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  q.prepare("SELECT id FROM kmmSchedules;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Schedule list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  const QList<MyMoneySchedule> list = m_storage->scheduleList();
  QSqlQuery q2(*this);
  //TODO: find a way to prepare the queries outside of the loop.  writeSchedule()
  // modifies the query passed to it, so they have to be re-prepared every pass.
  signalProgress(0, list.count(), "Writing Schedules...");
  foreach (const MyMoneySchedule& it, list) {
    q.prepare(m_db.m_tables["kmmSchedules"].updateString());
    q2.prepare(m_db.m_tables["kmmSchedules"].insertString());
    bool insert = true;
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      insert = false;
      writeSchedule(it, q, insert);
    } else {
      writeSchedule(it, q2, insert);
    }
    signalProgress(++m_schedules, 0);
  }

  if (!dbList.isEmpty()) {
    foreach (const QString& it, dbList) {
      deleteSchedule(it);
    }
  }
}

void MyMoneyStorageSql::addSchedule(const MyMoneySchedule& sched)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmSchedules"].insertString());
  writeSchedule(sched, q, true);
  ++m_schedules;
  writeFileInfo();
}

void MyMoneyStorageSql::modifySchedule(const MyMoneySchedule& sched)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmSchedules"].updateString());
  writeSchedule(sched, q, false);
  writeFileInfo();
}

void MyMoneyStorageSql::removeSchedule(const MyMoneySchedule& sched)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  deleteSchedule(sched.id());
  --m_schedules;
  writeFileInfo();
}

void MyMoneyStorageSql::deleteSchedule(const QString& id)
{
  deleteTransaction(id);
  QSqlQuery q(*this);
  q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id");
  q.bindValue(":id", id);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Schedule Payment History")); // krazy:exclude=crashy
  q.prepare(m_db.m_tables["kmmSchedules"].deleteString());
  q.bindValue(":id", id);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Schedule")); // krazy:exclude=crashy
  //FIXME: enable when schedules have KVPs.
  //deleteKeyValuePairs("SCHEDULE", id);
}

void MyMoneyStorageSql::writeSchedule(const MyMoneySchedule& sch, QSqlQuery& q, bool insert)
{
  q.bindValue(":id", sch.id());
  q.bindValue(":name", sch.name());
  q.bindValue(":type", (int)sch.type());
  q.bindValue(":typeString", MyMoneySchedule::scheduleTypeToString(sch.type()));
  q.bindValue(":occurence", (int)sch.occurrencePeriod()); // krazy:exclude=spelling
  q.bindValue(":occurenceMultiplier", sch.occurrenceMultiplier());
  q.bindValue(":occurenceString", sch.occurrenceToString());
  q.bindValue(":paymentType", (int)sch.paymentType());
  q.bindValue(":paymentTypeString", MyMoneySchedule::paymentMethodToString(sch.paymentType()));
  q.bindValue(":startDate", sch.startDate().toString(Qt::ISODate));
  q.bindValue(":endDate", sch.endDate().toString(Qt::ISODate));
  if (sch.isFixed()) {
    q.bindValue(":fixed", "Y");
  } else {
    q.bindValue(":fixed", "N");
  }
  if (sch.autoEnter()) {
    q.bindValue(":autoEnter", "Y");
  } else {
    q.bindValue(":autoEnter", "N");
  }
  q.bindValue(":lastPayment", sch.lastPayment());
  q.bindValue(":nextPaymentDue", sch.nextDueDate().toString(Qt::ISODate));
  q.bindValue(":weekendOption", (int)sch.weekendOption());
  q.bindValue(":weekendOptionString", MyMoneySchedule::weekendOptionToString(sch.weekendOption()));
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Schedules"))); // krazy:exclude=crashy

  //store the payment history for this scheduled task.
  //easiest way is to delete all and re-insert; it's not a high use table
  q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id;");
  q.bindValue(":id", sch.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting  Schedule Payment History"))); // krazy:exclude=crashy

  q.prepare(m_db.m_tables["kmmSchedulePaymentHistory"].insertString());
  foreach (const QDate& it, sch.recordedPayments()) {
    q.bindValue(":schedId", sch.id());
    q.bindValue(":payDate", it.toString(Qt::ISODate));
    if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Schedule Payment History"))); // krazy:exclude=crashy
  }

  //store the transaction data for this task.
  if (!insert) {
    q.prepare(m_db.m_tables["kmmTransactions"].updateString());
  } else {
    q.prepare(m_db.m_tables["kmmTransactions"].insertString());
  }
  writeTransaction(sch.id(), sch.transaction(), q, "S");

  //FIXME: enable when schedules have KVPs.

  //Add in Key-Value Pairs for transactions.
  //deleteKeyValuePairs("SCHEDULE", sch.id());
  //writeKeyValuePairs("SCHEDULE", sch.id(), sch.pairs());
}

// **** Securities ****
void MyMoneyStorageSql::writeSecurities()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  QSqlQuery q2(*this);
  q.prepare("SELECT id FROM kmmSecurities;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building security list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  const QList<MyMoneySecurity> securityList = m_storage->securityList();
  signalProgress(0, securityList.count(), "Writing Securities...");
  q.prepare(m_db.m_tables["kmmSecurities"].updateString());
  q2.prepare(m_db.m_tables["kmmSecurities"].insertString());
  foreach (const MyMoneySecurity& it, securityList) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      writeSecurity(it, q);
    } else {
      writeSecurity(it, q2);
    }
    signalProgress(++m_securities, 0);
  }

  if (!dbList.isEmpty()) {
    QVariantList idList;
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      idList << it;
    }

    q.prepare("DELETE FROM kmmSecurities WHERE id = :id");
    q2.prepare("DELETE FROM kmmPrices WHERE fromId = :id OR toId = :id");
    q.bindValue(":id", idList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Security"));

    q2.bindValue(":fromId", idList);
    q2.bindValue(":toId", idList);
    if (!q2.execBatch()) throw MYMONEYEXCEPTION(buildError(q2, Q_FUNC_INFO, "deleting Security"));

    deleteKeyValuePairs("SECURITY", idList);
  }
}

void MyMoneyStorageSql::addSecurity(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmSecurities"].insertString());
  writeSecurity(sec, q);
  ++m_securities;
  writeFileInfo();
}

void MyMoneyStorageSql::modifySecurity(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << sec.id();
  deleteKeyValuePairs("SECURITY", kvpList);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmSecurities"].updateString());
  writeSecurity(sec, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeSecurity(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QVariantList kvpList;
  kvpList << sec.id();
  deleteKeyValuePairs("SECURITY", kvpList);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmSecurities"].deleteString());
  q.bindValue(":id", kvpList);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Security")));
  --m_securities;
  writeFileInfo();
}

void MyMoneyStorageSql::writeSecurity(const MyMoneySecurity& security, QSqlQuery& q)
{
  q.bindValue(":id", security.id());
  q.bindValue(":name", security.name());
  q.bindValue(":symbol", security.tradingSymbol());
  q.bindValue(":type", static_cast<int>(security.securityType()));
  q.bindValue(":typeString", MyMoneySecurity::securityTypeToString(security.securityType()));
  q.bindValue(":roundingMethod", static_cast<int>(security.roundingMethod()));
  q.bindValue(":smallestAccountFraction", security.smallestAccountFraction());
  q.bindValue(":pricePrecision", security.pricePrecision());
  q.bindValue(":tradingCurrency", security.tradingCurrency());
  q.bindValue(":tradingMarket", security.tradingMarket());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Securities"))); // krazy:exclude=crashy

  //Add in Key-Value Pairs for security
  QVariantList idList;
  idList << security.id();
  QList<QMap<QString, QString> > pairs;
  pairs << security.pairs();
  writeKeyValuePairs("SECURITY", idList, pairs);
  m_hiIdSecurities = 0;
}

// **** Prices ****
void MyMoneyStorageSql::writePrices()
{
  // due to difficulties in matching and determining deletes
  // easiest way is to delete all and re-insert
  QSqlQuery q(*this);
  q.prepare("DELETE FROM kmmPrices");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Prices"))); // krazy:exclude=crashy
  m_prices = 0;

  const MyMoneyPriceList list = m_storage->priceList();
  signalProgress(0, list.count(), "Writing Prices...");
  MyMoneyPriceList::ConstIterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it)   {
    writePricePair(*it);
  }
}

void MyMoneyStorageSql::writePricePair(const MyMoneyPriceEntries& p)
{
  MyMoneyPriceEntries::ConstIterator it;
  for (it = p.constBegin(); it != p.constEnd(); ++it) {
    writePrice(*it);
    signalProgress(++m_prices, 0);
  }
}

void MyMoneyStorageSql::addPrice(const MyMoneyPrice& p)
{
  if (m_readingPrices) return;
  // the app always calls addPrice, whether or not there is already one there
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  bool newRecord = false;
  QSqlQuery q(*this);
  QString s = m_db.m_tables["kmmPrices"].selectAllString(false);
  s += " WHERE fromId = :fromId AND toId = :toId AND priceDate = :priceDate;";
  q.prepare(s);
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("finding Price"))); // krazy:exclude=crashy
  if (q.next()) {
    q.prepare(m_db.m_tables["kmmPrices"].updateString());
  } else {
    q.prepare(m_db.m_tables["kmmPrices"].insertString());
    ++m_prices;
    newRecord = true;
  }
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  q.bindValue(":price", p.rate(QString()).toString());
  const MyMoneySecurity sec = m_storagePtr->security(p.to());
  q.bindValue(":priceFormatted",
              p.rate(QString()).formatMoney("", sec.pricePrecision()));
  q.bindValue(":priceSource", p.source());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Price"))); // krazy:exclude=crashy

  if (newRecord) writeFileInfo();
}

void MyMoneyStorageSql::removePrice(const MyMoneyPrice& p)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPrices"].deleteString());
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Price"))); // krazy:exclude=crashy
  --m_prices;
  writeFileInfo();
}

void MyMoneyStorageSql::writePrice(const MyMoneyPrice& p)
{
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPrices"].insertString());
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date().toString(Qt::ISODate));
  q.bindValue(":price", p.rate(QString()).toString());
  q.bindValue(":priceFormatted", p.rate(QString()).formatMoney("", 2));
  q.bindValue(":priceSource", p.source());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Prices"))); // krazy:exclude=crashy
}

// **** Currencies ****
void MyMoneyStorageSql::writeCurrencies()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  QSqlQuery q2(*this);
  q.prepare("SELECT ISOCode FROM kmmCurrencies;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Currency list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  const QList<MyMoneySecurity> currencyList = m_storage->currencyList();
  signalProgress(0, currencyList.count(), "Writing Currencies...");
  q.prepare(m_db.m_tables["kmmCurrencies"].updateString());
  q2.prepare(m_db.m_tables["kmmCurrencies"].insertString());
  foreach (const MyMoneySecurity& it, currencyList) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      writeCurrency(it, q);
    } else {
      writeCurrency(it, q2);
    }
    signalProgress(++m_currencies, 0);
  }

  if (!dbList.isEmpty()) {
    QVariantList isoCodeList;
    q.prepare("DELETE FROM kmmCurrencies WHERE ISOCode = :ISOCode");
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      isoCodeList << it;
    }

    q.bindValue(":ISOCode", isoCodeList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Currency"));
  }
}

void MyMoneyStorageSql::addCurrency(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmCurrencies"].insertString());
  writeCurrency(sec, q);
  ++m_currencies;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyCurrency(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmCurrencies"].updateString());
  writeCurrency(sec, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeCurrency(const MyMoneySecurity& sec)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmCurrencies"].deleteString());
  q.bindValue(":ISOcode", sec.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Currency"))); // krazy:exclude=crashy
  --m_currencies;
  writeFileInfo();
}

void MyMoneyStorageSql::writeCurrency(const MyMoneySecurity& currency, QSqlQuery& q)
{
  q.bindValue(":ISOcode", currency.id());
  q.bindValue(":name", currency.name());
  q.bindValue(":type", static_cast<int>(currency.securityType()));
  q.bindValue(":typeString", MyMoneySecurity::securityTypeToString(currency.securityType()));
  // writing the symbol as three short ints is a PITA, but the
  // problem is that database drivers have incompatible ways of declaring UTF8
  QString symbol = currency.tradingSymbol() + "   ";
  const ushort* symutf = symbol.utf16();
  //int ix = 0;
  //while (x[ix] != '\0') qDebug() << "symbol" << symbol << "char" << ix << "=" << x[ix++];
  //q.bindValue(":symbol1", symbol.mid(0,1).unicode()->unicode());
  //q.bindValue(":symbol2", symbol.mid(1,1).unicode()->unicode());
  //q.bindValue(":symbol3", symbol.mid(2,1).unicode()->unicode());
  q.bindValue(":symbol1", symutf[0]);
  q.bindValue(":symbol2", symutf[1]);
  q.bindValue(":symbol3", symutf[2]);
  q.bindValue(":symbolString", symbol);
  q.bindValue(":smallestCashFraction", currency.smallestCashFraction());
  q.bindValue(":smallestAccountFraction", currency.smallestAccountFraction());
  q.bindValue(":pricePrecision", currency.pricePrecision());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Currencies"))); // krazy:exclude=crashy
}


void MyMoneyStorageSql::writeReports()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  QSqlQuery q2(*this);
  q.prepare("SELECT id FROM kmmReportConfig;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Report list")); // krazy:exclude=crashy
  while (q.next()) dbList.append(q.value(0).toString());

  QList<MyMoneyReport> list = m_storage->reportList();
  signalProgress(0, list.count(), "Writing Reports...");
  q.prepare(m_db.m_tables["kmmReportConfig"].updateString());
  q2.prepare(m_db.m_tables["kmmReportConfig"].insertString());
  foreach (const MyMoneyReport& it, list) {
    if (dbList.contains(it.id())) {
      dbList.removeAll(it.id());
      writeReport(it, q);
    } else {
      writeReport(it, q2);
    }
    signalProgress(++m_reports, 0);
  }

  if (!dbList.isEmpty()) {
    QVariantList idList;
    q.prepare("DELETE FROM kmmReportConfig WHERE id = :id");
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      idList << it;
    }

    q.bindValue(":id", idList);
    if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Report"));
  }
}

void MyMoneyStorageSql::addReport(const MyMoneyReport& rep)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmReportConfig"].insertString());
  writeReport(rep, q);
  ++m_reports;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyReport(const MyMoneyReport& rep)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmReportConfig"].updateString());
  writeReport(rep, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeReport(const MyMoneyReport& rep)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare("DELETE FROM kmmReportConfig WHERE id = :id");
  q.bindValue(":id", rep.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Report"))); // krazy:exclude=crashy
  --m_reports;
  writeFileInfo();
}

void MyMoneyStorageSql::writeReport(const MyMoneyReport& rep, QSqlQuery& q)
{
  QDomDocument d; // create a dummy XML document
  QDomElement e = d.createElement("REPORTS");
  d.appendChild(e);
  rep.writeXML(d, e); // write the XML to document
  q.bindValue(":id", rep.id());
  q.bindValue(":name", rep.name());
  q.bindValue(":XML", d.toString());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Reports"))); // krazy:exclude=crashy
}

void MyMoneyStorageSql::writeBudgets()
{
  // first, get a list of what's on the database (see writeInstitutions)
  QList<QString> dbList;
  QSqlQuery q(*this);
  QSqlQuery q2(*this);
  q.prepare("SELECT name FROM kmmBudgetConfig;");
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "building Budget list")); // krazy:exclude=crashy
  while (q.next())
    dbList.append(q.value(0).toString());

  QList<MyMoneyBudget> list = m_storage->budgetList();
  signalProgress(0, list.count(), "Writing Budgets...");
  q.prepare(m_db.m_tables["kmmBudgetConfig"].updateString());
  q2.prepare(m_db.m_tables["kmmBudgetConfig"].insertString());
  foreach (const MyMoneyBudget& it, list) {
    if (dbList.contains(it.name())) {
      dbList.removeAll(it.name());
      writeBudget(it, q);
    } else {
      writeBudget(it, q2);
    }
    signalProgress(++m_budgets, 0);
  }

  if (!dbList.isEmpty()) {
    QVariantList idList;
    q.prepare("DELETE FROM kmmBudgetConfig WHERE id = :id");
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (const QString& it, dbList) {
      idList << it;
    }

    q.bindValue(":name", idList);
    if (!q.execBatch())
      throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "deleting Budget"));
  }
}

void MyMoneyStorageSql::addBudget(const MyMoneyBudget& bud)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmBudgetConfig"].insertString());
  writeBudget(bud, q);
  ++m_budgets;
  writeFileInfo();
}

void MyMoneyStorageSql::modifyBudget(const MyMoneyBudget& bud)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmBudgetConfig"].updateString());
  writeBudget(bud, q);
  writeFileInfo();
}

void MyMoneyStorageSql::removeBudget(const MyMoneyBudget& bud)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmBudgetConfig"].deleteString());
  q.bindValue(":id", bud.id());
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting Budget"))); // krazy:exclude=crashy
  --m_budgets;
  writeFileInfo();
}

void MyMoneyStorageSql::writeBudget(const MyMoneyBudget& bud, QSqlQuery& q)
{
  QDomDocument d; // create a dummy XML document
  QDomElement e = d.createElement("BUDGETS");
  d.appendChild(e);
  bud.writeXML(d, e); // write the XML to document
  q.bindValue(":id", bud.id());
  q.bindValue(":name", bud.name());
  q.bindValue(":start", bud.budgetStart());
  q.bindValue(":XML", d.toString());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing Budgets"))); // krazy:exclude=crashy
}

bool MyMoneyStorageSql::setupStoragePlugin(QString iid)
{
  // setupDatabase has to be called every time because this simple technique to check if was updated already
  // does not work if a user opens another file
  // also the setup is removed if the current database transaction is rolled back
  if (iid.isEmpty() /*|| m_loadedStoragePlugins.contains(iid)*/)
    return false;

  QString errorMsg;
  // TODO: port KF5 (needed for payeeidentifier plugin)
#if 0
  KMyMoneyPlugin::storagePlugin* plugin = KServiceTypeTrader::createInstanceFromQuery<KMyMoneyPlugin::storagePlugin>(
    QLatin1String("KMyMoney/sqlStoragePlugin"),
    QString("'%1' ~in [X-KMyMoney-PluginIid]").arg(iid.replace(QLatin1Char('\''), QLatin1String("\\'"))),
    0,
    QVariantList(),
    &errorMsg
  );
#else
  KMyMoneyPlugin::storagePlugin* plugin = 0;
#endif

  if (plugin == 0)
    throw MYMONEYEXCEPTION(QString("Could not load sqlStoragePlugin '%1', (error: %2)").arg(iid, errorMsg));

  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  if (plugin->setupDatabase(*this)) {
    m_loadedStoragePlugins.insert(iid);
    return true;
  }

  throw MYMONEYEXCEPTION(QString("Could not install sqlStoragePlugin '%1' in database.").arg(iid));
}

void MyMoneyStorageSql::insertStorableObject(const databaseStoreableObject& obj, const QString& id)
{
  setupStoragePlugin(obj.storagePluginIid());
  if (!obj.sqlSave(*this, id))
    throw MYMONEYEXCEPTION(QString("Could not save object with id '%1' in database (plugin failed).").arg(id));
}

void MyMoneyStorageSql::updateStorableObject(const databaseStoreableObject& obj, const QString& id)
{
  setupStoragePlugin(obj.storagePluginIid());
  if (!obj.sqlModify(*this, id))
    throw MYMONEYEXCEPTION(QString("Could not modify object with id '%1' in database (plugin failed).").arg(id));
}

void MyMoneyStorageSql::deleteStorableObject(const databaseStoreableObject& obj, const QString& id)
{
  setupStoragePlugin(obj.storagePluginIid());
  if (!obj.sqlRemove(*this, id))
    throw MYMONEYEXCEPTION(QString("Could not remove object with id '%1' from database (plugin failed).").arg(id));
}

void MyMoneyStorageSql::addOnlineJob(const onlineJob& job)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);
  QSqlQuery q(*this);
  q.prepare("INSERT INTO kmmOnlineJobs (id, type, jobSend, bankAnswerDate, state, locked) VALUES(:id, :type, :jobSend, :bankAnswerDate, :state, :locked);");
  writeOnlineJob(job, q);
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing onlineJob"))); // krazy:exclude=crashy
  ++m_onlineJobs;

  try {
    // Save online task
    insertStorableObject(*job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
  }
}

void MyMoneyStorageSql::modifyOnlineJob(const onlineJob& job)
{
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

  writeOnlineJob(job, query);
  if (!query.exec())
    throw MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, QString("writing onlineJob"))); // krazy:exclude=crashy

  try {
    // Modify online task
    updateStorableObject(*job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
    // If there is no task attached this is fine as well
  }
}

void MyMoneyStorageSql::writeOnlineJob(const onlineJob& job, QSqlQuery& query)
{
  Q_ASSERT(job.id().startsWith('O'));

  query.bindValue(":id", job.id());
  query.bindValue(":type", job.taskIid());
  query.bindValue(":jobSend", job.sendDate());
  query.bindValue(":bankAnswerDate", job.bankAnswerDate());
  switch (job.bankAnswerState()) {
    case onlineJob::acceptedByBank: query.bindValue(":state", QLatin1String("acceptedByBank")); break;
    case onlineJob::rejectedByBank: query.bindValue(":state", QLatin1String("rejectedByBank")); break;
    case onlineJob::abortedByUser: query.bindValue(":state", QLatin1String("abortedByUser")); break;
    case onlineJob::sendingError: query.bindValue(":state", QLatin1String("sendingError")); break;
    case onlineJob::noBankAnswer:
    default: query.bindValue(":state", QLatin1String("noBankAnswer"));
  }
  query.bindValue(":locked", QVariant::fromValue<QString>(job.isLocked() ? QLatin1String("Y") : QLatin1String("N")));
}

void MyMoneyStorageSql::writeOnlineJobs()
{
  QSqlQuery query(*this);
  if (!query.exec("DELETE FROM kmmOnlineJobs;"))
    throw MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, QLatin1String("Clean kmmOnlineJobs table")));

  const QList<onlineJob> jobs(m_storage->onlineJobList());
  signalProgress(0, jobs.count(), i18n("Inserting online jobs."));
  // Create list for onlineJobs which failed and the reason therefor
  QList<QPair<onlineJob, QString> > failedJobs;
  int jobCount = 0;
  foreach (const onlineJob& job, jobs) {
    try {
      addOnlineJob(job);
    } catch (MyMoneyException& e) {
      // Do not save e as this may point to an inherited class
      failedJobs.append(QPair<onlineJob, QString>(job, e.what()));
      qDebug() << "Failed to save onlineJob" << job.id() << "Reson:" << e.what();
    }

    signalProgress(++jobCount, 0);
  }

  if (!failedJobs.isEmpty()) {
    /** @todo Improve error message */
    throw MYMONEYEXCEPTION(i18np("Could not save one onlineJob.", "Could not save %1 onlineJobs.", failedJobs.count()));
  }
}

void MyMoneyStorageSql::removeOnlineJob(const onlineJob& job)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  // Remove onlineTask first, because it could have a contraint
  // which could block the removal of the onlineJob

  try {
    // Remove task
    deleteStorableObject(*job.constTask(), job.id());
  } catch (onlineJob::emptyTask&) {
  }

  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmOnlineJobs"].deleteString());
  q.bindValue(":id", job.id());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting onlineJob"))); // krazy:exclude=crashy
  --m_onlineJobs;
}

void MyMoneyStorageSql::addPayeeIdentifier(payeeIdentifier& ident)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  ident = payeeIdentifier(incrementPayeeIdentfierId(), ident);

  QSqlQuery q(*this);
  q.prepare("INSERT INTO kmmPayeeIdentifier (id, type) VALUES(:id, :type)");
  writePayeeIdentifier(ident, q);
  ++m_payeeIdentifier;

  try {
    insertStorableObject(*ident.data(), ident.idString());
  } catch (payeeIdentifier::empty&) {
  }
}

void MyMoneyStorageSql::modifyPayeeIdentifier(const payeeIdentifier& ident)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  QSqlQuery q(*this);
  q.prepare("SELECT type FROM kmmPayeeIdentifier WHERE id = ?");
  q.bindValue(0, ident.idString());
  if (!q.exec() || !q.next())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("modifying payeeIdentifier"))); // krazy:exclude=crashy

  bool typeChanged = (q.value(0).toString() != ident.iid());

  if (typeChanged) {
    // Delete old identifier if type changed
    const payeeIdentifier oldIdent(fetchPayeeIdentifier(ident.idString()));
    try {
      deleteStorableObject(*oldIdent.data(), ident.idString());
    } catch (payeeIdentifier::empty&) {
      // Note: this should not happen because the ui does not offer a way to change
      // the type of an payeeIdentifier if it was not correctly loaded.
      throw MYMONEYEXCEPTION(QLatin1String("Could not modify payeeIdentifier '")
        + ident.idString()
        + QLatin1String("' because type changed and could not remove identifier of old type. Maybe a plugin is missing?")
      ); // krazy:exclude=crashy
    }
  }

  q.prepare("UPDATE kmmPayeeIdentifier SET type = :type WHERE id = :id");
  writePayeeIdentifier(ident, q);

  try {
    if (typeChanged)
      insertStorableObject(*ident.data(), ident.idString());
    else
      updateStorableObject(*ident.data(), ident.idString());
  } catch (payeeIdentifier::empty&) {
  }
}

void MyMoneyStorageSql::removePayeeIdentifier(const payeeIdentifier& ident)
{
  MyMoneyDbTransaction t(*this, Q_FUNC_INFO);

  // Remove first, the table could have a contraint which prevents removal
  // of row in kmmPayeeIdentifier
  try {
    deleteStorableObject(*ident.data(), ident.idString());
  } catch (payeeIdentifier::empty&) {
  }

  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmPayeeIdentifier"].deleteString());
  q.bindValue(":id", ident.idString());
  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting payeeIdentifier"))); // krazy:exclude=crashy
  --m_payeeIdentifier;
}

void MyMoneyStorageSql::writePayeeIdentifier(const payeeIdentifier& pid, QSqlQuery& query)
{
  query.bindValue(":id", pid.idString());
  query.bindValue(":type", pid.iid());
  if (!query.exec()) {
    qWarning() << buildError(query, Q_FUNC_INFO, QString("modifying payeeIdentifier"));
    throw MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, QString("modifying payeeIdentifier"))); // krazy:exclude=crashy
  }
}

void MyMoneyStorageSql::writeFileInfo()
{
  // we have no real way of knowing when these change, so re-write them every time
  QVariantList kvpList;
  kvpList << "";
  QList<QMap<QString, QString> > pairs;
  pairs << m_storage->pairs();
  deleteKeyValuePairs("STORAGE", kvpList);
  writeKeyValuePairs("STORAGE", kvpList, pairs);

  QSqlQuery q(*this);
  q.prepare("SELECT count(*) FROM kmmFileInfo;");
  if (!q.exec() || !q.next())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "checking fileinfo")); // krazy:exclude=crashy

  if (q.value(0).toInt() == 0) {
    // Cannot use "INSERT INTO kmmFileInfo DEFAULT VALUES;" because it is not supported by MySQL
    q.prepare(QLatin1String("INSERT INTO kmmFileInfo (version) VALUES (null);"));
    if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, "inserting fileinfo")); // krazy:exclude=crashy
  }

  q.prepare(QLatin1String(
    "UPDATE kmmFileInfo SET "
      "version = :version, "
      "fixLevel = :fixLevel, "
      "created = :created, "
      "lastModified = :lastModified, "
      "baseCurrency = :baseCurrency, "
      "dateRangeStart = :dateRangeStart, "
      "dateRangeEnd = :dateRangeEnd, "
      "hiInstitutionId = :hiInstitutionId, "
      "hiPayeeId = :hiPayeeId, "
      "hiTagId = :hiTagId, "
      "hiAccountId = :hiAccountId, "
      "hiTransactionId = :hiTransactionId, "
      "hiScheduleId = :hiScheduleId, "
      "hiSecurityId = :hiSecurityId, "
      "hiReportId = :hiReportId, "
      "hiBudgetId = :hiBudgetId, "
      "hiOnlineJobId = :hiOnlineJobId, "
      "hiPayeeIdentifierId = :hiPayeeIdentifierId, "
      "encryptData = :encryptData, "
      "updateInProgress = :updateInProgress, "
      "logonUser = :logonUser, "
      "logonAt = :logonAt, "
      //! @todo The following updates are for backwards compatibility only
      //! remove backwards compatibility in a later version
      "institutions = :institutions, "
      "accounts = :accounts, "
      "payees = :payees, "
      "tags = :tags, "
      "transactions = :transactions, "
      "splits = :splits, "
      "securities = :securities, "
      "prices = :prices, "
      "currencies = :currencies, "
      "schedules = :schedules, "
      "reports = :reports, "
      "kvps = :kvps, "
      "budgets = :budgets; "
    )
  );

  q.bindValue(":version", m_dbVersion);
  q.bindValue(":fixLevel", m_storage->fileFixVersion());
  q.bindValue(":created", m_storage->creationDate().toString(Qt::ISODate));
  //q.bindValue(":lastModified", m_storage->lastModificationDate().toString(Qt::ISODate));
  q.bindValue(":lastModified", QDate::currentDate().toString(Qt::ISODate));
  q.bindValue(":baseCurrency", m_storage->pairs()["kmm-baseCurrency"]);
  q.bindValue(":dateRangeStart", QDate());
  q.bindValue(":dateRangeEnd", QDate());

  //FIXME: This modifies all m_<variable> used in this function.
  // Sometimes the memory has been updated.

  // Should most of these be tracked in a view?
  // Variables actually needed are: version, fileFixVersion, creationDate,
  // baseCurrency, encryption, update info, and logon info.
  //try {
  //readFileInfo();
  //} catch (...) {
  //startCommitUnit(Q_FUNC_INFO);
  //}

  //! @todo The following bindings are for backwards compatibility only
  //! remove backwards compatibility in a later version
  q.bindValue(":hiInstitutionId", QVariant::fromValue(getNextInstitutionId()));
  q.bindValue(":hiPayeeId", QVariant::fromValue(getNextPayeeId()));
  q.bindValue(":hiTagId", QVariant::fromValue(getNextTagId()));
  q.bindValue(":hiAccountId", QVariant::fromValue(getNextAccountId()));
  q.bindValue(":hiTransactionId", QVariant::fromValue(getNextTransactionId()));
  q.bindValue(":hiScheduleId", QVariant::fromValue(getNextScheduleId()));
  q.bindValue(":hiSecurityId", QVariant::fromValue(getNextSecurityId()));
  q.bindValue(":hiReportId", QVariant::fromValue(getNextReportId()));
  q.bindValue(":hiBudgetId", QVariant::fromValue(getNextBudgetId()));
  q.bindValue(":hiOnlineJobId", QVariant::fromValue(getNextOnlineJobId()));
  q.bindValue(":hiPayeeIdentifierId", QVariant::fromValue(getNextPayeeIdentifierId()));

  q.bindValue(":encryptData", m_encryptData);
  q.bindValue(":updateInProgress", "N");
  q.bindValue(":logonUser", m_logonUser);
  q.bindValue(":logonAt", m_logonAt.toString(Qt::ISODate));

  //! @todo The following bindings are for backwards compatibility only
  //! remove backwards compatibility in a later version
  q.bindValue(":institutions", (unsigned long long) m_institutions);
  q.bindValue(":accounts", (unsigned long long) m_accounts);
  q.bindValue(":payees", (unsigned long long) m_payees);
  q.bindValue(":tags", (unsigned long long) m_tags);
  q.bindValue(":transactions", (unsigned long long) m_transactions);
  q.bindValue(":splits", (unsigned long long) m_splits);
  q.bindValue(":securities", (unsigned long long) m_securities);
  q.bindValue(":prices", (unsigned long long) m_prices);
  q.bindValue(":currencies", (unsigned long long) m_currencies);
  q.bindValue(":schedules", (unsigned long long) m_schedules);
  q.bindValue(":reports", (unsigned long long) m_reports);
  q.bindValue(":kvps", (unsigned long long) m_kvps);
  q.bindValue(":budgets", (unsigned long long) m_budgets);

  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing FileInfo"))); // krazy:exclude=crashy
}

// **** Key/value pairs ****
void MyMoneyStorageSql::writeKeyValuePairs(const QString& kvpType, const QVariantList& kvpId, const QList<QMap<QString, QString> >& pairs)
{
  if (pairs.empty())
    return;

  QVariantList type;
  QVariantList id;
  QVariantList key;
  QVariantList value;
  int pairCount = 0;

  for (int i = 0; i < kvpId.size(); ++i) {
    QMap<QString, QString>::ConstIterator it;
    for (it = pairs[i].constBegin(); it != pairs[i].constEnd(); ++it) {
      type << kvpType;
      id << kvpId[i];
      key << it.key();
      value << it.value();
    }
    pairCount += pairs[i].size();
  }

  QSqlQuery q(*this);
  q.prepare(m_db.m_tables["kmmKeyValuePairs"].insertString());
  q.bindValue(":kvpType", type);
  q.bindValue(":kvpId", id);
  q.bindValue(":kvpKey", key);
  q.bindValue(":kvpData", value);
  if (!q.execBatch()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("writing KVP")));
  m_kvps += pairCount;
}

void MyMoneyStorageSql::deleteKeyValuePairs(const QString& kvpType, const QVariantList& idList)
{
  QSqlQuery q(*this);
  q.prepare("DELETE FROM kmmKeyValuePairs WHERE kvpType = :kvpType AND kvpId = :kvpId;");
  QVariantList typeList;
  for (int i = 0; i < idList.size(); ++i) {
    typeList << kvpType;
  }
  q.bindValue(":kvpType", typeList);
  q.bindValue(":kvpId", idList);
  if (!q.execBatch()) {
    QString idString;
    for (int i = 0; i < idList.size(); ++i) {
      idString.append(idList[i].toString() + ' ');
    }
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("deleting kvp for %1 %2").arg(kvpType).arg(idString)));
  }
  m_kvps -= q.numRowsAffected();
}

//******************************** read SQL routines **************************************
#define GETSTRING(a) q.value(a).toString()
#define GETDATE(a) getDate(GETSTRING(a))
#define GETDATETIME(a) getDateTime(GETSTRING(a))
#define GETINT(a) q.value(a).toInt()
#define GETULL(a) q.value(a).toULongLong()

void MyMoneyStorageSql::readFileInfo()
{
  signalProgress(0, 1, QObject::tr("Loading file information..."));

  QSqlQuery q(*this);

  q.prepare(
    "SELECT "
    "  created, lastModified, "
    "  encryptData, logonUser, logonAt, "
    "  (SELECT count(*) FROM kmmInstitutions) AS institutions, "
    "  (SELECT count(*) from kmmAccounts) AS accounts, "
    "  (SELECT count(*) FROM kmmCurrencies) AS currencies, "
    "  (SELECT count(*) FROM kmmPayees) AS payees, "
    "  (SELECT count(*) FROM kmmTags) AS tags, "
    "  (SELECT count(*) FROM kmmTransactions) AS transactions, "
    "  (SELECT count(*) FROM kmmSplits) AS splits, "
    "  (SELECT count(*) FROM kmmSecurities) AS securities, "
    "  (SELECT count(*) FROM kmmCurrencies) AS currencies, "
    "  (SELECT count(*) FROM kmmSchedules) AS schedules, "
    "  (SELECT count(*) FROM kmmPrices) AS prices, "
    "  (SELECT count(*) FROM kmmKeyValuePairs) AS kvps, "
    "  (SELECT count(*) FROM kmmReportConfig) AS reports, "
    "  (SELECT count(*) FROM kmmBudgetConfig) AS budgets, "
    "  (SELECT count(*) FROM kmmOnlineJobs) AS onlineJobs, "
    "  (SELECT count(*) FROM kmmPayeeIdentifier) AS payeeIdentifier "
    "FROM kmmFileInfo;"
  );

  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading FileInfo"))); // krazy:exclude=crashy
  if (!q.next())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("retrieving FileInfo")));

  QSqlRecord rec = q.record();
  m_storage->setCreationDate(GETDATE(rec.indexOf("created")));
  m_storage->setLastModificationDate(GETDATE(rec.indexOf("lastModified")));

  m_institutions = (unsigned long) GETULL(rec.indexOf("institutions"));
  m_accounts = (unsigned long) GETULL(rec.indexOf("accounts"));
  m_payees = (unsigned long) GETULL(rec.indexOf("payees"));
  m_tags = (unsigned long) GETULL(rec.indexOf("tags"));
  m_transactions = (unsigned long) GETULL(rec.indexOf("transactions"));
  m_splits = (unsigned long) GETULL(rec.indexOf("splits"));
  m_securities = (unsigned long) GETULL(rec.indexOf("securities"));
  m_currencies = (unsigned long) GETULL(rec.indexOf("currencies"));
  m_schedules = (unsigned long) GETULL(rec.indexOf("schedules"));
  m_prices = (unsigned long) GETULL(rec.indexOf("prices"));
  m_kvps = (unsigned long) GETULL(rec.indexOf("kvps"));
  m_reports = (unsigned long) GETULL(rec.indexOf("reports"));
  m_budgets = (unsigned long) GETULL(rec.indexOf("budgets"));
  m_onlineJobs = (unsigned long) GETULL(rec.indexOf("onlineJobs"));
  m_payeeIdentifier = (unsigned long) GETULL(rec.indexOf("payeeIdentifier"));

  m_encryptData = GETSTRING(rec.indexOf("encryptData"));
  m_logonUser = GETSTRING(rec.indexOf("logonUser"));
  m_logonAt = GETDATETIME(rec.indexOf("logonAt"));

  signalProgress(1, 0);
  m_storage->setPairs(readKeyValuePairs("STORAGE", QString("")).pairs());
}

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

void MyMoneyStorageSql::readInstitutions()
{
  try {
    QMap<QString, MyMoneyInstitution> iList = fetchInstitutions();
    m_storage->loadInstitutions(iList);
    readFileInfo();
    m_storage->loadInstitutionId(m_hiIdInstitutions);
  } catch (const MyMoneyException &) {
    throw;
  }
}

const QMap<QString, MyMoneyInstitution> MyMoneyStorageSql::fetchInstitutions(const QStringList& idList, bool forUpdate) const
{
  int institutionsNb = (idList.isEmpty() ? m_institutions : idList.size());
  signalProgress(0, institutionsNb, QObject::tr("Loading institutions..."));
  int progress = 0;
  QMap<QString, MyMoneyInstitution> iList;
  unsigned long lastId = 0;
  const MyMoneyDbTable& t = m_db.m_tables["kmmInstitutions"];
  QSqlQuery sq(*const_cast <MyMoneyStorageSql*>(this));
  sq.prepare("SELECT id from kmmAccounts where institutionId = :id");
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
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
    queryString += m_driver->forUpdateString();

  queryString += ';';

  q.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      q.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Institution"))); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int managerCol = t.fieldNumber("manager");
  int routingCodeCol = t.fieldNumber("routingCode");
  int addressStreetCol = t.fieldNumber("addressStreet");
  int addressCityCol = t.fieldNumber("addressCity");
  int addressZipcodeCol = t.fieldNumber("addressZipcode");
  int telephoneCol = t.fieldNumber("telephone");

  while (q.next()) {
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
    if (!sq.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Institution AccountList"))); // krazy:exclude=crashy
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    foreach (const QString& it, aList)
    inst.addAccountId(it);

    iList[iid] = MyMoneyInstitution(iid, inst);
    unsigned long id = extractId(iid);
    if (id > lastId)
      lastId = id;

    signalProgress(++progress, 0);
  }
  return iList;
}

void MyMoneyStorageSql::readPayees(const QString& id)
{
  QList<QString> list;
  list.append(id);
  readPayees(list);
}

void MyMoneyStorageSql::readPayees(const QList<QString>& pid)
{
  try {
    m_storage->loadPayees(fetchPayees(pid));
    m_storage->loadPayeeId(getNextPayeeId());
  } catch (const MyMoneyException &) {
  }
//  if (pid.isEmpty()) m_payeeListRead = true;
}

const QMap<QString, MyMoneyPayee> MyMoneyStorageSql::fetchPayees(const QStringList& idList, bool /*forUpdate*/) const
{
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (m_displayStatus) {
    int payeesNb = (idList.isEmpty() ? m_payees : idList.size());
    signalProgress(0, payeesNb, QObject::tr("Loading payees..."));
  }

  int progress = 0;
  QMap<QString, MyMoneyPayee> pList;

  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
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

  q.prepare(queryString);

  if (!idList.isEmpty()) {
    // Bind values
    QStringList::const_iterator end = idList.constEnd();
    for (QStringList::const_iterator iter = idList.constBegin(); iter != end; ++iter) {
      q.addBindValue(*iter);
    }
  }

  if (!q.exec())
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Payee"))); // krazy:exclude=crashy
  const QSqlRecord record = q.record();
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

  q.next();
  while (q.isValid()) {
    QString pid;
    QString boolChar;
    MyMoneyPayee payee;
    unsigned int type;
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

    payee.setMatchData(static_cast<MyMoneyPayee::payeeMatchType>(type), ignoreCase, matchKeys);

    // Get payeeIdentifier ids
    QStringList identifierIds;
    do {
      identifierIds.append(GETSTRING(identIdCol));
    } while (q.next() && GETSTRING(idCol) == pid);   // as long as the payeeId is unchanged

    // Fetch and save payeeIdentifier
    if (!identifierIds.isEmpty()) {
      QList< ::payeeIdentifier > identifier = fetchPayeeIdentifiers(identifierIds).values();
      payee.resetPayeeIdentifiers(identifier);
    }

    if (pid == "USER")
      m_storage->setUser(payee);
    else
      pList[pid] = MyMoneyPayee(pid, payee);

    if (m_displayStatus)
      signalProgress(++progress, 0);
  }
  return pList;
}

void MyMoneyStorageSql::readTags(const QString& id)
{
  QList<QString> list;
  list.append(id);
  readTags(list);
}

void MyMoneyStorageSql::readTags(const QList<QString>& pid)
{
  try {
    m_storage->loadTags(fetchTags(pid));
    readFileInfo();
    m_storage->loadTagId(m_hiIdTags);
  } catch (const MyMoneyException &) {
  }
//  if (pid.isEmpty()) m_tagListRead = true;
}

const QMap<QString, onlineJob> MyMoneyStorageSql::fetchOnlineJobs(const QStringList& idList, bool forUpdate) const
{
  Q_UNUSED(forUpdate);
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (m_displayStatus)
    signalProgress(0, idList.isEmpty() ? m_onlineJobs : idList.size(), QObject::tr("Loading online banking data..."));

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
    throw MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, QString("reading onlineJobs"))); // krazy:exclude=crashy

  // Create onlineJobs
  int progress = 0;
  QMap<QString, onlineJob> jobList;

  while (query.next()) {
    const QString& id = query.value(0).toString();
    onlineTask *const task = onlineJobAdministration::instance()->createOnlineTaskFromSqlDatabase(query.value(1).toString(), id, *this);
    onlineJob job = onlineJob(task, id);
    job.setJobSend(query.value(2).toDateTime());
    onlineJob::sendingState state;
    const QString stateString = query.value(4).toString();
    if (stateString == "acceptedByBank")
      state = onlineJob::acceptedByBank;
    else if (stateString == "rejectedByBank")
      state = onlineJob::rejectedByBank;
    else if (stateString == "abortedByUser")
      state = onlineJob::abortedByUser;
    else if (stateString == "sendingError")
      state = onlineJob::sendingError;
    else // includes: stateString == "noBankAnswer"
      state = onlineJob::noBankAnswer;

    job.setBankAnswer(state, query.value(4).toDateTime());
    job.setLock(query.value(5).toString() == QLatin1String("Y") ? true : false);
    jobList.insert(job.id(), job);
    if (m_displayStatus)
      signalProgress(++progress, 0);
  }
  return jobList;
}

payeeIdentifier MyMoneyStorageSql::fetchPayeeIdentifier(const QString& id) const
{
  QMap<QString, payeeIdentifier> list = fetchPayeeIdentifiers(QStringList(id));
  QMap<QString, payeeIdentifier>::const_iterator iter = list.constFind(id);
  if (iter == list.constEnd())
    throw MYMONEYEXCEPTION(QLatin1String("payeeIdentifier with id '") + id + QLatin1String("' not found.")); // krazy:exclude=crashy
  return *iter;
}

const QMap< QString, payeeIdentifier > MyMoneyStorageSql::fetchPayeeIdentifiers(const QStringList& idList) const
{
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
    throw MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, QString("reading payee identifiers"))); // krazy:exclude=crashy

  QMap<QString, payeeIdentifier> identList;

  while (query.next()) {
    const QString id = query.value(0).toString();
    identList.insert(id, payeeIdentifierLoader::instance()->createPayeeIdentifierFromSqlDatabase(*this, query.value(1).toString(), id));
  }

  return identList;
}

const QMap<QString, MyMoneyTag> MyMoneyStorageSql::fetchTags(const QStringList& idList, bool /*forUpdate*/) const
{
  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (m_displayStatus) {
    int tagsNb = (idList.isEmpty() ? m_tags : idList.size());
    signalProgress(0, tagsNb, QObject::tr("Loading tags..."));
  } else {
//    if (m_tagListRead) return;
  }
  int progress = 0;
  QMap<QString, MyMoneyTag> taList;
  //unsigned long lastId;
  const MyMoneyDbTable& t = m_db.m_tables["kmmTags"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    q.prepare(t.selectAllString());
  } else {
    QString whereClause = " where (";
    QString itemConnector = "";
    foreach (const QString& it, idList) {
      whereClause.append(QString("%1id = '%2'").arg(itemConnector).arg(it));
      itemConnector = " or ";
    }
    whereClause += ')';
    q.prepare(t.selectAllString(false) + whereClause);
  }
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Tag"))); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int notesCol = t.fieldNumber("notes");
  int tagColorCol = t.fieldNumber("tagColor");
  int closedCol = t.fieldNumber("closed");

  while (q.next()) {
    QString pid;
    QString boolChar;
    MyMoneyTag tag;
    pid = GETSTRING(idCol);
    tag.setName(GETSTRING(nameCol));
    tag.setNotes(GETSTRING(notesCol));
    tag.setClosed((GETSTRING(closedCol) == "Y"));
    tag.setTagColor(QColor(GETSTRING(tagColorCol)));
    taList[pid] = MyMoneyTag(pid, tag);
    if (m_displayStatus) signalProgress(++progress, 0);
  }
  return taList;
}

const QMap<QString, MyMoneyAccount> MyMoneyStorageSql::fetchAccounts(const QStringList& idList, bool forUpdate) const
{
  int accountsNb = (idList.isEmpty() ? m_accounts : idList.size());
  signalProgress(0, accountsNb, QObject::tr("Loading accounts..."));
  int progress = 0;
  QMap<QString, MyMoneyAccount> accList;
  QStringList kvpAccountList(idList);

  const MyMoneyDbTable& t = m_db.m_tables["kmmAccounts"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
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
    queryString += m_driver->forUpdateString();
    childQueryString += m_driver->forUpdateString();
  }

  q.prepare(queryString);
  sq.prepare(childQueryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      q.bindValue(QString(":id%1").arg(i), *bindVal);
      sq.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Account"))); // krazy:exclude=crashy
  if (!sq.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading subAccountList"))); // krazy:exclude=crashy

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

  while (q.next()) {
    QString aid;
    QString balance;
    MyMoneyAccount acc;

    aid = GETSTRING(idCol);
    acc.setInstitutionId(GETSTRING(institutionIdCol));
    acc.setParentAccountId(GETSTRING(parentIdCol));
    acc.setLastReconciliationDate(GETDATE(lastReconciledCol));
    acc.setLastModified(GETDATE(lastModifiedCol));
    acc.setOpeningDate(GETDATE(openingDateCol));
    acc.setNumber(GETSTRING(accountNumberCol));
    acc.setAccountType(static_cast<Account>(GETINT(accountTypeCol)));
    acc.setName(GETSTRING(accountNameCol));
    acc.setDescription(GETSTRING(descriptionCol));
    acc.setCurrencyId(GETSTRING(currencyIdCol));
    acc.setBalance(MyMoneyMoney(GETSTRING(balanceCol)));
    const_cast <MyMoneyStorageSql*>(this)->m_transactionCountMap[aid] = (unsigned long) GETULL(transactionCountCol);

    // Process any key value pair
    if (idList.empty())
      kvpAccountList.append(aid);

    accList.insert(aid, MyMoneyAccount(aid, acc));
    if (acc.value("PreferredAccount") == "Yes") {
      const_cast <MyMoneyStorageSql*>(this)->m_preferred.addAccount(aid);
    }
    signalProgress(++progress, 0);
  }

  QMap<QString, MyMoneyAccount>::Iterator it_acc;
  QMap<QString, MyMoneyAccount>::Iterator accListEnd = accList.end();
  while (sq.next()) {
    it_acc = accList.find(sq.value(1).toString());
    if (it_acc != accListEnd && it_acc.value().id() == sq.value(1).toString()) {
      while (sq.isValid() && it_acc != accListEnd
             && it_acc.value().id() == sq.value(1).toString()) {
        it_acc.value().addAccountId(sq.value(0).toString());
        sq.next();
      }
      sq.previous();
    }
  }

  //TODO: There should be a better way than this.  What's below is O(n log n) or more,
  // where it may be able to be done in O(n), if things are just right.
  // The operator[] call in the loop is the most expensive call in this function, according
  // to several profile runs.
  QHash <QString, MyMoneyKeyValueContainer> kvpResult = readKeyValuePairs("ACCOUNT", kvpAccountList);
  QHash <QString, MyMoneyKeyValueContainer>::const_iterator kvp_end = kvpResult.constEnd();
  for (QHash <QString, MyMoneyKeyValueContainer>::const_iterator it_kvp = kvpResult.constBegin();
       it_kvp != kvp_end; ++it_kvp) {
    accList[it_kvp.key()].setPairs(it_kvp.value().pairs());
  }

  kvpResult = readKeyValuePairs("ONLINEBANKING", kvpAccountList);
  kvp_end = kvpResult.constEnd();
  for (QHash <QString, MyMoneyKeyValueContainer>::const_iterator it_kvp = kvpResult.constBegin();
       it_kvp != kvp_end; ++it_kvp) {
    accList[it_kvp.key()].setOnlineBankingSettings(it_kvp.value());
  }

  return accList;
}

void MyMoneyStorageSql::readAccounts()
{
  m_storage->loadAccounts(fetchAccounts());
  m_storage->loadAccountId(m_hiIdAccounts);
}

const QMap<QString, MyMoneyMoney> MyMoneyStorageSql::fetchBalance(const QStringList& idList, const QDate& date) const
{

  QMap<QString, MyMoneyMoney> returnValue;
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
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
  q.prepare(queryString);

  int i = 0;
  foreach (const QString& bindVal, idList) {
    q.bindValue(QString(":id%1").arg(i), bindVal);
    ++i;
  }

  if (!q.exec()) // krazy:exclude=crashy
    throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("fetching balance")));
  QString id;
  QString oldId;
  MyMoneyMoney temp;
  while (q.next()) {
    id = q.value(2).toString();
    // If the old ID does not match the new ID, then the account being summed has changed.
    // Write the balance into the returnValue map and update the oldId to the current one.
    if (id != oldId) {
      if (!oldId.isEmpty()) {
        returnValue.insert(oldId, temp);
        temp = 0;
      }
      oldId = id;
    }
    if (MyMoneySplit::ActionSplitShares == q.value(0).toString())
      temp *= MyMoneyMoney(q.value(1).toString());
    else
      temp += MyMoneyMoney(q.value(1).toString());
  }
  // Do not forget the last id in the list.
  returnValue.insert(id, temp);

  // Return the map.
  return returnValue;
}

void MyMoneyStorageSql::readTransactions(const QString& tidList, const QString& dateClause)
{
  try {
    m_storage->loadTransactions(fetchTransactions(tidList, dateClause));
    m_storage->loadTransactionId(getNextTransactionId());
  } catch (const MyMoneyException &) {
    throw;
  }
}

void MyMoneyStorageSql::readTransactions(const MyMoneyTransactionFilter& filter)
{
  try {
    m_storage->loadTransactions(fetchTransactions(filter));
    m_storage->loadTransactionId(getNextTransactionId());
  } catch (const MyMoneyException &) {
    throw;
  }
}

const QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions(const QString& tidList, const QString& dateClause, bool /*forUpdate*/) const
{
//  if (m_transactionListRead) return; // all list already in memory
  if (m_displayStatus) {
    int transactionsNb = (tidList.isEmpty() ? m_transactions : tidList.size());
    signalProgress(0, transactionsNb, QObject::tr("Loading transactions..."));
  }
  int progress = 0;
//  m_payeeList.clear();
  QString whereClause = " WHERE txType = 'N' ";
  if (! tidList.isEmpty()) {
    whereClause += " AND id IN " + tidList;
  }
  if (!dateClause.isEmpty()) whereClause += " and " + dateClause;
  const MyMoneyDbTable& t = m_db.m_tables["kmmTransactions"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare(t.selectAllString(false) + whereClause + " ORDER BY id;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Transaction"))); // krazy:exclude=crashy
  const MyMoneyDbTable& ts = m_db.m_tables["kmmSplits"];
  whereClause = " WHERE txType = 'N' ";
  if (! tidList.isEmpty()) {
    whereClause += " AND transactionId IN " + tidList;
  }
  if (!dateClause.isEmpty()) whereClause += " and " + dateClause;
  QSqlQuery qs(*const_cast <MyMoneyStorageSql*>(this));
  QString splitQuery = ts.selectAllString(false) + whereClause
                       + " ORDER BY transactionId, splitId;";
  qs.prepare(splitQuery);
  if (!qs.exec()) throw MYMONEYEXCEPTION(buildError(qs, Q_FUNC_INFO, "reading Splits")); // krazy:exclude=crashy
  QString splitTxId = "ZZZ";
  MyMoneySplit s;
  if (qs.next()) {
    splitTxId = qs.value(0).toString();
    readSplit(s, qs);
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

  while (q.next()) {
    MyMoneyTransaction tx;
    QString txId = GETSTRING(idCol);
    tx.setPostDate(GETDATE(postDateCol));
    tx.setMemo(GETSTRING(memoCol));
    tx.setEntryDate(GETDATE(entryDateCol));
    tx.setCommodity(GETSTRING(currencyIdCol));
    tx.setBankID(GETSTRING(bankIdCol));

    // skip all splits while the transaction id of the split is less than
    // the transaction id of the current transaction. Don't forget to check
    // for the ZZZ flag for the end of the list.
    while (txId < splitTxId && splitTxId != "ZZZ") {
      if (qs.next()) {
        splitTxId = qs.value(0).toString();
        readSplit(s, qs);
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
        readSplit(s, qs);
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
  QHash <QString, MyMoneyKeyValueContainer> kvpMap = readKeyValuePairs("TRANSACTION", txList);
  QMap<QString, MyMoneyTransaction>::Iterator txMapEnd = txMap.end();
  for (QMap<QString, MyMoneyTransaction>::Iterator i = txMap.begin();
       i != txMapEnd; ++i) {
    i.value().setPairs(kvpMap[i.value().id()].pairs());

    if (m_displayStatus) signalProgress(++progress, 0);
  }

  if ((tidList.isEmpty()) && (dateClause.isEmpty())) {
    //qDebug("setting full list read");
  }
  return txMap;
}

int MyMoneyStorageSql::splitState(const TransactionFilter::State& state) const
{
  auto rc = (int)Split::State::NotReconciled;

  switch (state) {
    default:
    case TransactionFilter::State::NotReconciled:
      break;

    case TransactionFilter::State::Cleared:
      rc = (int)Split::State::Cleared;
      break;

    case TransactionFilter::State::Reconciled:
      rc = (int)Split::State::Reconciled;
      break;

    case TransactionFilter::State::Frozen:
      rc = (int)Split::State::Frozen;
      break;
  }
  return rc;
}

const QMap<QString, MyMoneyTransaction> MyMoneyStorageSql::fetchTransactions(const MyMoneyTransactionFilter& filter) const
{
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
    alert("Amount Filter Set");
    canImplementFilter = false;
  }
  QString n1, n2;
  if (filter.numberFilter(n1, n2)) {
    alert("Number filter set");
    canImplementFilter = false;
  }
  int t1;
  if (filter.firstType(t1)) {
    alert("Type filter set");
    canImplementFilter = false;
  }
//  int s1;
//  if (filter.firstState(s1)) {
//    alert("State filter set");
//    canImplementFilter = false;
//  }
  QRegExp t2;
  if (filter.textFilter(t2)) {
    alert("text filter set");
    canImplementFilter = false;
  }
  MyMoneyTransactionFilter::FilterSet s = filter.filterSet();
  if (s.singleFilter.validityFilter) {
    alert("Validity filter set");
    canImplementFilter = false;
  }
  if (!canImplementFilter) {
    QMap<QString, MyMoneyTransaction> transactionList =  fetchTransactions();
    QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

    std::remove_if(transactionList.begin(), transactionList.end(), FilterFail(filter));
    return transactionList;
  }

  bool splitFilterActive = false; // the split filter is active if we are selecting on fields in the split table
  // get start and end dates
  QDate start = filter.fromDate();
  QDate end = filter.toDate();
  // not entirely sure if the following is correct, but at best, saves a lot of reads, at worst
  // it only causes us to read a few more transactions that strictly necessary (I think...)
  if (start == m_startDate) start = QDate();
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
                          .arg(splitState(TransactionFilter::State(it))));
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

unsigned long MyMoneyStorageSql::transactionCount(const QString& aid) const
{
  if (aid.isEmpty())
    return m_transactions;
  else
    return m_transactionCountMap[aid];
}

void MyMoneyStorageSql::readSplit(MyMoneySplit& s, const QSqlQuery& q) const
{
  // Set these up as statics, since the field numbers should not change
  // during execution.
  static const MyMoneyDbTable& t = m_db.m_tables["kmmSplits"];
  static const int splitIdCol = t.fieldNumber("splitId");
  static const int transactionIdCol = t.fieldNumber("transactionId");
  static const int payeeIdCol = t.fieldNumber("payeeId");
  static const int reconcileDateCol = t.fieldNumber("reconcileDate");
  static const int actionCol = t.fieldNumber("action");
  static const int reconcileFlagCol = t.fieldNumber("reconcileFlag");
  static const int valueCol = t.fieldNumber("value");
  static const int sharesCol = t.fieldNumber("shares");
  static const int priceCol = t.fieldNumber("price");
  static const int memoCol = t.fieldNumber("memo");
  static const int accountIdCol = t.fieldNumber("accountId");
  static const int costCenterIdCol = t.fieldNumber("costCenterId");
  static const int checkNumberCol = t.fieldNumber("checkNumber");
//  static const int postDateCol = t.fieldNumber("postDate"); // FIXME - when Tom puts date into split object
  static const int bankIdCol = t.fieldNumber("bankId");

  s.clearId();

  QList<QString> tagIdList;
  QSqlQuery q1(*const_cast <MyMoneyStorageSql*>(this));
  q1.prepare("SELECT tagId from kmmTagSplits where splitId = :id and transactionId = :transactionId");
  q1.bindValue(":id", GETSTRING(splitIdCol));
  q1.bindValue(":transactionId", GETSTRING(transactionIdCol));
  if (!q1.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading tagId in Split"))); // krazy:exclude=crashy
  while (q1.next())
    tagIdList << q1.value(0).toString();

  s.setTagIdList(tagIdList);
  s.setPayeeId(GETSTRING(payeeIdCol));
  s.setReconcileDate(GETDATE(reconcileDateCol));
  s.setAction(GETSTRING(actionCol));
  s.setReconcileFlag(static_cast<Split::State>(GETINT(reconcileFlagCol)));
  s.setValue(MyMoneyMoney(QStringEmpty(GETSTRING(valueCol))));
  s.setShares(MyMoneyMoney(QStringEmpty(GETSTRING(sharesCol))));
  s.setPrice(MyMoneyMoney(QStringEmpty(GETSTRING(priceCol))));
  s.setMemo(GETSTRING(memoCol));
  s.setAccountId(GETSTRING(accountIdCol));
  s.setCostCenterId(GETSTRING(costCenterIdCol));
  s.setNumber(GETSTRING(checkNumberCol));
  //s.setPostDate(GETDATETIME(postDateCol)); // FIXME - when Tom puts date into split object
  s.setBankID(GETSTRING(bankIdCol));

  return;
}

bool MyMoneyStorageSql::isReferencedByTransaction(const QString& id) const
{
  //FIXME-ALEX should I add sub query for kmmTagSplits here?
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare("SELECT COUNT(*) FROM kmmTransactions "
            "INNER JOIN kmmSplits ON kmmTransactions.id = kmmSplits.transactionId "
            "WHERE kmmTransactions.currencyId = :ID OR kmmSplits.payeeId = :ID "
            "OR kmmSplits.accountId = :ID OR kmmSplits.costCenterId = :ID");
  q.bindValue(":ID", id);
  if ((!q.exec()) || (!q.next())) { // krazy:exclude=crashy
    buildError(q, Q_FUNC_INFO, "error retrieving reference count");
    qFatal("Error retrieving reference count"); // definitely shouldn't happen
  }
  return (0 != q.value(0).toULongLong());
}

void MyMoneyStorageSql::readSchedules()
{

  try {
    m_storage->loadSchedules(fetchSchedules());
    m_storage->loadScheduleId(getNextScheduleId());
  } catch (const MyMoneyException &) {
    throw;
  }

}

const QMap<QString, MyMoneySchedule> MyMoneyStorageSql::fetchSchedules(const QStringList& idList, bool forUpdate) const
{
  int schedulesNb = (idList.isEmpty() ? m_schedules : idList.size());
  signalProgress(0, schedulesNb, QObject::tr("Loading schedules..."));
  int progress = 0;
  const MyMoneyDbTable& t = m_db.m_tables["kmmSchedules"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  QMap<QString, MyMoneySchedule> sList;
  //unsigned long lastId = 0;
  const MyMoneyDbTable& ts = m_db.m_tables["kmmSplits"];
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
    queryString += m_driver->forUpdateString();

  q.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.constEnd(); ++i, ++bindVal) {
      q.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Schedules"))); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int typeCol = t.fieldNumber("type");
  int occurenceCol = t.fieldNumber("occurence"); // krazy:exclude=spelling
  int occurenceMultiplierCol = t.fieldNumber("occurenceMultiplier");
  int paymentTypeCol = t.fieldNumber("paymentType");
  int startDateCol = t.fieldNumber("startDate");
  int endDateCol = t.fieldNumber("endDate");
  int fixedCol = t.fieldNumber("fixed");
  int autoEnterCol = t.fieldNumber("autoEnter");
  int lastPaymentCol = t.fieldNumber("lastPayment");
  int weekendOptionCol = t.fieldNumber("weekendOption");
  int nextPaymentDueCol = t.fieldNumber("nextPaymentDue");

  while (q.next()) {
    MyMoneySchedule s;
    QString boolChar;

    QString sId = GETSTRING(idCol);
    s.setName(GETSTRING(nameCol));
    s.setType(static_cast<Schedule::Type>(GETINT(typeCol)));
    s.setOccurrencePeriod(static_cast<Schedule::Occurrence>(GETINT(occurenceCol)));
    s.setOccurrenceMultiplier(GETINT(occurenceMultiplierCol));
    s.setPaymentType(static_cast<Schedule::PaymentType>(GETINT(paymentTypeCol)));
    s.setStartDate(GETDATE(startDateCol));
    s.setEndDate(GETDATE(endDateCol));
    boolChar = GETSTRING(fixedCol); s.setFixed(boolChar == "Y");
    boolChar = GETSTRING(autoEnterCol); s.setAutoEnter(boolChar == "Y");
    s.setLastPayment(GETDATE(lastPaymentCol));
    s.setWeekendOption(static_cast<Schedule::WeekendOption>(GETINT(weekendOptionCol)));
    QDate nextPaymentDue = GETDATE(nextPaymentDueCol);

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

    const MyMoneyDbTable& t = m_db.m_tables["kmmTransactions"];
    QSqlQuery q2(*const_cast <MyMoneyStorageSql*>(this));
    q2.prepare(t.selectAllString(false) + " WHERE id = :id;");
    q2.bindValue(":id", s.id());
    if (!q2.exec()) throw MYMONEYEXCEPTION(buildError(q2, Q_FUNC_INFO, QString("reading Scheduled Transaction"))); // krazy:exclude=crashy
    QSqlRecord rec = q2.record();
    if (!q2.next()) throw MYMONEYEXCEPTION(buildError(q2, Q_FUNC_INFO, QString("retrieving scheduled transaction")));
    MyMoneyTransaction tx(s.id(), MyMoneyTransaction());
    tx.setPostDate(getDate(q2.value(t.fieldNumber("postDate")).toString()));
    tx.setMemo(q2.value(t.fieldNumber("memo")).toString());
    tx.setEntryDate(getDate(q2.value(t.fieldNumber("entryDate")).toString()));
    tx.setCommodity(q2.value(t.fieldNumber("currencyId")).toString());
    tx.setBankID(q2.value(t.fieldNumber("bankId")).toString());

    qs.bindValue(":id", s.id());
    if (!qs.exec()) throw MYMONEYEXCEPTION(buildError(qs, Q_FUNC_INFO, "reading Scheduled Splits")); // krazy:exclude=crashy
    while (qs.next()) {
      MyMoneySplit sp;
      readSplit(sp, qs);
      tx.addSplit(sp);
    }
//    if (!m_payeeList.isEmpty())
//      readPayees(m_payeeList);
    // Process any key value pair
    tx.setPairs(readKeyValuePairs("TRANSACTION", s.id()).pairs());

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
    if (!sq.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading schedule payment history"))); // krazy:exclude=crashy
    while (sq.next()) s.recordPayment(sq.value(0).toDate());

    sList[s.id()] = s;

    //FIXME: enable when schedules have KVPs.
    //  s.setPairs(readKeyValuePairs("SCHEDULE", s.id()).pairs());

    //unsigned long id = extractId(s.id().data());
    //if(id > lastId)
    //  lastId = id;

    signalProgress(++progress, 0);
  }
  return sList;
}

void MyMoneyStorageSql::readSecurities()
{
  try {
    m_storage->loadSecurities(fetchSecurities());
    m_storage->loadSecurityId(getNextSecurityId());
  } catch (const MyMoneyException &) {
    throw;
  }

}

const QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchSecurities(const QStringList& /*idList*/, bool /*forUpdate*/) const
{
  signalProgress(0, m_securities, QObject::tr("Loading securities..."));
  int progress = 0;
  QMap<QString, MyMoneySecurity> sList;
  unsigned long lastId = 0;
  const MyMoneyDbTable& t = m_db.m_tables["kmmSecurities"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare(t.selectAllString(false) + " ORDER BY id;");
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Securities"))); // krazy:exclude=crashy
  int idCol = t.fieldNumber("id");
  int nameCol = t.fieldNumber("name");
  int symbolCol = t.fieldNumber("symbol");
  int typeCol = t.fieldNumber("type");
  int roundingMethodCol = t.fieldNumber("roundingMethod");
  int smallestAccountFractionCol = t.fieldNumber("smallestAccountFraction");
  int pricePrecisionCol = t.fieldNumber("pricePrecision");
  int tradingCurrencyCol = t.fieldNumber("tradingCurrency");
  int tradingMarketCol = t.fieldNumber("tradingMarket");

  while (q.next()) {
    MyMoneySecurity e;
    QString eid;
    eid = GETSTRING(idCol);
    e.setName(GETSTRING(nameCol));
    e.setTradingSymbol(GETSTRING(symbolCol));
    e.setSecurityType(static_cast<Security>(GETINT(typeCol)));
    e.setRoundingMethod(static_cast<AlkValue::RoundingMethod>(GETINT(roundingMethodCol)));
    int saf = GETINT(smallestAccountFractionCol);
    int pp = GETINT(pricePrecisionCol);
    e.setTradingCurrency(GETSTRING(tradingCurrencyCol));
    e.setTradingMarket(GETSTRING(tradingMarketCol));

    if (e.tradingCurrency().isEmpty())
      e.setTradingCurrency(m_storage->pairs()["kmm-baseCurrency"]);
    if (saf == 0)
      saf = 100;
    if (pp == 0 || pp > 10)
      pp = 4;
    e.setSmallestAccountFraction(saf);
    e.setPricePrecision(pp);

    // Process any key value pairs
    e.setPairs(readKeyValuePairs("SECURITY", eid).pairs());
    //tell the storage objects we have a new security object.

    // FIXME: Adapt to new interface make sure, to take care of the currencies as well
    //   see MyMoneyStorageXML::readSecurites()
    MyMoneySecurity security(eid, e);
    sList[security.id()] = security;

    unsigned long id = extractId(security.id());
    if (id > lastId)
      lastId = id;

    signalProgress(++progress, 0);
  }
  return sList;
}

void MyMoneyStorageSql::readPrices()
{
//  try {
//    m_storage->addPrice(MyMoneyPrice(from, to,  date, rate, source));
//  } catch (const MyMoneyException &) {
//    throw;
//  }
}

MyMoneyPrice MyMoneyStorageSql::fetchSinglePrice(const QString& fromId, const QString& toId, const QDate& date_, bool exactDate, bool /*forUpdate*/) const
{
  const MyMoneyDbTable& t = m_db.m_tables["kmmPrices"];

  static const int priceDateCol = t.fieldNumber("priceDate");
  static const int priceCol = t.fieldNumber("price");
  static const int priceSourceCol = t.fieldNumber("priceSource");

  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));

  // Use bind variables, instead of just inserting the values in the queryString,
  // so that values containing a ':' will work.
  // See balance query for why the date logic seems odd.
  QString queryString = t.selectAllString(false) +
                        " WHERE fromId = :fromId  AND toId = :toId AND priceDate < :priceDate ";

  if (exactDate)
    queryString += "AND priceDate > :exactDate ";

  queryString += "ORDER BY priceDate DESC;";

  q.prepare(queryString);

  QDate date(date_);

  if (!date.isValid())
    date = QDate::currentDate();

  q.bindValue(":fromId", fromId);
  q.bindValue(":toId", toId);
  q.bindValue(":priceDate", date.addDays(1).toString(Qt::ISODate));

  if (exactDate)
    q.bindValue(":exactDate", date.toString(Qt::ISODate));

  if (! q.exec()) return MyMoneyPrice(); // krazy:exclude=crashy

  if (q.next()) {

    return MyMoneyPrice(fromId,
                        toId,
                        GETDATE(priceDateCol),
                        MyMoneyMoney(GETSTRING(priceCol)),
                        GETSTRING(priceSourceCol));
  }

  return MyMoneyPrice();
}

const MyMoneyPriceList MyMoneyStorageSql::fetchPrices(const QStringList& fromIdList, const QStringList& toIdList, bool forUpdate) const
{
  int pricesNb = (fromIdList.isEmpty() ? m_prices : fromIdList.size());
  signalProgress(0, pricesNb, QObject::tr("Loading prices..."));
  int progress = 0;
  const_cast <MyMoneyStorageSql*>(this)->m_readingPrices = true;
  MyMoneyPriceList pList;
  const MyMoneyDbTable& t = m_db.m_tables["kmmPrices"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
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
    queryString += m_driver->forUpdateString();

  queryString += ';';

  q.prepare(queryString);

  if (! fromIdList.empty()) {
    QStringList::ConstIterator bindVal = fromIdList.constBegin();
    for (int i = 0; bindVal != fromIdList.constEnd(); ++i, ++bindVal) {
      q.bindValue(QString(":fromId%1").arg(i), *bindVal);
    }
  }
  if (! toIdList.empty()) {
    QStringList::ConstIterator bindVal = toIdList.constBegin();
    for (int i = 0; bindVal != toIdList.constEnd(); ++i, ++bindVal) {
      q.bindValue(QString(":toId%1").arg(i), *bindVal);
    }
  }

  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Prices"))); // krazy:exclude=crashy
  static const int fromIdCol = t.fieldNumber("fromId");
  static const int toIdCol = t.fieldNumber("toId");
  static const int priceDateCol = t.fieldNumber("priceDate");
  static const int priceCol = t.fieldNumber("price");
  static const int priceSourceCol = t.fieldNumber("priceSource");

  while (q.next()) {
    QString from = GETSTRING(fromIdCol);
    QString to = GETSTRING(toIdCol);
    QDate date = GETDATE(priceDateCol);

    pList [MyMoneySecurityPair(from, to)].insert(date, MyMoneyPrice(from, to, date, MyMoneyMoney(GETSTRING(priceCol)), GETSTRING(priceSourceCol)));
    signalProgress(++progress, 0);
  }
  const_cast <MyMoneyStorageSql*>(this)->m_readingPrices = false;

  return pList;
}

void MyMoneyStorageSql::readCurrencies()
{
  try {
    m_storage->loadCurrencies(fetchCurrencies());
  } catch (const MyMoneyException &) {
    throw;
  }
}

const QMap<QString, MyMoneySecurity> MyMoneyStorageSql::fetchCurrencies(const QStringList& idList, bool forUpdate) const
{
  int currenciesNb = (idList.isEmpty() ? m_currencies : idList.size());
  signalProgress(0, currenciesNb, QObject::tr("Loading currencies..."));
  int progress = 0;
  QMap<QString, MyMoneySecurity> cList;
  const MyMoneyDbTable& t = m_db.m_tables["kmmCurrencies"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));

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
    queryString += m_driver->forUpdateString();

  queryString += ';';

  q.prepare(queryString);

  if (! idList.empty()) {
    QStringList::ConstIterator bindVal = idList.constBegin();
    for (int i = 0; bindVal != idList.end(); ++i, ++bindVal) {
      q.bindValue(QString(":id%1").arg(i), *bindVal);
    }
  }

  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Currencies"))); // krazy:exclude=crashy
  int ISOcodeCol = t.fieldNumber("ISOcode");
  int nameCol = t.fieldNumber("name");
  int typeCol = t.fieldNumber("type");
  int symbol1Col = t.fieldNumber("symbol1");
  int symbol2Col = t.fieldNumber("symbol2");
  int symbol3Col = t.fieldNumber("symbol3");
  int smallestCashFractionCol = t.fieldNumber("smallestCashFraction");
  int smallestAccountFractionCol = t.fieldNumber("smallestAccountFraction");
  int pricePrecisionCol = t.fieldNumber("pricePrecision");

  while (q.next()) {
    QString id;
    MyMoneySecurity c;
    QChar symbol[3];
    id = GETSTRING(ISOcodeCol);
    c.setName(GETSTRING(nameCol));
    c.setSecurityType(static_cast<Security>(GETINT(typeCol)));
    symbol[0] = QChar(GETINT(symbol1Col));
    symbol[1] = QChar(GETINT(symbol2Col));
    symbol[2] = QChar(GETINT(symbol3Col));
    c.setSmallestCashFraction(GETINT(smallestCashFractionCol));
    c.setSmallestAccountFraction(GETINT(smallestAccountFractionCol));
    c.setPricePrecision(GETINT(pricePrecisionCol));
    c.setTradingSymbol(QString(symbol, 3).trimmed());

    cList[id] = MyMoneySecurity(id, c);

    signalProgress(++progress, 0);
  }
  return cList;
}

void MyMoneyStorageSql::readReports()
{
  try {
    m_storage->loadReports(fetchReports());
    m_storage->loadReportId(getNextReportId());
  } catch (const MyMoneyException &) {
    throw;
  }
}

const QMap<QString, MyMoneyReport> MyMoneyStorageSql::fetchReports(const QStringList& /*idList*/, bool /*forUpdate*/) const
{
  signalProgress(0, m_reports, QObject::tr("Loading reports..."));
  int progress = 0;
  const MyMoneyDbTable& t = m_db.m_tables["kmmReportConfig"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare(t.selectAllString(true));
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading reports"))); // krazy:exclude=crashy
  int xmlCol = t.fieldNumber("XML");
  QMap<QString, MyMoneyReport> rList;
  while (q.next()) {
    QDomDocument d;
    d.setContent(GETSTRING(xmlCol), false);

    QDomNode child = d.firstChild();
    child = child.firstChild();
    MyMoneyReport report;

    if (report.read(child.toElement()))
      rList[report.id()] = report;

    signalProgress(++progress, 0);
  }
  return rList;
}

const QMap<QString, MyMoneyBudget> MyMoneyStorageSql::fetchBudgets(const QStringList& idList, bool forUpdate) const
{
  int budgetsNb = (idList.isEmpty() ? m_budgets : idList.size());
  signalProgress(0, budgetsNb, QObject::tr("Loading budgets..."));
  int progress = 0;
  const MyMoneyDbTable& t = m_db.m_tables["kmmBudgetConfig"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  QString queryString(t.selectAllString(false));
  if (! idList.empty()) {
    queryString += " WHERE id = '" + idList.join("' OR id = '") + '\'';
  }
  if (forUpdate)
    queryString += m_driver->forUpdateString();

  queryString += ';';

  q.prepare(queryString);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading budgets"))); // krazy:exclude=crashy
  QMap<QString, MyMoneyBudget> budgets;
  int xmlCol = t.fieldNumber("XML");
  while (q.next()) {
    QDomDocument d;
    d.setContent(GETSTRING(xmlCol), false);

    QDomNode child = d.firstChild();
    child = child.firstChild();
    MyMoneyBudget budget(child.toElement());
    budgets.insert(budget.id(), budget);
    signalProgress(++progress, 0);
  }
  return budgets;
}

void MyMoneyStorageSql::readBudgets()
{
  m_storage->loadBudgets(fetchBudgets());
}

const MyMoneyKeyValueContainer MyMoneyStorageSql::readKeyValuePairs(const QString& kvpType, const QString& kvpId) const
{
  MyMoneyKeyValueContainer list;
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  q.prepare("SELECT kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type and kvpId = :id;");
  q.bindValue(":type", kvpType);
  q.bindValue(":id", kvpId);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Kvp for %1 %2").arg(kvpType) // krazy:exclude=crashy
                                          .arg(kvpId)));
  while (q.next()) list.setValue(q.value(0).toString(), q.value(1).toString());
  return (list);
}

const QHash<QString, MyMoneyKeyValueContainer> MyMoneyStorageSql::readKeyValuePairs(const QString& kvpType, const QStringList& kvpIdList) const
{
  QHash<QString, MyMoneyKeyValueContainer> retval;

  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));

  QString idList;
  if (!kvpIdList.empty()) {
    idList = QString(" and kvpId IN ('%1')").arg(kvpIdList.join("', '"));
  }

  QString query = QString("SELECT kvpId, kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type %1 order by kvpId;").arg(idList);

  q.prepare(query);
  q.bindValue(":type", kvpType);
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading Kvp List for %1").arg(kvpType))); // krazy:exclude=crashy

  // Reserve enough space for all values.
  retval.reserve(kvpIdList.size());

  // The loop below is designed to limit the number of calls to
  // QHash::operator[] in order to speed up calls to this function. This
  // assumes that QString::operator== is faster.
  /*
    if (q.next()) {
      QString oldkey = q.value(0).toString();
      MyMoneyKeyValueContainer& kvpc = retval[oldkey];

      kvpc.setValue(q.value(1).toString(), q.value(2).toString());

      while (q.next()) {
        if (q.value(0).toString() != oldkey) {
          oldkey = q.value(0).toString();
          kvpc = retval[oldkey];
        }
        kvpc.setValue(q.value(1).toString(), q.value(2).toString());
      }
    }
  */
  while (q.next()) {
    retval[q.value(0).toString()].setValue(q.value(1).toString(), q.value(2).toString());
  }
  return (retval);
}

template<long unsigned MyMoneyStorageSql::* cache>
long unsigned int MyMoneyStorageSql::getNextId(const QString& table, const QString& id, const int prefixLength) const
{
  Q_CHECK_PTR(cache);
  if (this->*cache == 0) {
    MyMoneyStorageSql* nonConstThis = const_cast<MyMoneyStorageSql*>(this);
    nonConstThis->*cache = 1 + nonConstThis->highestNumberFromIdString(table, id, prefixLength);
  }
  Q_ASSERT(this->*cache > 0); // everything else is never a valid id
  return this->*cache;
}

long unsigned MyMoneyStorageSql::getNextBudgetId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdBudgets>(QLatin1String("kmmBudgetConfig"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextAccountId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdAccounts>(QLatin1String("kmmAccounts"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextInstitutionId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdInstitutions>(QLatin1String("kmmInstitutions"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextPayeeId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdPayees>(QLatin1String("kmmPayees"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextTagId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdTags>(QLatin1String("kmmTags"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextReportId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdReports>(QLatin1String("kmmReportConfig"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextScheduleId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdSchedules>(QLatin1String("kmmSchedules"), QLatin1String("id"), 3);
}

long unsigned MyMoneyStorageSql::getNextSecurityId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdSecurities>(QLatin1String("kmmSecurities"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextTransactionId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdTransactions>(QLatin1String("kmmTransactions"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextOnlineJobId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdOnlineJobs>(QLatin1String("kmmOnlineJobs"), QLatin1String("id"), 1);
}

long unsigned MyMoneyStorageSql::getNextPayeeIdentifierId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdPayeeIdentifier>(QLatin1String("kmmPayeeIdentifier"), QLatin1String("id"), 5);
}

long unsigned int MyMoneyStorageSql::getNextCostCenterId() const
{
  return getNextId<&MyMoneyStorageSql::m_hiIdCostCenter>(QLatin1String("kmmCostCenterIdentifier"), QLatin1String("id"), 5);
}

long unsigned MyMoneyStorageSql::incrementBudgetId()
{
  m_hiIdBudgets = getNextBudgetId() + 1;
  return (m_hiIdBudgets - 1);
}

/**
 * @warning This method uses getNextAccountId() internaly. The database is not informed which can cause issues
 * when the database is accessed concurrently. Then maybe a single id is used twice but the RDBMS will detect the
 * issue and KMyMoney crashes. This issue can only occour when two instances of KMyMoney access the same database.
 * But in this unlikley case MyMoneyStorageSql will have a lot more issues, I think.
 */
long unsigned MyMoneyStorageSql::incrementAccountId()
{
  m_hiIdAccounts = getNextAccountId() + 1;
  return (m_hiIdAccounts - 1);
}

long unsigned MyMoneyStorageSql::incrementInstitutionId()
{
  m_hiIdInstitutions = getNextInstitutionId() + 1;
  return (m_hiIdInstitutions - 1);
}

long unsigned MyMoneyStorageSql::incrementPayeeId()
{
  m_hiIdPayees = getNextPayeeId() + 1;
  return (m_hiIdPayees - 1);
}

long unsigned MyMoneyStorageSql::incrementTagId()
{
  m_hiIdTags = getNextTagId() + 1;
  return (m_hiIdTags - 1);
}

long unsigned MyMoneyStorageSql::incrementReportId()
{
  m_hiIdReports = getNextReportId() + 1;
  return (m_hiIdReports - 1);
}

long unsigned MyMoneyStorageSql::incrementScheduleId()
{
  m_hiIdSchedules = getNextScheduleId() + 1;
  return (m_hiIdSchedules - 1);
}

long unsigned MyMoneyStorageSql::incrementSecurityId()
{
  m_hiIdSecurities = getNextSecurityId() + 1;
  return (m_hiIdSecurities - 1);
}

long unsigned MyMoneyStorageSql::incrementTransactionId()
{
  m_hiIdTransactions = getNextTransactionId() + 1;
  return (m_hiIdTransactions - 1);
}

long unsigned int MyMoneyStorageSql::incrementOnlineJobId()
{
  m_hiIdOnlineJobs = getNextOnlineJobId() + 1;
  return (m_hiIdOnlineJobs - 1);
}

long unsigned int MyMoneyStorageSql::incrementPayeeIdentfierId()
{
  m_hiIdPayeeIdentifier = getNextPayeeIdentifierId() + 1;
  return (m_hiIdPayeeIdentifier - 1);
}

long unsigned int MyMoneyStorageSql::incrementCostCenterId()
{
  m_hiIdCostCenter = getNextCostCenterId() + 1;
  return (m_hiIdCostCenter - 1);
}

void MyMoneyStorageSql::loadAccountId(const unsigned long& id)
{
  m_hiIdAccounts = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadTransactionId(const unsigned long& id)
{
  m_hiIdTransactions = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadPayeeId(const unsigned long& id)
{
  m_hiIdPayees = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadTagId(const unsigned long& id)
{
  m_hiIdTags = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadInstitutionId(const unsigned long& id)
{
  m_hiIdInstitutions = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadScheduleId(const unsigned long& id)
{
  m_hiIdSchedules = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadSecurityId(const unsigned long& id)
{
  m_hiIdSecurities = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadReportId(const unsigned long& id)
{
  m_hiIdReports = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadBudgetId(const unsigned long& id)
{
  m_hiIdBudgets = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadOnlineJobId(const long unsigned int& id)
{
  m_hiIdOnlineJobs = id;
  writeFileInfo();
}

void MyMoneyStorageSql::loadPayeeIdentifierId(const long unsigned int& id)
{
  m_hiIdPayeeIdentifier = id;
  writeFileInfo();
}

//****************************************************
long unsigned MyMoneyStorageSql::calcHighId(const long unsigned& i, const QString& id)
{
  QString nid = id;
  long unsigned high = (unsigned long) nid.remove(QRegExp("[A-Z]*")).toULongLong();
  return std::max(high, i);
}

void MyMoneyStorageSql::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageSql::signalProgress(int current, int total, const QString& msg) const
{
  if (m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

// **************************** Error display routine *******************************
QString& MyMoneyStorageSql::buildError(const QSqlQuery& q, const QString& function,
                                       const QString& messageb) const
{
  return (buildError(q, function, messageb, this));
}

QString& MyMoneyStorageSql::buildError(const QSqlQuery& q, const QString& function,
                                       const QString& message, const QSqlDatabase* db) const
{
  QString s = QString("Error in function %1 : %2").arg(function).arg(message);
  s += QString("\nDriver = %1, Host = %2, User = %3, Database = %4")
       .arg(db->driverName()).arg(db->hostName()).arg(db->userName()).arg(db->databaseName());
  QSqlError e = db->lastError();
  s += QString("\nDriver Error: %1").arg(e.driverText());
  s += QString("\nDatabase Error No %1: %2").arg(e.number()).arg(e.databaseText());
  s += QString("\nText: %1").arg(e.text());
  s += QString("\nError type %1").arg(e.type());
  e = q.lastError();
  s += QString("\nExecuted: %1").arg(q.executedQuery());
  s += QString("\nQuery error No %1: %2").arg(e.number()).arg(e.text());
  s += QString("\nError type %1").arg(e.type());

  const_cast <MyMoneyStorageSql*>(this)->m_error = s;
  qDebug("%s", qPrintable(s));
  const_cast <MyMoneyStorageSql*>(this)->cancelCommitUnit(function);
  return (const_cast <MyMoneyStorageSql*>(this)->m_error);
}

QDate MyMoneyStorageSql::m_startDate = QDate(1900, 1, 1);

void MyMoneyStorageSql::setStartDate(const QDate& startDate)
{
  m_startDate = startDate;
}

const QMap< QString, MyMoneyCostCenter > MyMoneyStorageSql::fetchCostCenters(const QStringList& idList, bool forUpdate) const
{
  Q_UNUSED(forUpdate);

  MyMoneyDbTransaction trans(const_cast <MyMoneyStorageSql&>(*this), Q_FUNC_INFO);
  if (m_displayStatus) {
    int costCenterNb = (idList.isEmpty() ? 100 : idList.size());
    signalProgress(0, costCenterNb, QObject::tr("Loading cost center..."));
  }
  int progress = 0;
  QMap<QString, MyMoneyCostCenter> costCenterList;
  //unsigned long lastId;
  const MyMoneyDbTable& t = m_db.m_tables["kmmCostCenter"];
  QSqlQuery q(*const_cast <MyMoneyStorageSql*>(this));
  if (idList.isEmpty()) {
    q.prepare(t.selectAllString());
  } else {
    QString whereClause = " where (";
    QString itemConnector = "";
    foreach (const QString& it, idList) {
      whereClause.append(QString("%1id = '%2'").arg(itemConnector).arg(it));
      itemConnector = " or ";
    }
    whereClause += ')';
    q.prepare(t.selectAllString(false) + whereClause);
  }
  if (!q.exec()) throw MYMONEYEXCEPTION(buildError(q, Q_FUNC_INFO, QString("reading CostCenter"))); // krazy:exclude=crashy
  const int idCol = t.fieldNumber("id");
  const int nameCol = t.fieldNumber("name");

  while (q.next()) {
    MyMoneyCostCenter costCenter;
    QString pid = GETSTRING(idCol);
    costCenter.setName(GETSTRING(nameCol));
    costCenterList[pid] = MyMoneyCostCenter(pid, costCenter);
    if (m_displayStatus) signalProgress(++progress, 0);
  }
  return costCenterList;
}
