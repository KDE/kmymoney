/***************************************************************************
                          kselectdatabasedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>

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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <QPushButton>
#include <QApplication>
#include <QSqlDatabase>
#include <QStatusBar>
#include <QCheckBox>
#include <QColor>
#include <QTextStream>
#include <QUrlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kurlrequester.h>
#include <QTextBrowser>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <khelpclient.h>
#include <khelpclient.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// Project Includes

KSelectDatabaseDlg::KSelectDatabaseDlg(int openMode, QUrl openURL, QWidget *)
{
  m_widget = new KSelectDatabaseDlgDecl();
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_widget);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
  QWidget *mainWidget = new QWidget(this);
  mainLayout->addWidget(mainWidget);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotHelp()));
  m_requiredFields = 0;
  m_url = openURL;
  m_mode = openMode;
  m_sqliteSelected = false;
  m_widget->checkPreLoad->setEnabled(openMode == QIODevice::ReadWrite);
}

KSelectDatabaseDlg::~KSelectDatabaseDlg()
{
  delete m_requiredFields;
}

bool KSelectDatabaseDlg::checkDrivers()
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
    it  ++;
  }
  if (m_url != QUrl()) {
    QString driverName = QUrlQuery(m_url).queryItemValue("driver");
    //qDebug() << driverName;
    //qDebug() << m_supportedDrivers;
    if (!m_supportedDrivers.join(",").contains(driverName)) {
      KMessageBox::error(0, i18n("Qt SQL driver %1 is no longer installed on your system", driverName),
                         "");
      return (false);
    }
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
    return(false);
  }
  return (true);
}

int KSelectDatabaseDlg::exec()
{
  m_widget->listDrivers->clear();
  if (m_url == QUrl()) {
    m_widget->listDrivers->addItems(m_supportedDrivers);
    m_widget->textDbName->setText("KMyMoney");
    m_widget->textHostName->setText("localhost");
    m_widget->textUserName->setText("");
    struct passwd * pwd = getpwuid(geteuid());
    if (pwd != 0)
      m_widget->textUserName->setText(QString(pwd->pw_name));
    m_widget->textPassword->setText("");
    m_requiredFields = new kMandatoryFieldGroup(this);
    // TODO: port to kf5
    //m_requiredFields->setOkButton(button(QDialogButtonBox::Ok));
    m_requiredFields->add(m_widget->listDrivers);
    m_requiredFields->add(m_widget->textDbName);
    connect(m_widget->listDrivers, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotDriverSelected(QListWidgetItem*)));
    m_widget->checkPreLoad->setChecked(false);
    // ensure a driver gets selected; pre-select if only one
    if (m_widget->listDrivers->count() == 1) {
      m_widget->listDrivers->setCurrentItem(m_widget->listDrivers->item(0));
      slotDriverSelected(m_widget->listDrivers->item(0));
    }
  } else {
    // fill in the fixed data from the URL
    QString driverName = QUrlQuery(m_url).queryItemValue("driver");
    m_widget->listDrivers->addItem(QString(driverName + " - " + MyMoneyDbDriver::driverMap()[driverName]));
    m_widget->listDrivers->setCurrentItem(m_widget->listDrivers->item(0));
    QString dbName = m_url.path().right(m_url.path().length() - 1); // remove separator slash
    m_widget->textDbName->setText(dbName);
    m_widget->textHostName->setText(m_url.host());
    m_widget->textUserName->setText(m_url.userName());
    // disable all but the password field, coz that's why we're here
    m_widget->textDbName->setEnabled(false);
    m_widget->urlSqlite->setEnabled(false);
    m_widget->listDrivers->setEnabled(false);
    m_widget->textHostName->setEnabled(false);
    m_widget->textUserName->setEnabled(false);
    m_widget->textPassword->setEnabled(true);
    m_widget->textPassword->setFocus();
    // set password required
    m_requiredFields = new kMandatoryFieldGroup(this);
    m_requiredFields->add(m_widget->textPassword);
    // TODO: port to kf5
    //m_requiredFields->setOkButton(button(QDialog::Ok));

    m_widget->checkPreLoad->setChecked(false);
    m_sqliteSelected = !m_widget->urlSqlite->text().isEmpty();
  }

  return (QDialog::exec());
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
               .arg(m_widget->listDrivers->currentItem()->text().section(' ', 0, 0));
  if (m_widget->checkPreLoad->isChecked()) qs.append("&options=loadAll");
  if (!m_widget->textPassword->text().isEmpty()) qs.append("&secure=yes");
  url.setQuery(qs);
  return (url);
}

void KSelectDatabaseDlg::slotDriverSelected(QListWidgetItem *driver)
{
  QExplicitlySharedDataPointer<MyMoneyDbDriver> dbDriver = MyMoneyDbDriver::create(driver->text().section(' ', 0, 0));
  if (!dbDriver->isTested()) {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
                  "Please make sure you have adequate backups of your data.\n"
                  "Please report any problems to the developer mailing list at "
                  "kmymoney-devel@kde.org", driver->text()),
             "");
    if (rc == KMessageBox::Cancel) {
      m_widget->listDrivers->clearSelection();
      return;
    }
  }

  if (dbDriver->requiresExternalFile()) {
    // currently, only sqlite requres an external file
    m_sqliteSelected = true;
    if (m_mode == QIODevice::WriteOnly)
      m_widget->urlSqlite->setMode(KFile::Modes(KFile::Files));
    else
      m_widget->urlSqlite->setMode(KFile::Modes(KFile::Files | KFile::ExistingOnly));

    m_requiredFields->remove(m_widget->textDbName);
    m_requiredFields->add(m_widget->urlSqlite);
    m_widget->textDbName->setEnabled(false);
    m_widget->urlSqlite->setEnabled(true);
    // sqlite databases do not react to host/user/password;
    // file system permissions must be used
    m_widget->textHostName->setEnabled(false);
    m_widget->textUserName->setEnabled(false);
  } else {                         // not sqlite3
    m_sqliteSelected = false;
    m_requiredFields->add(m_widget->textDbName);
    m_requiredFields->remove(m_widget->urlSqlite);
    m_widget->textDbName->setEnabled(true);
    m_widget->urlSqlite->setEnabled(false);
    m_widget->textUserName->setEnabled(true);
    m_widget->textHostName->setEnabled(true);
  }
  m_widget->textPassword->setEnabled(dbDriver->isPasswordSupported());
}

void KSelectDatabaseDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.database.selectdatabase");
}
