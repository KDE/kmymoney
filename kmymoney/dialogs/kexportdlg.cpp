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

#include "kexportdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QLabel>
#include <QPixmap>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneycategory.h"
#include "mymoneyqifprofileeditor.h"
#include "mymoneyfile.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneyutils.h"
#include "models.h"

KExportDlg::KExportDlg(QWidget *parent)
    : KExportDlgDecl(parent)
{
  // Set (almost) all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();

  // load button icons
  m_qbuttonCancel->setGuiItem(KStandardGuiItem::cancel());

  KGuiItem okButtenItem(i18n("&Export"),
                        KIcon("document-export"),
                        i18n("Start operation"),
                        i18n("Use this to start the export operation"));
  m_qbuttonOk->setGuiItem(okButtenItem);

  KGuiItem browseButtenItem(i18n("&Browse..."),
                            KIcon("document-open"),
                            i18n("Select filename"),
                            i18n("Use this to select a filename to export to"));
  m_qbuttonBrowse->setGuiItem(browseButtenItem);

  KGuiItem newButtenItem(i18nc("New profile", "&New..."),
                         KIcon("document-new"),
                         i18n("Create a new profile"),
                         i18n("Use this to open the profile editor"));
  m_profileEditorButton->setGuiItem(newButtenItem);


  // connect the buttons to their functionality
  connect(m_qbuttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(QString)), this, SLOT(checkData()));
  connect(m_qcheckboxAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_qcheckboxCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_accountComboBox, SIGNAL(accountSelected(QString)), this, SLOT(checkData(QString)));
  connect(m_profileComboBox, SIGNAL(activated(int)), this, SLOT(checkData()));
  connect(m_kmymoneydateStart, SIGNAL(dateChanged(QDate)), this, SLOT(checkData()));
  connect(m_kmymoneydateEnd, SIGNAL(dateChanged(QDate)), this, SLOT(checkData()));

  checkData(QString());
}

KExportDlg::~KExportDlg()
{
}

void KExportDlg::slotBrowse()
{
  QString newName(KFileDialog::getSaveFileName(KUrl(), "*.QIF", this));
  KMyMoneyUtils::appendCorrectFileExt(newName, QString("qif"));
  if (!newName.isEmpty())
    m_qlineeditFile->setText(newName);
}

void KExportDlg::slotNewProfile()
{
  QPointer<MyMoneyQifProfileEditor> editor = new MyMoneyQifProfileEditor(true, this);
  editor->setObjectName("QIF Profile Editor");
  if (editor->exec()) {
    m_profileComboBox->setCurrentIndex(m_profileComboBox->findText(editor->selectedProfile(), Qt::MatchExactly));
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

  list = grp.readEntry("profiles", QStringList());
  list.sort();
  m_profileComboBox->insertItems(0, list);

  if (selectLast == true) {
    grp = config->group("Last Use Settings");
    current = grp.readEntry("KExportDlg_LastProfile");
  }

  m_profileComboBox->setCurrentItem(0);
  if (list.contains(current))
    m_profileComboBox->setCurrentIndex(m_profileComboBox->findText(current, Qt::MatchExactly));
}

void KExportDlg::slotOkClicked()
{
  // Make sure we save the last used settings for use next time,
  writeConfig();
  accept();
}

void KExportDlg::readConfig()
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup kgrp = kconfig->group("Last Use Settings");
  m_qlineeditFile->setText(kgrp.readEntry("KExportDlg_LastFile"));
  m_qcheckboxAccount->setChecked(kgrp.readEntry("KExportDlg_AccountOpt", true));
  m_qcheckboxCategories->setChecked(kgrp.readEntry("KExportDlg_CatOpt", true));
  m_kmymoneydateStart->setDate(kgrp.readEntry("KExportDlg_StartDate", QDate()));
  m_kmymoneydateEnd->setDate(kgrp.readEntry("KExportDlg_EndDate", QDate()));
  // m_profileComboBox is loaded in loadProfiles(), so we don't worry here
  // m_accountComboBox is loaded in loadAccounts(), so we don't worry here
}

void KExportDlg::writeConfig()
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

  if (!m_qlineeditFile->text().isEmpty()) {
    QString strFile(m_qlineeditFile->text());
    if (KMyMoneyUtils::appendCorrectFileExt(strFile, QString("qif")))
      m_qlineeditFile->setText(strFile);
  }

  MyMoneyAccount account;
  if (!accountId.isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    account = file->account(accountId);
    if (m_lastAccount != accountId) {
      MyMoneyTransactionFilter filter(accountId);
      QList<MyMoneyTransaction> list = file->transactionList(filter);
      QList<MyMoneyTransaction>::Iterator it;

      if (!list.isEmpty()) {
        it = list.begin();
        m_kmymoneydateStart->loadDate((*it).postDate());
        it = list.end();
        --it;
        m_kmymoneydateEnd->loadDate((*it).postDate());
      }
      m_lastAccount = accountId;
      m_accountComboBox->setSelected(account.id());
    }
  }

  if (!m_qlineeditFile->text().isEmpty()
      && !m_accountComboBox->getSelected().isEmpty()
      && !m_profileComboBox->currentText().isEmpty()
      && m_kmymoneydateStart->date() <= m_kmymoneydateEnd->date()
      && (m_qcheckboxAccount->isChecked() || m_qcheckboxCategories->isChecked()))
    okEnabled = true;

  m_qbuttonOk->setEnabled(okEnabled);
}

void KExportDlg::loadAccounts()
{
  AccountNamesFilterProxyModel *filterProxyModel = new AccountNamesFilterProxyModel(this);
  filterProxyModel->addAccountGroup(MyMoneyAccount::Asset);
  filterProxyModel->addAccountGroup(MyMoneyAccount::Liability);
  filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  filterProxyModel->sort(0);
  m_accountComboBox->setModel(filterProxyModel);
}

QString KExportDlg::accountId() const
{
  return m_lastAccount;
}
