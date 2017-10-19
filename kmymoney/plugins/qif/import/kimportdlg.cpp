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
#include <QPushButton>
#include <QIcon>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kcombobox.h>
#include <kmessagebox.h>
#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include <KSharedConfig>
#include "../config/mymoneyqifprofile.h"
#include <icons/icons.h>

using namespace Icons;

KImportDlg::KImportDlg(QWidget *parent)
    : KImportDlgDecl(parent)
{
  // Set all the last used options
  readConfig();

  loadProfiles(true);

  // load button icons
  KGuiItem okButtenItem(i18n("&Import"),
                        QIcon::fromTheme(g_Icons[Icon::DocumentImport]),
                        i18n("Start operation"),
                        i18n("Use this to start the import operation"));
  KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Ok), okButtenItem);

  KGuiItem browseButtenItem(i18n("&Browse..."),
                            QIcon::fromTheme(g_Icons[Icon::DocumentOpen]),
                            i18n("Select filename"),
                            i18n("Use this to select a filename to export to"));
  KGuiItem::assign(m_qbuttonBrowse, browseButtenItem);

  KGuiItem newButtenItem(i18nc("New profile", "&New..."),
                         QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                         i18n("Create a new profile"),
                         i18n("Use this to open the profile editor"));

  // connect the buttons to their functionality
  connect(m_qbuttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(slotOkClicked()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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

  QUrl file = QFileDialog::getOpenFileUrl(this, i18n("Import File..."), QUrl("kfiledialog:///kmymoney-import"),
      i18n("Import files (%1);;All files (%2)", tmpprofile.filterFileType(), "*")
  );

  if (!file.isEmpty()) {
    m_qlineeditFile->setText(file.toDisplayString(QUrl::PreferLocalFile));
  }
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
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  KConfigGroup kgrp = kconfig->group("Last Use Settings");
  m_qlineeditFile->setText(kgrp.readEntry("KImportDlg_LastFile"));

}

void KImportDlg::writeConfig()
{
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  KConfigGroup grp = kconfig->group("Last Use Settings");
  grp.writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  grp.writeEntry("KImportDlg_LastProfile", m_profileComboBox->currentText());
  kconfig->sync();
}

/** Make sure the text input is ok */
void KImportDlg::slotFileTextChanged(const QString& text)
{
  // TODO: port to kf5
  Q_UNUSED(text)
#if 0
  if (!text.isEmpty() && KIO::NetAccess::exists(file(), KIO::NetAccess::SourceSide, KMyMoneyUtils::mainWindow())) {
    // m_qcomboboxDateFormat->setEnabled(true);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_qlineeditFile->setText(text);
  } else {
    // m_qcomboboxDateFormat->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
#endif
}

void KImportDlg::loadProfiles(const bool selectLast)
{
  QString current = m_profileComboBox->currentText();

  m_profileComboBox->clear();

  QStringList list;
  KSharedConfigPtr config = KSharedConfig::openConfig();
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
    addCategories(strList, *it_a, leadIn + account.name() + MyMoneyFile::AccountSeperator);
  }
}
