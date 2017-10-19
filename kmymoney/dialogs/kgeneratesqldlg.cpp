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
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KUrlRequester>
#include <KHelpClient>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "storage/mymoneystoragesql.h"
#include "storage/mymoneyseqaccessmgr.h"
#include "kguiutils.h"

KGenerateSqlDlg::KGenerateSqlDlg(QWidget *)
{
  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
  m_createTablesButton = m_buttonBox->addButton(i18n("Create Tables"), QDialogButtonBox::ButtonRole::AcceptRole);
  m_saveSqlButton = m_buttonBox->addButton(i18n("Save SQL"), QDialogButtonBox::ButtonRole::ActionRole);
  Q_ASSERT(m_createTablesButton);
  Q_ASSERT(m_saveSqlButton);

  QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  m_widget = new KGenerateSqlDlgDecl();
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_widget);

  QWidget *mainWidget = new QWidget(this);
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);
  mainLayout->addWidget(m_buttonBox);
  initializeForm();
}

KGenerateSqlDlg::~KGenerateSqlDlg()
{
  // Stub required to delete m_requiredFields
}

void KGenerateSqlDlg::initializeForm()
{
  m_requiredFields = nullptr;
  // at this point, we don't know which fields are required, so disable everything but the list
  m_saveSqlButton->setEnabled(false);
  m_createTablesButton->setEnabled(false);
  QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okButton);
  okButton->setEnabled(false);

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

  connect(m_buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KGenerateSqlDlg::slotHelp);
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
      KHelpClient::invokeHelp("details.database.usage");
    }
    return (1);
  }
  m_widget->listDrivers->clear();
  m_widget->listDrivers->addItems(m_supportedDrivers);
  connect(m_widget->listDrivers, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotdriverSelected()));
  return (QDialog::exec());
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
    Q_ASSERT(!selectedDriver().isEmpty());

    QSqlDatabase dbase = QSqlDatabase::addDatabase(selectedDriver(), "creation");
    dbase.setHostName(m_widget->textHostName->text());
    dbase.setDatabaseName(m_dbName);
    dbase.setUserName(m_widget->textUserName->text());
    dbase.setPassword(m_widget->textPassword->text());
    if (!dbase.open()) {
      KMessageBox::error(this,
                         i18n("Unable to open database.\n"
                              "You must use an SQL CREATE DATABASE statement before creating the tables.\n")
                        );
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

  QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okButton);
  okButton->setEnabled(true);
}

void KGenerateSqlDlg::slotsaveSQL()
{
  QString fileName = QFileDialog::getSaveFileName(
                       this,
                       i18n("Select output file"),
                       QString(),
                       QString());
  if (fileName.isEmpty()) return;
  QFile out(fileName);
  if (!out.open(QIODevice::WriteOnly)) return;
  QTextStream s(&out);
  MyMoneyDbDef db;
  s << m_widget->textSQL->toPlainText();
  out.close();

  QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okButton);
  okButton->setEnabled(true);
}

QString KGenerateSqlDlg::selectedDriver()
{
  QList<QListWidgetItem *> drivers = m_widget->listDrivers->selectedItems();
  if (drivers.count() != 1) {
    return QString();
  }

  return drivers[0]->text().section(' ', 0, 0);
}

void KGenerateSqlDlg::slotdriverSelected()
{
  const QString driverName = selectedDriver();
  if (driverName.isEmpty()) {
    initializeForm();
    return;
  }

  m_dbDriver = MyMoneyDbDriver::create(driverName);
  if (!m_dbDriver->isTested()) {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
                  "Please make sure you have adequate backups of your data.\n"
                  "Please report any problems to the developer mailing list at "
                  "kmymoney-devel@kde.org", driverName),
             "");
    if (rc == KMessageBox::Cancel) {
      m_widget->listDrivers->clearSelection();
      initializeForm();
      return;
    }
  }

  m_requiredFields.reset(new kMandatoryFieldGroup(this));
  // currently, only sqlite need an external file
  if (m_dbDriver->requiresExternalFile()) {
    m_sqliteSelected = true;
    m_widget->urlSqlite->setMode(KFile::Mode::File);
    m_widget->urlSqlite->setEnabled(true);
    m_requiredFields->add(m_widget->urlSqlite);

    m_widget->textDbName->setEnabled(false);
    m_widget->textHostName->setEnabled(false);
    m_widget->textUserName->setEnabled(false);
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
    if (pwd != nullptr)
      m_widget->textUserName->setText(QString(pwd->pw_name));
    m_widget->textPassword->setText("");
  }

  m_widget->textPassword->setEnabled(m_dbDriver->isPasswordSupported());
  m_requiredFields->setOkButton(m_createTablesButton);
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

  m_saveSqlButton->setEnabled(true);
  connect(m_saveSqlButton, &QPushButton::clicked, this, &KGenerateSqlDlg::slotsaveSQL);
  connect(m_createTablesButton, &QPushButton::clicked, this, &KGenerateSqlDlg::slotcreateTables);
}

void KGenerateSqlDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.database.generatesql");
}
