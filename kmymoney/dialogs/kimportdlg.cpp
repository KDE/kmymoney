/***************************************************************************
                          kimportdlg.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@ctv.es>
                           Felix Rodriguez <frodriguez@mail.wesleyan.edu>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kimportdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QLabel>
#include <QPixmap>
#include <QApplication>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "kmymoneyutils.h"
#include <mymoneyfile.h>
#include "mymoneyqifprofileeditor.h"
#include "mymoneyqifprofile.h"

KImportDlg::KImportDlg(QWidget *parent)
    : KImportDlgDecl(parent)
{
  setModal(true);
  // Set all the last used options
  readConfig();

  loadProfiles(true);

  // load button icons
  m_qbuttonCancel->setGuiItem(KStandardGuiItem::cancel());

  KGuiItem okButtenItem(i18n("&Import"),
                        KIcon("document-import"),
                        i18n("Start operation"),
                        i18n("Use this to start the import operation"));
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
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(QString)), this,
          SLOT(slotFileTextChanged(QString)));

  // setup button enable status
  slotFileTextChanged(m_qlineeditFile->text());
}

KImportDlg::~KImportDlg()
{
}

void KImportDlg::slotBrowse()
{
  // determine what the browse prefix should be from the current profile

  MyMoneyQifProfile tmpprofile;
  tmpprofile.loadProfile("Profile-" + profile());

  QPointer<KFileDialog> dialog = new KFileDialog(KUrl("kfiledialog:///kmymoney-import"),
      i18n("%1|Import files\n%2|All files", tmpprofile.filterFileType(), "*"),
      this);
  dialog->setCaption(i18n("Import File..."));
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if (dialog->exec() == QDialog::Accepted) {
    m_qlineeditFile->setText(dialog->selectedUrl().pathOrUrl());
  }
  delete dialog;
}

void KImportDlg::slotOkClicked()
{
  // Save the used options.
  writeConfig();
  // leave dialog directly
  accept();
}

void KImportDlg::readConfig()
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup kgrp = kconfig->group("Last Use Settings");
  m_qlineeditFile->setText(kgrp.readEntry("KImportDlg_LastFile"));

}

void KImportDlg::writeConfig()
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group("Last Use Settings");
  grp.writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  grp.writeEntry("KImportDlg_LastProfile", m_profileComboBox->currentText());
  kconfig->sync();
}

/** Make sure the text input is ok */
void KImportDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty() && KIO::NetAccess::exists(text, KIO::NetAccess::SourceSide, KMyMoneyUtils::mainWindow())) {
    // m_qcomboboxDateFormat->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
    m_qlineeditFile->setText(text);
  } else {
    // m_qcomboboxDateFormat->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
  }
}

void KImportDlg::slotNewProfile()
{
  QPointer<MyMoneyQifProfileEditor> editor = new MyMoneyQifProfileEditor(true, this);
  editor->setObjectName("QIF Profile Editor");

  if (editor->exec()) {
    loadProfiles();
    m_profileComboBox->setCurrentIndex(m_profileComboBox->findText(editor->selectedProfile(), Qt::MatchExactly));
  }

  delete editor;
}

void KImportDlg::loadProfiles(const bool selectLast)
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
  m_profileComboBox->addItems(list);

  if (selectLast == true) {
    config->group("Last Use Settings");
    current = grp.readEntry("KImportDlg_LastProfile");
  }

  int index = m_profileComboBox->findText(current, Qt::MatchExactly);
  if (index > -1) {
    m_profileComboBox->setCurrentIndex(index);
  } else {
    m_profileComboBox->setCurrentIndex(0);
  }
}

void KImportDlg::addCategories(QStringList& strList, const QString& id, const QString& leadIn) const
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QStringList accList = account.accountList();
  QStringList::ConstIterator it_a;

  for (it_a = accList.constBegin(); it_a != accList.constEnd(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + MyMoneyFile::AccountSeparator);
  }
}
