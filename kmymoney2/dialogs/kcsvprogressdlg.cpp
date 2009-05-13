/***************************************************************************
                          kcsvprogressdlg.cpp  -  description
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstandarddirs.h>
#endif

#include <qpixmap.h>
// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <q3textstream.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3progressbar.h>
#include <qlineedit.h>
#include <q3groupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kcsvprogressdlg.h"
#include "../widgets/kmymoneydateinput.h"
#include "../kmymoneyutils.h"

/** Simple constructor */
KCsvProgressDlg::KCsvProgressDlg(int type, MyMoneyAccount *account, QWidget *parent, const char *name )
 : KCsvProgressDlgDecl(parent)
{
  m_nType = type;
  if (m_nType==0)
  {
    m_kmymoneydateEnd->setEnabled(false);
    m_kmymoneydateStart->setEnabled(false);
    m_qbuttonRun->setText(i18n("&Import"));
  }
  else
    m_qbuttonRun->setText(i18n("&Export"));

  m_mymoneyaccount = account;

  m_qbuttonOk->setText(i18n("C&lose"));

  readConfig();

  connect(m_qbuttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowseClicked()));
  connect(m_qbuttonRun, SIGNAL(clicked()), this, SLOT(slotRunClicked()));
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this,
    SLOT(slotFileTextChanged(const QString&)));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(accept()));
}

/** Simple destructor */
KCsvProgressDlg::~KCsvProgressDlg()
{
  writeConfig();
}

/** Perform the export process */
void KCsvProgressDlg::performExport(void)
{
/*
  // Do some validation on the inputs.
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter the path to the CSV file"), i18n("Export CSV"));
    m_qlineeditFile->setFocus();
    return;
  }

  QString strFile = m_qlineeditFile->text();
  if(KMyMoneyUtils::appendCorrectFileExt(strFile, QString("csv")))
    m_qlineeditFile->setText(strFile);

  if (m_kmymoneydateEnd->getQDate() < m_kmymoneydateStart->getQDate()) {
    KMessageBox::information(this, i18n("Please enter a start date lower than the end date."));
    return;
  }

  QFile qfile(m_qlineeditFile->text());
  if (!qfile.open(QIODevice::WriteOnly)) {
    KMessageBox::error(this, i18n("Unable to open export file for writing."));
    return;
  }
  qfile.close();

  m_qlabelAccount->setText(m_mymoneyaccount->name());
  m_qlabelTransaction->setText(QString("0") + i18n(" of ") + QString::number(m_mymoneyaccount->transactionCount()));
  m_qprogressbar->setTotalSteps(m_mymoneyaccount->transactionCount());

  // Make sure we have an account to operate on
  if (m_mymoneyaccount) {
    // Connect to the provided signals in MyMoneyAccount
    // These signals will be emitted at appropriate times.
    connect(m_mymoneyaccount, SIGNAL(signalProgressCount(int)), m_qprogressbar, SLOT(setTotalSteps(int)));
    connect(m_mymoneyaccount, SIGNAL(signalProgress(int)), this, SLOT(slotSetProgress(int)));

    int nTransCount = 0;

    // Do the actual write
    if (!m_mymoneyaccount->writeCSVFile(m_qlineeditFile->text(), m_kmymoneydateStart->getQDate(),
          m_kmymoneydateEnd->getQDate(), nTransCount)) {
      KMessageBox::error(this, i18n("Error occurred whilst exporting to csv file."), i18n("Export CSV"));
    }
    else {
      QString qstringPrompt = i18n("Export finished successfully.\n\n");
      qstringPrompt += i18n("Number of transactions exported ");
      qstringPrompt += QString::number(nTransCount);
      qstringPrompt += ".";
      KMessageBox::information(this, qstringPrompt, i18n("Export CSV"));
    }
  }
*/
}

/** perform the import process */
void KCsvProgressDlg::performImport(void)
{
/*
  // Do some validation on the inputs.
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter the path to the CSV file"), i18n("Import CSV"));
    m_qlineeditFile->setFocus();
    return;
  }

  QFile qfile(m_qlineeditFile->text());
  if (!qfile.open(QIODevice::ReadOnly)) {
    KMessageBox::error(this, i18n("Unable to open import file for reading."));
    return;
  }
  qfile.close();

  m_qlabelAccount->setText(m_mymoneyaccount->name());

  // Make sure we have an account to operate on
  if (m_mymoneyaccount) {
    // Connect to the provided signals in MyMoneyAccount
    // These signals will be emitted at appropriate times.
    connect(m_mymoneyaccount, SIGNAL(signalProgressCount(int)), m_qprogressbar, SLOT(setTotalSteps(int)));
    connect(m_mymoneyaccount, SIGNAL(signalProgress(int)), this, SLOT(slotSetProgress(int)));

    int nTransCount = 0;

    // Do the actual write
    if (!m_mymoneyaccount->readCSVFile(m_qlineeditFile->text(), nTransCount)) {
      KMessageBox::error(this, i18n("Error occurred whilst importing csv file."), i18n("Import CSV"));
    }
    else {
      QString qstringPrompt = i18n("Import finished successfully.\n\n");
      qstringPrompt += i18n("Number of transactions imported ");
      qstringPrompt += QString::number(nTransCount);
      qstringPrompt += ".";
      KMessageBox::information(this, qstringPrompt, i18n("Import CSV"));
    }
  }
*/
}

/** Called when the user clicks on the Browser button */
void KCsvProgressDlg::slotBrowseClicked()
{
  QString newName = KFileDialog::getSaveFileName(QString::null,"*.CSV");
  if (!newName.isEmpty())
  {
    m_qlineeditFile->setText(newName);
    m_qbuttonRun->setEnabled(true);
  }
  else
    m_qbuttonRun->setEnabled(false);
}

/** Called when user clicks on the Run button */
void KCsvProgressDlg::slotRunClicked()
{
  m_qgroupbox->setEnabled(true);
  if (m_nType==0)
    performImport();
  else
    performExport();
}

/** Make sure the text input is ok */
void KCsvProgressDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty()) {
    m_qlineeditFile->setText(text);
    m_qbuttonRun->setEnabled(true);
  } else
    m_qbuttonRun->setEnabled(false);
}

void KCsvProgressDlg::readConfig(void)
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group("Last Use Settings");
  m_kmymoneydateStart->setDate(grp.readDateTimeEntry("KCsvProgressDlg_StartDate").date());
  m_kmymoneydateEnd->setDate(grp.readDateTimeEntry("KCsvProgressDlg_EndDate").date());
  m_qlineeditFile->setText(grp.readEntry("KCsvProgressDlg_LastFile", ""));
  if (m_qlineeditFile->text().length()>=1)
    m_qbuttonRun->setEnabled(true);
  else
    m_qbuttonRun->setEnabled(false);
}

void KCsvProgressDlg::writeConfig(void)
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group("Last Use Settings");
  grp.writeEntry("KCsvProgressDlg_LastFile", m_qlineeditFile->text());
  grp.writeEntry("KCsvProgressDlg_StartDate", QDateTime(m_kmymoneydateStart->date()));
  grp.writeEntry("KCsvProgressDlg_EndDate", QDateTime(m_kmymoneydateEnd->date()));
  grp.sync();
}

/** Update the progress bar, and update the transaction count indicator. */
void KCsvProgressDlg::slotSetProgress(int progress)
{
  m_qprogressbar->setProgress(progress);
  QString qstring = QString::number(progress);
  qstring += i18n(" of ");
  qstring += QString::number(m_qprogressbar->totalSteps());
  m_qlabelTransaction->setText(qstring);
}

#include "kcsvprogressdlg.moc"
