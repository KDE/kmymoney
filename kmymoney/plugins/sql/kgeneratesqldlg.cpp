/***************************************************************************
                          kgeneratesqldlg.cpp
                             -------------------
    copyright            : (C) 2009 by Tony Bloomfield <tonybloom@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "kgeneratesqldlg.h"

// ----------------------------------------------------------------------------
// Std Includes

#include <memory>

// ----------------------------------------------------------------------------
// System includes

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

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneratesqldlg.h"

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "storage/mymoneystoragemgr.h"
#include "kguiutils.h"
#include "misc/platformtools.h"
#include "mymoneydbdriver.h"
#include "mymoneydbdef.h"

class KGenerateSqlDlgPrivate
{
  Q_DISABLE_COPY(KGenerateSqlDlgPrivate)
  Q_DECLARE_PUBLIC(KGenerateSqlDlg)

public:
  explicit KGenerateSqlDlgPrivate(KGenerateSqlDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KGenerateSqlDlg),
    m_createTablesButton(nullptr),
    m_saveSqlButton(nullptr),
    m_sqliteSelected(false),
    m_storage(nullptr),
    m_mustDetachStorage(false)
  {
  }

  ~KGenerateSqlDlgPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KGenerateSqlDlg);
    ui->setupUi(q);
    m_createTablesButton = ui->buttonBox->addButton(i18n("Create Tables"), QDialogButtonBox::ButtonRole::AcceptRole);
    m_saveSqlButton = ui->buttonBox->addButton(i18n("Save SQL"), QDialogButtonBox::ButtonRole::ActionRole);
    Q_ASSERT(m_createTablesButton);
    Q_ASSERT(m_saveSqlButton);

    q->connect(ui->buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept);
    q->connect(ui->buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    initializeForm();
  }

  void initializeForm()
  {
    Q_Q(KGenerateSqlDlg);
    m_requiredFields = nullptr;
    // at this point, we don't know which fields are required, so disable everything but the list
    m_saveSqlButton->setEnabled(false);
    m_createTablesButton->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->urlSqlite->clear();
    ui->textDbName->clear();
    ui->textHostName->clear();
    ui->textPassword->clear();
    ui->textUserName->clear();
    ui->textSQL->clear();
    ui->urlSqlite->setEnabled(false);
    ui->textDbName->setEnabled(false);
    ui->textHostName->setEnabled(false);
    ui->textPassword->setEnabled(false);
    ui->textUserName->setEnabled(false);
    ui->textSQL->setEnabled(false);

    q->connect(ui->buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, q, &KGenerateSqlDlg::slotHelp);
  }

  QString selectedDriver()
  {
    auto drivers = ui->listDrivers->selectedItems();
    if (drivers.count() != 1) {
      return QString();
    }

    return drivers[0]->text().section(' ', 0, 0);
  }

  KGenerateSqlDlg      *q_ptr;
  Ui::KGenerateSqlDlg  *ui;
  QPushButton          *m_createTablesButton;
  QPushButton          *m_saveSqlButton;

  QList<QString> m_supportedDrivers;
  //MyMoneyDbDrivers m_map;
  std::unique_ptr<KMandatoryFieldGroup> m_requiredFields;
  bool m_sqliteSelected;
  QExplicitlySharedDataPointer<MyMoneyDbDriver> m_dbDriver;
  QString m_dbName;
  MyMoneyStorageMgr* m_storage;
  bool m_mustDetachStorage;
};


KGenerateSqlDlg::KGenerateSqlDlg(QWidget *parent) :
  QDialog(parent),
  d_ptr(new KGenerateSqlDlgPrivate(this))
{
  Q_D(KGenerateSqlDlg);
  d->init();
}

KGenerateSqlDlg::~KGenerateSqlDlg()
{
  Q_D(KGenerateSqlDlg);
  delete d;
}

int  KGenerateSqlDlg::exec()
{
  Q_D(KGenerateSqlDlg);
  // list drivers supported by KMM
  auto map = MyMoneyDbDriver::driverMap();
  // list drivers installed on system
  auto list = QSqlDatabase::drivers();
  // join the two
  QStringList::Iterator it = list.begin();
  while (it != list.end()) {
    QString dname = *it;
    if (map.keys().contains(dname)) { // only keep if driver is supported
      dname = dname + " - " + map[dname];
      d->m_supportedDrivers.append(dname);
    }
    ++it;
  }
  if (d->m_supportedDrivers.count() == 0) {
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
  d->ui->listDrivers->clear();
  d->ui->listDrivers->addItems(d->m_supportedDrivers);
  connect(d->ui->listDrivers, &QListWidget::itemSelectionChanged,
          this, &KGenerateSqlDlg::slotdriverSelected);
  return (QDialog::exec());
}

void KGenerateSqlDlg::slotcreateTables()
{
  Q_D(KGenerateSqlDlg);
  if (d->m_sqliteSelected) {
    d->m_dbName = d->ui->urlSqlite->text();
  } else {
    d->m_dbName = d->ui->textDbName->text();
  }
  // check that the database has been pre-created
  { // all queries etc. must be in a block - see 'remove database' API doc
    Q_ASSERT(!d->selectedDriver().isEmpty());

    QSqlDatabase dbase = QSqlDatabase::addDatabase(d->selectedDriver(), "creation");
    dbase.setHostName(d->ui->textHostName->text());
    dbase.setDatabaseName(d->m_dbName);
    dbase.setUserName(d->ui->textUserName->text());
    dbase.setPassword(d->ui->textPassword->text());
    if (!dbase.open()) {
      KMessageBox::error(this,
                         i18n("Unable to open database.\n"
                              "You must use an SQL CREATE DATABASE statement before creating the tables.\n")
                        );
      return;
    }
    QSqlQuery q(dbase);
    QString message(i18n("Tables successfully created"));
    QStringList commands = d->ui->textSQL->toPlainText().split('\n');
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

  auto okButton = d->ui->buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okButton);
  okButton->setEnabled(true);
}

void KGenerateSqlDlg::slotsaveSQL()
{
  Q_D(KGenerateSqlDlg);
  auto fileName = QFileDialog::getSaveFileName(
        this,
        i18n("Select output file"),
        QString(),
        QString());
  if (fileName.isEmpty()) return;
  QFile out(fileName);
  if (!out.open(QIODevice::WriteOnly)) return;
  QTextStream s(&out);
  MyMoneyDbDef db;
  s << d->ui->textSQL->toPlainText();
  out.close();

  auto okButton = d->ui->buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okButton);
  okButton->setEnabled(true);
}

void KGenerateSqlDlg::slotdriverSelected()
{
  Q_D(KGenerateSqlDlg);
  const auto driverName = d->selectedDriver();
  if (driverName.isEmpty()) {
    d->initializeForm();
    return;
  }

  d->m_dbDriver = MyMoneyDbDriver::create(driverName);
  if (!d->m_dbDriver->isTested()) {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
                  "Please make sure you have adequate backups of your data.\n"
                  "Please report any problems to the developer mailing list at "
                  "kmymoney-devel@kde.org", driverName),
             "");
    if (rc == KMessageBox::Cancel) {
      d->ui->listDrivers->clearSelection();
      d->initializeForm();
      return;
    }
  }

  d->m_requiredFields.reset(new KMandatoryFieldGroup(this));
  // currently, only sqlite need an external file
  if (d->m_dbDriver->requiresExternalFile()) {
    d->m_sqliteSelected = true;
    d->ui->urlSqlite->setMode(KFile::Mode::File);
    d->ui->urlSqlite->setEnabled(true);
    d->m_requiredFields->add(d->ui->urlSqlite);

    d->ui->textDbName->setEnabled(false);
    d->ui->textHostName->setEnabled(false);
    d->ui->textUserName->setEnabled(false);
  } else {                         // not sqlite3
    d->m_sqliteSelected = false;
    d->ui->urlSqlite->setEnabled(false);
    d->ui->textDbName->setEnabled(true);
    d->ui->textHostName->setEnabled(true);
    d->ui->textUserName->setEnabled(true);
    d->m_requiredFields->add(d->ui->textDbName);
    d->m_requiredFields->add(d->ui->textHostName);
    d->m_requiredFields->add(d->ui->textUserName);
    d->ui->textDbName->setText("KMyMoney");
    d->ui->textHostName->setText("localhost");
    d->ui->textUserName->setText("");
    d->ui->textUserName->setText(platformTools::osUsername());
    d->ui->textPassword->setText("");
  }

  d->ui->textPassword->setEnabled(d->m_dbDriver->isPasswordSupported());
  d->m_requiredFields->setOkButton(d->m_createTablesButton);
  d->ui->textSQL->setEnabled(true);
  // check if we have a storage; if not, create a skeleton one
  // we need a storage for MyMoneyDbDef to generate standard accounts
  d->m_storage = new MyMoneyStorageMgr;
  d->m_mustDetachStorage = true;
  try {
    MyMoneyFile::instance()->attachStorage(d->m_storage);
  } catch (const MyMoneyException &) {
    d->m_mustDetachStorage = false; // there is already a storage attached
  }
  MyMoneyDbDef db;
  d->ui->textSQL->setText
  (db.generateSQL(d->m_dbDriver));
  if (d->m_mustDetachStorage) {
    MyMoneyFile::instance()->detachStorage();
  }
  delete d->m_storage;

  d->m_saveSqlButton->setEnabled(true);
  connect(d->m_saveSqlButton, &QPushButton::clicked, this, &KGenerateSqlDlg::slotsaveSQL);
  connect(d->m_createTablesButton, &QPushButton::clicked, this, &KGenerateSqlDlg::slotcreateTables);
}

void KGenerateSqlDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.database.generatesql");
}
