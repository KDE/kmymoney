/***************************************************************************
                          kselectdatabasedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <kfiledialog.h>
// ----------------------------------------------------------------------------
// Project Includes

#include "kselectdatabasedlg.h"

KSelectDatabaseDlg::KSelectDatabaseDlg(QWidget *parent)
 : KSelectDatabaseDlgDecl(parent) {
  listDrivers->clear();
  // list drivers supported by KMM
  QMap<QString, QString> map = m_map.driverMap();
  // list drivers installed on system
  QStringList list = QSqlDatabase::drivers();
  if (list.count() == 0) {
    KMessageBox::error (0, i18n("There are no Qt SQL drivers installed in your system.\n"
        "Please consult documentation for your distro, or visit the Qt web site (www.trolltech.com)"
            " and search for SQL drivers."),
        "");
    setError();
  } else {
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
      QString dname = *it;
      if (map.keys().contains(dname)) { // only display if driver is supported
        dname = dname + " - " + map[dname];
        listDrivers->addItem (dname);
      }
      it++;
    }
    textDbName->setText ("KMyMoney");
    textHostName->setText ("localhost");
    textUserName->setText("");
    struct passwd * pwd = getpwuid(geteuid());
    if (pwd != 0)
      textUserName->setText (QString(pwd->pw_name));
    textPassword->setText ("");
    m_requiredFields = new kMandatoryFieldGroup(this);
    m_requiredFields->setOkButton(buttonOK);
    m_requiredFields->add(listDrivers);
    m_requiredFields->add(textDbName);
    connect (listDrivers, SIGNAL(itemClicked(QListWidgetItem *)),
           this, SLOT(slotDriverSelected(QListWidgetItem *)));
    connect (buttonSQL, SIGNAL(clicked()), this, SLOT(slotGenerateSQL()));
    connect (buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
    checkPreLoad->setChecked(false);
    buttonSQL->setEnabled(true);
  }
  connect (buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
   // ensure a driver gets selected; pre-select if only one
  if (listDrivers->count() == 1) {
    listDrivers->setCurrentItem(listDrivers->item(0));
    slotDriverSelected(listDrivers->item(0));
  }
}

KSelectDatabaseDlg::KSelectDatabaseDlg(KUrl openURL, QWidget *parent)
 : KSelectDatabaseDlgDecl(parent) {
  // here we are re-opening a database from a URL
  // probably taken from the last-used or recent file list
  listDrivers->clear();
  // check that the SQL driver is still available
  QString driverName = openURL.queryItem("driver");
  // list drivers installed on system
  QStringList list = QSqlDatabase::drivers();
  if (!list.contains(driverName)) {
    KMessageBox::error (0, i18n("Qt SQL driver %1 is no longer installed on your system",driverName),
        "");
        reject();
  }
  // check drivers supported by KMM
  QMap<QString, QString> map = m_map.driverMap();
  if (!map.contains(driverName)) {
    KMessageBox::error (0, i18n("Qt SQL driver %1 is not suported").arg(driverName),
        "");
        setError();
  } else if (!map.contains(driverName)) {
    KMessageBox::error (0, i18n("Qt SQL driver %1 is not suported").arg(driverName),
        "");
        setError();
  } else {
    // fill in the fixed data from the URL
    listDrivers->addItem (QString(driverName + " - " + map[driverName]));
    listDrivers->setCurrentItem(listDrivers->item(0));
    QString dbName = openURL.path().right(openURL.path().length() - 1); // remove separator slash
    textDbName->setText (dbName);
    textHostName->setText (openURL.host());
    textUserName->setText(openURL.user());
    // disable all but the password field, coz that's why we're here
    textDbName->setEnabled(false);
    listDrivers->setEnabled(false);
    textHostName->setEnabled(false);
    textUserName->setEnabled(false);
    textPassword->setEnabled(true);
    textPassword->setFocus();
    buttonSQL->setEnabled(false);
    // set password as required
    m_requiredFields = new kMandatoryFieldGroup(this);
    m_requiredFields->add(textPassword);
    m_requiredFields->setOkButton(buttonOK);

    connect (buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
    checkPreLoad->setChecked(false);
  }
  connect (buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
 }

KSelectDatabaseDlg::~KSelectDatabaseDlg() {
  if (m_requiredFields != 0) delete m_requiredFields;
}

void KSelectDatabaseDlg::setMode (int openMode) {
  m_mode = openMode;
  checkPreLoad->setEnabled (openMode == QIODevice::ReadWrite);
}

const KUrl KSelectDatabaseDlg::selectedURL() {
  KUrl url;
  url.setProtocol("sql");
  url.setUser(textUserName->text());
  url.setPass(textPassword->text());
  url.setHost(textHostName->text());
  url.setPath("/" + textDbName->text());
  QString qs = QString("driver=%1")
      .arg(listDrivers->currentItem()->text().section (' ', 0, 0));
  if (checkPreLoad->isChecked()) qs.append("&options=loadAll");
  if (!textPassword->text().isEmpty()) qs.append("&secure=yes");
  url.setQuery(qs);
  return (url);
}

void KSelectDatabaseDlg::slotDriverSelected (QListWidgetItem *driver) {
  databaseTypeE dbType = m_map.driverToType(driver->text().section(' ', 0, 0));
  if (!m_map.isTested(dbType)) {
    int rc = KMessageBox::warningContinueCancel (0,
       i18n("Database type %1 has not been fully tested in a KMyMoney environment.\n"
            "Please make sure you have adequate backups of your data.\n"
            "Please report any problems to the developer mailing list at "
            "kmymoney2-developer@lists.sourceforge.net")
           .arg(driver->text()),
        "");
    if (rc == KMessageBox::Cancel) {
      listDrivers->clearSelection();
      return;
    }
  }

  if (m_map.driverToType(driver->text().section(' ', 0, 0)) == Sqlite3){
    QString dbName;
    if (m_mode == QIODevice::WriteOnly)
      dbName = KFileDialog::getSaveFileName(
      KUrl(),
      i18n("*.sql *.*|SQLite files (*.sql)| All files (*.*)"),
      this,
      i18n("Select SQLite file"));
    else
      dbName = KFileDialog::getOpenFileName(
      KUrl(),
      i18n("*.sql *.*|SQLite files (*.sql)| All files (*.*)"),
      this,
      i18n("Select SQLite file"));
    if (dbName.isNull())
      return;

    textDbName->setText(dbName);
    // sqlite databases do not react to host/user/password; file system permissions must be used
    textHostName->setEnabled (false);
    textUserName->setEnabled (false);
    textPassword->setEnabled(false);
  } else {                         // not sqlite3
    textUserName->setEnabled (true);
    textHostName->setEnabled (true);
    textPassword->setEnabled(true);
  }
}

void KSelectDatabaseDlg::slotGenerateSQL () {
  QString fileName = KFileDialog::getSaveFileName(
      KUrl(),
      i18n("All files (*.*)"),
      this,
      i18n("Select output file"));
  if (fileName == "") return;
  QFile out(fileName);
  if (!out.open(QIODevice::WriteOnly)) return;
  QTextStream s(&out);
  MyMoneyDbDef db;
  s << db.generateSQL(listDrivers->currentItem()->text().section (' ', 0, 0));
  out.close();
}

void KSelectDatabaseDlg::slotHelp(void) {
  KToolInvocation::invokeHelp("details.database.selectdatabase");
}

void KSelectDatabaseDlg::setError() {
  buttonOK->setEnabled(false);
  buttonSQL->setEnabled(false);
  m_requiredFields = 0;
}

#include "kselectdatabasedlg.moc"
