/***************************************************************************
                          kbackupdialog.cpp  -  description
                             -------------------
    begin                : Mon Jun 4 2001
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

#include "kbackupdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QLabel>
#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kfiledialog.h>

// ----------------------------------------------------------------------------
// Project Includes

KBackupDlg::KBackupDlg(QWidget* parent)
    : kbackupdlgdecl(parent)
{
  readConfig();

  // add icons to buttons
  btnOK->setGuiItem(KStandardGuiItem::ok());
  btnCancel->setGuiItem(KStandardGuiItem::cancel());

  KGuiItem chooseButtenItem(i18n("C&hoose..."),
                            KIcon("folder"),
                            i18n("Select mount point"),
                            i18n("Use this to browse to the mount point."));
  chooseButton->setGuiItem(chooseButtenItem);

  connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseButtonClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

KBackupDlg::~KBackupDlg()
{
  writeConfig();
}

void KBackupDlg::chooseButtonClicked()
{
  KUrl newDir = KFileDialog::getExistingDirectoryUrl(KUrl::fromPath(KGlobalSettings::documentPath()));
  if (newDir.hasPath())
    txtMountPoint->setText(newDir.path());
}

void KBackupDlg::readConfig()
{
  QString backupDefaultLocation;
#ifdef Q_OS_WIN
  backupDefaultLocation = QDir::toNativeSeparators(QDir::homePath() + "/kmymoney/backup");
#else
  backupDefaultLocation = "/mnt/floppy";
#endif
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  mountCheckBox->setChecked(grp.readEntry("KBackupDlg_mountDevice", false));
  txtMountPoint->setText(grp.readEntry("KBackupDlg_BackupMountPoint", backupDefaultLocation));
}

void KBackupDlg::writeConfig()
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KBackupDlg_mountDevice", mountCheckBox->isChecked());
  grp.writeEntry("KBackupDlg_BackupMountPoint", txtMountPoint->text());
  config->sync();
}
