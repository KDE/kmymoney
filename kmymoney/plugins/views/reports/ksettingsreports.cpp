/***************************************************************************
                             ksettingsreports.cpp
                             --------------------
    copyright            : (C) 2010 by Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ksettingsreports.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>
#include <QFileInfo>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsreports.h"

#include "kmymoneysettings.h"



KSettingsReports::KSettingsReports(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsReportsPrivate)
{
  Q_D(KSettingsReports);
  d->ui->setupUi(this);

  // keep initial (default) css file in mind
  d->m_cssFileOld = KMyMoneySettings::cssFileDefault();

  // set default css file in ksettingsreports dialog
  d->ui->kcfg_CssFileDefault->setUrl(QUrl::fromLocalFile(KMyMoneySettings::cssFileDefault()));

  d->m_fileKLineEdit = d->ui->kcfg_CssFileDefault->lineEdit();

  connect(d->ui->kcfg_CssFileDefault, &KUrlRequester::urlSelected,
          this, &KSettingsReports::slotCssUrlSelected);

  connect(d->m_fileKLineEdit, &QLineEdit::editingFinished,
          this, &KSettingsReports::slotEditingFinished);
}

KSettingsReports::~KSettingsReports()
{
  Q_D(KSettingsReports);
  delete d;
}


