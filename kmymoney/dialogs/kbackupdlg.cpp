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
#include <QUrl>
#include <QPushButton>
#include <QIcon>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <KGuiItem>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

KBackupDlg::KBackupDlg(QWidget* parent)
    : kbackupdlgdecl(parent)
{
  readConfig();

  KGuiItem chooseButtenItem(i18n("C&hoose..."),
                            QIcon::fromTheme("folder"),
                            i18n("Select mount point"),
                            i18n("Use this to browse to the mount point."));
  KGuiItem::assign(chooseButton, chooseButtenItem);

  connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseButtonClicked()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

KBackupDlg::~KBackupDlg()
{
  writeConfig();
}

void KBackupDlg::chooseButtonClicked()
{
  QUrl newDir = QFileDialog::getExistingDirectoryUrl(this, QString(), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
  if (!newDir.path().isEmpty())
    txtMountPoint->setText(newDir.path());
}

void KBackupDlg::readConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  mountCheckBox->setChecked(grp.readEntry("KBackupDlg_mountDevice", false));
  txtMountPoint->setText(grp.readEntry("KBackupDlg_BackupMountPoint", "/mnt/floppy"));
}

void KBackupDlg::writeConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KBackupDlg_mountDevice", mountCheckBox->isChecked());
  grp.writeEntry("KBackupDlg_BackupMountPoint", txtMountPoint->text());
  config->sync();
}
