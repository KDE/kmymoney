/***************************************************************************
                             kfilepage.cpp
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kfilepage.h"
#include "kfilepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <QLabel>
#include <QFileInfo>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KUser>
#include <KFile>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kfilepage.h"
#include "knewuserwizard.h"
#include "kguiutils.h"

namespace NewUserWizard
{
  FilePage::FilePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new FilePagePrivate(wizard), stepCount++, this, wizard)
  {
    Q_D(FilePage);
    d->ui->setupUi(this);
    d->m_mandatoryGroup->add(d->ui->m_dataFileEdit->lineEdit());
    connect(d->m_mandatoryGroup, static_cast<void (KMandatoryFieldGroup::*)()>(&KMandatoryFieldGroup::stateChanged), object(), &KMyMoneyWizardPagePrivate::completeStateChanged);

    KUser user;
    QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (folder.isEmpty() || !QDir().exists(folder))
      folder = QDir::homePath();
    d->ui->m_dataFileEdit->setStartDir(QUrl::fromLocalFile(folder));
    d->ui->m_dataFileEdit->setUrl(QUrl::fromLocalFile(folder + QLatin1Char('/') + user.loginName() + QLatin1String(".kmy")));
    d->ui->m_dataFileEdit->setFilter(i18n("*.kmy *.xml|KMyMoney files\n*|All files"));
    d->ui->m_dataFileEdit->setMode(KFile::File);
  }

  FilePage::~FilePage()
  {
  }

  bool FilePage::isComplete() const
  {
    Q_D(const FilePage);
    //! @todo Allow to overwrite files
    bool rc = d->m_mandatoryGroup->isEnabled();
    d->ui->m_existingFileLabel->hide();
    d->ui->m_finishLabel->show();
    if (rc) {
      // if a filename is present, check that
      // a) the file does not exist
      // b) the directory does exist
      // c) the file is stored locally (because we cannot check previous conditions if it is not)
      const QUrl fullPath = d->ui->m_dataFileEdit->url();
      QFileInfo directory{fullPath.adjusted(QUrl::RemoveFilename).toLocalFile()};
      qDebug() << "Selected fileptah: " << fullPath << " " << directory.absoluteFilePath() << " dir: " << directory.isDir();
      rc = false;
      if (!fullPath.isValid() || !fullPath.isLocalFile()) {
        d->ui->m_dataFileEdit->setToolTip(i18n("The path has to be valid and cannot be on a remote location."));
      } else if (QFileInfo::exists(fullPath.toLocalFile())) {
        d->ui->m_dataFileEdit->setToolTip(i18n("The file exists already. Please create a new file."));
      } else if (!directory.isDir()) {
        d->ui->m_dataFileEdit->setToolTip(i18n("The destination directory does not exist or cannot be written to."));
      } else {
        d->ui->m_dataFileEdit->setToolTip("");
        rc = true;
      }

      d->ui->m_existingFileLabel->setHidden(rc);
      d->ui->m_finishLabel->setVisible(rc);
    }
    return rc;
  }

}
