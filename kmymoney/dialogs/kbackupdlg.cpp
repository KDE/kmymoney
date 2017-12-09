/***************************************************************************
                          kbackupdialog.cpp  -  description
                             -------------------
    begin                : Mon Jun 4 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
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

#include "kbackupdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QUrl>
#include <QPushButton>
#include <QIcon>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbackupdlg.h"

#include "icons/icons.h"

using namespace Icons;

KBackupDlg::KBackupDlg(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::KBackupDlg)
{
  ui->setupUi(this);
  readConfig();

  ui->chooseButton->setIcon(Icons::get(Icon::Folder));

  connect(ui->chooseButton, &QAbstractButton::clicked, this, &KBackupDlg::chooseButtonClicked);
}

KBackupDlg::~KBackupDlg()
{
  writeConfig();
  delete ui;
}

QString KBackupDlg::mountPoint() const
{
  return ui->txtMountPoint->text();
}

bool KBackupDlg::mountCheckBox() const
{
  return ui->mountCheckBox;
}

void KBackupDlg::chooseButtonClicked()
{
  auto newDir = QFileDialog::getExistingDirectoryUrl(this, QString(), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
  if (!newDir.path().isEmpty())
    ui->txtMountPoint->setText(newDir.path());
}

void KBackupDlg::readConfig()
{
  QString backupDefaultLocation;
#ifdef Q_OS_WIN
  backupDefaultLocation = QDir::toNativeSeparators(QDir::homePath() + "/kmymoney/backup");
#else
  backupDefaultLocation = "/mnt/floppy";
#endif
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  ui->mountCheckBox->setChecked(grp.readEntry("KBackupDlg_mountDevice", false));
  ui->txtMountPoint->setText(grp.readEntry("KBackupDlg_BackupMountPoint", backupDefaultLocation));
}

void KBackupDlg::writeConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KBackupDlg_mountDevice", ui->mountCheckBox->isChecked());
  grp.writeEntry("KBackupDlg_BackupMountPoint", ui->txtMountPoint->text());
  config->sync();
}
