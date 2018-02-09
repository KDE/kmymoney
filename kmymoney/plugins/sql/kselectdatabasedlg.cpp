/***************************************************************************
                          kselectdatabasedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>
                           (C) 2017 by Thomas Baumgart <tbaumgart@kde.org>
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

#include "kselectdatabasedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kselectdatabasedlg.h"
#include "kguiutils.h"
#include "mymoneystoragesql.h"
#include "mymoneydbdriver.h"
#include "misc/platformtools.h"


KSelectDatabaseDlg::KSelectDatabaseDlg(int openMode, QUrl openURL, QWidget *)
  : m_widget(new Ui::KSelectDatabaseDlg())
  , m_mode(openMode)
  , m_url(openURL)
  , m_requiredFields(new KMandatoryFieldGroup(this))
  , m_sqliteSelected(false)
{
  m_widget->setupUi(this);

  connect(m_widget->buttonBox, &QDialogButtonBox::accepted, this, &KSelectDatabaseDlg::accept);
  connect(m_widget->buttonBox, &QDialogButtonBox::rejected, this, &KSelectDatabaseDlg::reject);
  connect(m_widget->buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KSelectDatabaseDlg::slotHelp);

  m_requiredFields->setOkButton(m_widget->buttonBox->button(QDialogButtonBox::Ok));

  m_widget->checkPreLoad->setEnabled(openMode == QIODevice::ReadWrite);
}

KSelectDatabaseDlg::~KSelectDatabaseDlg()
{
}

bool KSelectDatabaseDlg::checkDrivers()
{
  QString driverName;
  if (m_url != QUrl()) {
    driverName = QUrlQuery(m_url).queryItemValue("driver");
  }

  // list drivers supported by KMM
  QMap<QString, QString> map = MyMoneyDbDriver::driverMap();
  // list drivers installed on system
  QStringList list = QSqlDatabase::drivers();

  // clear out the current list of drivers
  while(m_widget->databaseTypeCombo->count()) {
    m_widget->databaseTypeCombo->removeItem(0);
  }

  // join the two
  QStringList::Iterator it = list.begin();
  bool driverSupported = false;
  while (it != list.end()) {
    QString dname = *it;
    if (map.keys().contains(dname)) { // only keep if driver is supported
      m_widget->databaseTypeCombo->addItem(map[dname], dname);
      if (driverName == dname) {
        driverSupported = true;
      }
    }
    it++;
  }
  if (!driverName.isEmpty() && !driverSupported) {
    KMessageBox::error(0, i18n("Qt SQL driver %1 is no longer installed on your system", driverName),
                         "");
    return false;
  }

  if (m_widget->databaseTypeCombo->count() == 0) {
    // why does KMessageBox not have a standard dialog with Help button?
    if ((KMessageBox::questionYesNo(this,
                                    i18n("In order to use a database, you need to install some additional software. Click Help for more information"),
                                    i18n("No Qt SQL Drivers"),
                                    KStandardGuiItem::help(), KStandardGuiItem::cancel()))
        == KMessageBox::Yes) { // Yes stands in for help here
      KHelpClient::invokeHelp("details.database.usage");
    }
    return false;
  }
  return true;
}

int KSelectDatabaseDlg::exec()
{
  m_requiredFields->removeAll();
  if (m_url == QUrl()) {
    m_widget->textDbName->setText(QLatin1String("KMyMoney"));
    m_widget->textHostName->setText(QLatin1String("localhost"));
    m_widget->textUserName->setText(QString());
    m_widget->textUserName->setText(platformTools::osUsername());
    m_widget->textPassword->setText(QString());
    connect(m_widget->databaseTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KSelectDatabaseDlg::slotDriverSelected);
    m_widget->checkPreLoad->setChecked(false);
    // ensure a driver gets selected; pre-select the first one
    if (m_widget->databaseTypeCombo->count() != 0) {
      m_widget->databaseTypeCombo->setCurrentIndex(0);
      slotDriverSelected(0);
    }
  } else {
    // fill in the fixed data from the URL
    QString driverName = QUrlQuery(m_url).queryItemValue("driver");
    int idx = m_widget->databaseTypeCombo->findData(driverName);
    m_widget->databaseTypeCombo->setCurrentIndex(idx);
    QString dbName = m_url.path().right(m_url.path().length() - 1); // remove separator slash
    m_widget->textDbName->setText(dbName);
    m_widget->textHostName->setText(m_url.host());
    m_widget->textUserName->setText(m_url.userName());
    // disable all but the password field, coz that's why we're here
    m_widget->textDbName->setEnabled(false);
    m_widget->urlSqlite->setEnabled(false);
    m_widget->databaseTypeCombo->setEnabled(false);
    m_widget->textHostName->setEnabled(false);
    m_widget->textUserName->setEnabled(false);
    m_widget->textPassword->setEnabled(true);
    m_widget->textPassword->setFocus();
    // set password required
    m_requiredFields->add(m_widget->textPassword);

    m_widget->checkPreLoad->setChecked(false);
    m_sqliteSelected = !m_widget->urlSqlite->text().isEmpty();
  }

  return QDialog::exec();
}

const QUrl KSelectDatabaseDlg::selectedURL()
{
  QUrl url;
  url.setScheme("sql");
  url.setUserName(m_widget->textUserName->text());
  url.setPassword(m_widget->textPassword->text());
  url.setHost(m_widget->textHostName->text());
  if (m_sqliteSelected)
    url.setPath('/' + m_widget->urlSqlite->url().path());
  else
    url.setPath('/' + m_widget->textDbName->text());
  QString qs = QString("driver=%1")
               .arg(m_widget->databaseTypeCombo->currentData().toString());
  if (m_widget->checkPreLoad->isChecked())
    qs.append("&options=loadAll");
  if (!m_widget->textPassword->text().isEmpty())
    qs.append("&secure=yes");
  url.setQuery(qs);
  return (url);
}

void KSelectDatabaseDlg::slotDriverSelected(int idx)
{
  QExplicitlySharedDataPointer<MyMoneyDbDriver> dbDriver = MyMoneyDbDriver::create(m_widget->databaseTypeCombo->itemData(idx).toString());
  if (!dbDriver->isTested()) {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
                  "Please make sure you have adequate backups of your data.\n"
                  "Please report any problems to the developer mailing list at "
                  "kmymoney-devel@kde.org", m_widget->databaseTypeCombo->currentText()),
             "");
    if (rc == KMessageBox::Cancel) {
      return;
    }
  }

  m_requiredFields->removeAll();

  if (dbDriver->requiresExternalFile()) {
    // currently, only sqlite requres an external file
    m_sqliteSelected = true;
    if (m_mode == QIODevice::WriteOnly) {
      m_widget->urlSqlite->setMode(KFile::Mode::File);
    } else {
      m_widget->urlSqlite->setMode(KFile::Mode::File | KFile::Mode::ExistingOnly);
    }

    m_widget->textDbName->setEnabled(false);
    m_widget->urlSqlite->setEnabled(true);
    // sqlite databases do not react to host/user/password;
    // file system permissions must be used
    m_widget->textHostName->setEnabled(false);
    m_widget->textUserName->setEnabled(false);

    // setup required fields last because the required widgets must be enabled
    m_requiredFields->add(m_widget->urlSqlite);
  } else {                         // not sqlite3
    m_sqliteSelected = false;

    m_widget->textDbName->setEnabled(true);
    m_widget->urlSqlite->setEnabled(false);
    m_widget->textUserName->setEnabled(true);
    m_widget->textHostName->setEnabled(true);

    // setup required fields last because the required widgets must be enabled
    m_requiredFields->add(m_widget->textDbName);
    m_requiredFields->add(m_widget->textHostName);
    m_requiredFields->add(m_widget->textUserName);
  }
  m_widget->textPassword->setEnabled(dbDriver->isPasswordSupported());
}

void KSelectDatabaseDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.database.selectdatabase");
}
