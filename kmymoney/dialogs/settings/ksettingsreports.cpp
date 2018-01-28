/***************************************************************************
                             ksettingsreports.cpp
                             --------------------
    copyright            : (C) 2010 by Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
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

class KSettingsReportsPrivate
{
  Q_DISABLE_COPY(KSettingsReportsPrivate)

public:
  KSettingsReportsPrivate() :
    ui(new Ui::KSettingsReports),
    m_fileKLineEdit(nullptr)
  {
  }

  ~KSettingsReportsPrivate()
  {
    delete ui;
  }


  /**
   * Collector for both signals
   * urlSelected and editingFinished.
   *
   * Only shows a warning
   * if the selected file
   * is not a readable plain file -
   * and only one time.
   *
   * @param[in] css  css file name
   *
   * @see KSettingsReports#slotCssUrlSelected
   * @see KSettingsReports#slotEditingFinished
   */
  void checkCssFile(QString& css) {

    if (css == m_cssFileOld) {
      // do not check again to avoid emitting a warning more than 1 time
      return;
    }

    m_cssFileOld = css;

    QFileInfo* info = new QFileInfo(css);

    if (!info->exists()) {
      KMessageBox::sorry(0, i18n("File %1 does not exist", css));
      return;
    }

    QList<QString> warnings;

    if (!info->isFile()) {
      warnings.append(i18n("it is not a plain file"));
    }

    if (!info->isReadable()) {
      warnings.append(i18n("it is not readable"));
    }

    if (info->size() < 1) {
      warnings.append(i18n("it is empty"));
    }

    if (warnings.size() < 1) {
      // no warnings, fine
      return;
    }

    QString out = i18np("There is a problem with file %1", "There are problems with file %1", css);

    QList<QString>::const_iterator i;
    for (i = warnings.constBegin(); i != warnings.constEnd(); ++i) {
      out += '\n' + *i;
    }

    KMessageBox::sorry(0, out);
  }

  Ui::KSettingsReports *ui;
  /**
   * Old value of css file to avoid warnings
   * when a signal is emitted
   * but the value itself did not change.
   */
  QString m_cssFileOld;

  /**
   * Pointer to the KLineEdit of the KFileDialog which we need
   * to receive signal editingFinished.
   */
  KLineEdit* m_fileKLineEdit;
};

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

/**
 * Receiver for signal urlSelected.
 *
 * Signal urlSelected only is emitted
 * when a file is selected with the file chooser.
 *
 * @param[in] cssUrl  url of css file
 *
 * @see KSettingsReports#Private#checkCssFile
 */
void KSettingsReports::slotCssUrlSelected(const QUrl &cssUrl)
{
  Q_D(KSettingsReports);
  auto css = cssUrl.toLocalFile();
  d->checkCssFile(css);
}

/**
 * Receiver for signal editingFinished.
 *
 * Signal editingFinished is emitted
 * on focus out only,
 * not  when a file is selected with the file chooser.
 *
 * @see KSettingsReports#Private#checkCssFile
 */
void KSettingsReports::slotEditingFinished()
{
  Q_D(KSettingsReports);
  auto txt = d->m_fileKLineEdit->text();
  d->checkCssFile(txt);
}
