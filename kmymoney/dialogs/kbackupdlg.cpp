/*
 * Copyright 2001-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2002  Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#ifdef Q_OS_WIN
  // in windows we don't have a mount capability so
  // we hide that option from the user and deactivate it
  ui->mountCheckBox->setChecked(false);
  ui->mountCheckBox->hide();
  ui->txtAutoMount->hide();
  ui->lblMountPoint->setText(i18n("Backup location:"));
#endif

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

bool KBackupDlg::mountCheckBoxChecked() const
{
  return ui->mountCheckBox->isChecked();
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
