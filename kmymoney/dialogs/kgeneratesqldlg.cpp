/***************************************************************************
                          kgeneratesqldlg.cpp
                             -------------------
    copyright            : (C) 2009 by Tony Bloomfield <tonybloom@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgeneratesqldlg.h"

// ----------------------------------------------------------------------------
// System includes

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTextStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// ----------------------------------------------------------------------------
// Project Includes

#define createTables User1
#define saveSQL User2

KGenerateSqlDlg::KGenerateSqlDlg(QWidget *)
{
  m_widget = new KGenerateSqlDlgDecl();
  setMainWidget(m_widget);
  setButtons(createTables | saveSQL | Ok | Cancel | Help);
  button(createTables)->setText(i18n("Create Tables"));
  button(saveSQL)->setText(i18n("Save SQL"));
  m_requiredFields = 0;
  initializeForm();
}

KGenerateSqlDlg::~KGenerateSqlDlg()
{
  delete m_requiredFields;
}

void KGenerateSqlDlg::initializeForm()
{
  delete m_requiredFields;
  m_requiredFields = 0;

  // at this point, we don't know which fields are required, so disable everything but the list
  button(saveSQL)->setEnabled(false);
  button(createTables)->setEnabled(false);
  enableButtonOk(false);
  m_widget->urlSqlite->clear();
  m_widget->textDbName->clear();
  m_widget->textHostName->clear();
  m_widget->textPassword->clear();
  m_widget->textUserName->clear();
  m_widget->textSQL->clear();
  m_widget->urlSqlite->setEnabled(false);
  m_widget->textDbName->setEnabled(false);
  m_widget->textHostName->setEnabled(false);
  m_widget->textPassword->setEnabled(false);
  m_widget->textUserName->setEnabled(false);
  m_widget->textSQL->setEnabled(false);
  connect(button(Help), SIGNAL(clicked()), this, SLOT(slotHelp()));
}

int  KGenerateSqlDlg::exec()
{
  // list drivers supported by KMM
  QMap<QString, QString> map = MyMoneyDbDriver::driverMap();
  // list drivers installed on system
  QStringList list = QSqlDatabase::drivers();
  // join the two
  QStringList::Iterator it = list.begin();
  while (it != list.end()) {
    QString dname = *it;
    if (map.keys().contains(dname)) { // only keep if driver is supported
      dname = dname + " - " + map[dname];
      m_supportedDrivers.append(dname);
    }
    ++it;
  }
  if (m_supportedDrivers.count() == 0) {
    // why does KMessageBox not have a standard dialog with Help button?
    if ((KMessageBox::questionYesNo(this,
                                    i18n("In order to use a database, you need to install some additional software. Click Help for more information"),
                                    i18n("No Qt SQL Drivers"),
                                    KStandardGuiItem::help(), KStandardGuiItem::cancel()))
        == KMessageBox::Yes) { // Yes stands in for help here
      KToolInvocation::invokeHelp("details.database.usage");
    }
    return (1);
  }
  m_widget->listDrivers->clear();
  m_widget->listDrivers->addItems(m_supportedDrivers);
  connect(m_widget->listDrivers, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotdriverSelected()));
  return (KDialog::exec());
}

void KGenerateSqlDlg::slotcreateTables()
{
  if (m_sqliteSelected) {
    m_dbName = m_widget->urlSqlite->text();
  } else {
    m_dbName = m_widget->textDbName->text();
  }
  // check that the database has been pre-created
  { // all queries etc. must be in a block - see 'remove database' API doc
    //FIXME: test the dbName
    QSqlDatabase dbase = QSqlDatabase::addDatabase(m_dbName, "creation");
    dbase.setHostName(m_widget->textHostName->text());
    dbase.setDatabaseName(m_dbName);
    dbase.setUserName(m_widget->textUserName->text());
    dbase.setPassword(m_widget->textPassword->text());
    if (!dbase.open()) {
      KMessageBox::error(this,
                         i18n("Unable to open database.\n"
                              "You must use an SQL CREATE DATABASE statement before creating the tables.\n"
                              "Click Help for more information."));
      return;
    }
    QSqlQuery q(dbase);
    QString message(i18n("Tables successfully created"));
    QStringList commands = m_widget->textSQL->toPlainText().split('\n');
    QStringList::ConstIterator cit;
    for (cit = commands.constBegin(); cit != commands.constEnd(); ++cit) {
      if (!(*cit).isEmpty()) {
        //qDebug() << "exec" << *cit;
        q.prepare(*cit);
        if (!q.exec()) {
          QSqlError e = q.lastError();
          message = i18n("Creation failed executing statement"
                         "\nExecuted: %1"
                         "\nError No %2: %3",
                         q.executedQuery(), e.number(), e.text());
          break;
        }
      }
    }
    KMessageBox::information(this, message);
  }
  QSqlDatabase::removeDatabase("creation");
  enableButtonOk(true);
}

void KGenerateSqlDlg::slotsaveSQL()
{
  QString fileName = KFileDialog::getSaveFileName(
                       KUrl(),
                       QString(),
                       this,
                       i18n("Select output file"));
  if (fileName.isEmpty()) return;
  QFile out(fileName);
  if (!out.open(QIODevice::WriteOnly)) return;
  QTextStream s(&out);
  MyMoneyDbDef db;
  s << m_widget->textSQL->toPlainText();
  out.close();
  enableButtonOk(true);
}

void KGenerateSqlDlg::slotdriverSelected()
{
  QList<QListWidgetItem *> drivers = m_widget->listDrivers->selectedItems();
  if (drivers.count() != 1) {
    initializeForm();
    return;
  }

  m_dbDriver = MyMoneyDbDriver::create(drivers[0]->text().section(' ', 0, 0));
  if (!m_dbDriver->isTested()) {
    int rc = KMessageBox::warningContinueCancel(nullptr,
             i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
                  "Please make sure you have adequate backups of your data.\n"
                  "Please report any problems to the developer mailing list at "
                  "kmymoney-devel@kde.org", m_dbDriver),
             "");
    if (rc == KMessageBox::Cancel) {
      m_widget->listDrivers->clearSelection();
      initializeForm();
      return;
    }
  }

  m_requiredFields = new kMandatoryFieldGroup(this);
  // currently, only sqlite need an external file
  if (m_dbDriver->requiresExternalFile()) {
    m_sqliteSelected = true;
    m_widget->urlSqlite->setMode(KFile::Modes(KFile::File));
    m_widget->urlSqlite->setEnabled(true);
    m_requiredFields->add(m_widget->urlSqlite);
  } else {                         // not sqlite3
    m_sqliteSelected = false;
    m_widget->urlSqlite->setEnabled(false);
    m_widget->textDbName->setEnabled(true);
    m_widget->textHostName->setEnabled(true);
    m_widget->textUserName->setEnabled(true);
    m_requiredFields->add(m_widget->textDbName);
    m_requiredFields->add(m_widget->textHostName);
    m_requiredFields->add(m_widget->textUserName);
    m_widget->textDbName->setText("KMyMoney");
    m_widget->textHostName->setText("localhost");
    m_widget->textUserName->setText("");
    struct passwd * pwd = getpwuid(geteuid());
    if (pwd != 0)
      m_widget->textUserName->setText(QString(pwd->pw_name));
    m_widget->textPassword->setText("");
  }

  m_widget->textPassword->setEnabled(m_dbDriver->isPasswordSupported());
  m_requiredFields->setOkButton(button(createTables));
  m_widget->textSQL->setEnabled(true);
  // check if we have a storage; if not, create a skeleton one
  // we need a storage for MyMoneyDbDef to generate standard accounts
  m_storage = new MyMoneySeqAccessMgr;
  m_mustDetachStorage = true;
  try {
    MyMoneyFile::instance()->attachStorage(m_storage);
  } catch (const MyMoneyException &) {
    m_mustDetachStorage = false; // there is already a storage attached
  }
  MyMoneyDbDef db;
  m_widget->textSQL->setText
  (db.generateSQL(m_dbDriver));
  if (m_mustDetachStorage) {
    MyMoneyFile::instance()->detachStorage();
  }
  delete m_storage;
  button(saveSQL)->setEnabled(true);
  connect(button(saveSQL), SIGNAL(clicked()), this, SLOT(slotsaveSQL()));
  connect(button(createTables), SIGNAL(clicked()), this, SLOT(slotcreateTables()));
}

void KGenerateSqlDlg::slotHelp()
{
  KToolInvocation::invokeHelp("details.database.generatesql");
}
