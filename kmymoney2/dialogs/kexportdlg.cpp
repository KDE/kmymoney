/***************************************************************************
                          kexportdlg.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Headers

#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstandarddirs.h>
#endif
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "kexportdlg.h"
#include "mymoneycategory.h"
#include "mymoneyqifprofileeditor.h"
#include "mymoneyfile.h"
#include "kmymoneyaccountcombo.h"
#include "../kmymoneyutils.h"

KExportDlg::KExportDlg(QWidget *parent)
  : KExportDlgDecl(parent)
{
  // Set (almost) all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();

  // load button icons
  KIconLoader* il = KIconLoader::global();
  m_qbuttonCancel->setGuiItem(KStandardGuiItem::cancel());

  KGuiItem okButtenItem( i18n( "&Export" ),
                      KIcon(il->loadIcon("fileexport", KIconLoader::Small, KIconLoader::SizeSmall)),
                      i18n("Start operation"),
                      i18n("Use this to start the export operation"));
  m_qbuttonOk->setGuiItem(okButtenItem);

  KGuiItem browseButtenItem( i18n( "&Browse..." ),
                      KIcon(il->loadIcon("fileopen", KIconLoader::Small, KIconLoader::SizeSmall)),
                      i18n("Select filename"),
                      i18n("Use this to select a filename to export to"));
  m_qbuttonBrowse->setGuiItem(browseButtenItem);

  KGuiItem newButtenItem( i18n( "&New..." ),
                      KIcon(il->loadIcon("filenew", KIconLoader::Small, KIconLoader::SizeSmall)),
                      i18n("Create a new profile"),
                      i18n("Use this to open the profile editor"));
  m_profileEditorButton->setGuiItem(newButtenItem);


  // connect the buttons to their functionality
  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
  connect(m_qcheckboxAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_qcheckboxCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_accountComboBox, SIGNAL(accountSelected(const QString&)), this, SLOT(checkData(const QString&)));
  connect(m_profileComboBox, SIGNAL(highlighted(int)), this, SLOT(checkData()));
  connect(m_kmymoneydateStart, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));
  connect(m_kmymoneydateEnd, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));

  checkData(QString());
}

KExportDlg::~KExportDlg()
{
}

void KExportDlg::slotBrowse()
{
  QString newName(KFileDialog::getSaveFileName(KUrl(),"*.QIF", this));
  KMyMoneyUtils::appendCorrectFileExt(newName, QString("qif"));
  if (!newName.isEmpty())
    m_qlineeditFile->setText(newName);
}

void KExportDlg::slotNewProfile(void)
{
  MyMoneyQifProfileEditor* editor = new MyMoneyQifProfileEditor(true, this);
  editor->setObjectName( "QIF Profile Editor");
  if(editor->exec()) {
    m_profileComboBox->setCurrentText(editor->selectedProfile());
    loadProfiles();
  }
  delete editor;
}

void KExportDlg::loadProfiles(const bool selectLast)
{
  // Creating an editor object here makes sure that
  // we have at least the default profile available
  MyMoneyQifProfileEditor* edit = new MyMoneyQifProfileEditor(true, 0);
  edit->slotOk();
  delete edit;

  QString current = m_profileComboBox->currentText();

  m_profileComboBox->clear();

  QStringList list;
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Profiles");

  list = grp.readEntry("profiles",QStringList());
  list.sort();
  m_profileComboBox->insertStringList(list);

  if(selectLast == true) {
    grp = config->group("Last Use Settings");
    current = grp.readEntry("KExportDlg_LastProfile");
  }

  m_profileComboBox->setCurrentItem(0);
  if(list.contains(current) > 0)
    m_profileComboBox->setCurrentText(current);
}

void KExportDlg::slotOkClicked()
{
  // Make sure we save the last used settings for use next time,
  writeConfig();
  accept();
}

void KExportDlg::readConfig(void)
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup kgrp = kconfig->group("Last Use Settings");
  m_qlineeditFile->setText(kgrp.readEntry("KExportDlg_LastFile"));
  m_qcheckboxAccount->setChecked(kgrp.readEntry("KExportDlg_AccountOpt", true));
  m_qcheckboxCategories->setChecked(kgrp.readEntry("KExportDlg_CatOpt", true));
#warning "port to kde4"
  //m_kmymoneydateStart->setDate(kgrp.readEntry("KExportDlg_StartDate").date());
  //m_kmymoneydateEnd->setDate(kgrp.readEntry("KExportDlg_EndDate").date());
  // m_profileComboBox is loaded in loadProfiles(), so we don't worry here
  // m_accountComboBox is loaded in loadAccounts(), so we don't worry here
}

void KExportDlg::writeConfig(void)
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group("Last Use Settings");
  grp.writeEntry("KExportDlg_LastFile", m_qlineeditFile->text());
  grp.writeEntry("KExportDlg_AccountOpt", m_qcheckboxAccount->isChecked());
  grp.writeEntry("KExportDlg_CatOpt", m_qcheckboxCategories->isChecked());
  grp.writeEntry("KExportDlg_StartDate", QDateTime(m_kmymoneydateStart->date()));
  grp.writeEntry("KExportDlg_EndDate", QDateTime(m_kmymoneydateEnd->date()));
  grp.writeEntry("KExportDlg_LastProfile", m_profileComboBox->currentText());
  kconfig->sync();
}

void KExportDlg::checkData(const QString& accountId)
{
  bool  okEnabled = false;

  if(!m_qlineeditFile->text().isEmpty()) {
    QString strFile(m_qlineeditFile->text());
    if(KMyMoneyUtils::appendCorrectFileExt(strFile, QString("qif")))
      m_qlineeditFile->setText(strFile);
  }

  MyMoneyAccount account;
  if(!accountId.isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    account = file->account(accountId);
    if(m_lastAccount != accountId) {
      MyMoneyTransactionFilter filter(accountId);
      Q3ValueList<MyMoneyTransaction> list = file->transactionList(filter);
      Q3ValueList<MyMoneyTransaction>::Iterator it;

      if(!list.isEmpty()) {
        it = list.begin();
        m_kmymoneydateStart->loadDate((*it).postDate());
        it = list.end();
        --it;
        m_kmymoneydateEnd->loadDate((*it).postDate());
      }
      m_lastAccount = accountId;
      m_accountComboBox->setSelected(account);
    }
  }

  if(!m_qlineeditFile->text().isEmpty()
  && m_accountComboBox->selectedAccounts().count() != 0
  && !m_profileComboBox->currentText().isEmpty()
  && m_kmymoneydateStart->date() <= m_kmymoneydateEnd->date()
  && (m_qcheckboxAccount->isChecked() || m_qcheckboxCategories->isChecked()))
    okEnabled = true;

  m_qbuttonOk->setEnabled(okEnabled);
}

void KExportDlg::loadAccounts(void)
{
/*
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    addCategories(strList, file->liability().id(), QString());
    addCategories(strList, file->asset().id(), QString());

  } catch (MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KExportDlg::loadAccounts:%d",
      e->what().toLatin1(), e->file().toLatin1(), e->line(), __LINE__);
    delete e;
  }
*/
  m_accountComboBox->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

/*
  m_accountComboBox->setCurrentItem(0);
  if(strList.contains(current) > 0)
    m_accountComboBox->setCurrentText(current);
*/
}

QString KExportDlg::accountId() const
{
  return m_lastAccount;
}

/*
void KExportDlg::addCategories(QStringList& strList, const QString& id, const QString& leadIn) const
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QStringList accList = account.accountList();
  QStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + ":");
  }
}

QString KExportDlg::accountId(const QString& account) const
{
  return MyMoneyFile::instance()->nameToAccount(account);
}
*/

#include "kexportdlg.moc"
