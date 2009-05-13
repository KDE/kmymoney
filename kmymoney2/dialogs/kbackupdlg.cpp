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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstandarddirs.h>
#endif

#include <kconfig.h>
#include <kdirselectdialog.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbackupdlg.h"

KBackupDlg::KBackupDlg( QWidget* parent,  const char* name/*, bool modal*/)
  : kbackupdlgdecl( parent,  name , true)
{
  readConfig();

  // add icons to buttons
  KIconLoader *il = KIconLoader::global();
  btnOK->setGuiItem(KStandardGuiItem::ok());
  btnCancel->setGuiItem(KStandardGuiItem::cancel());

  KGuiItem chooseButtenItem( i18n("C&hoose..."),
                    QIcon(il->loadIcon("folder", KIconLoader::Small, KIconLoader::SizeSmall)),
                    i18n("Select mount point"),
                    i18n("Use this to browse to the mount point."));
  chooseButton->setGuiItem(chooseButtenItem);
  
  connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseButtonClicked()));
  connect(btnOK,SIGNAL(clicked()),this,SLOT(accept()));
  connect(btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

KBackupDlg::~KBackupDlg()
{
  writeConfig();
}

void KBackupDlg::chooseButtonClicked()
{
  KUrl newDir = KDirSelectDialog::selectDirectory(KGlobalSettings::documentPath());
  if (newDir.hasPath())
    txtMountPoint->setText(newDir.path());
}

void KBackupDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->group("Last Use Settings");
  mountCheckBox->setChecked(grp.readEntry("KBackupDlg_mountDevice", false));
  txtMountPoint->setText(grp.readEntry("KBackupDlg_BackupMountPoint", "/mnt/floppy"));
}

void KBackupDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->group("Last Use Settings");
  config->writeEntry("KBackupDlg_mountDevice", mountCheckBox->isChecked());
  config->writeEntry("KBackupDlg_BackupMountPoint", txtMountPoint->text());
  config->sync();
}

#include "kbackupdlg.moc"
