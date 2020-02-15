/***************************************************************************
                          kgncimportoptions.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgncimportoptionsdlg.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QCheckBox>
#include <QLayout>
#include <QApplication>
#include <QComboBox>
#include <QList>
#include <QByteArray>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <ktoolinvocation.h>

// ----------------------------------------------------------------------------
// Project Includes

// dialog constructor
KGncImportOptionsDlg::KGncImportOptionsDlg(QWidget *)
{
  setButtons(Ok | Help);
  m_widget = new KGncImportOptionsDlgDecl();
  setMainWidget(m_widget);

  m_widget->buttonInvestGroup->setId(m_widget->radioInvest1, 0); // one invest acct per stock
  m_widget->buttonInvestGroup->setId(m_widget->radioInvest2, 1); // one invest acct for all stocks
  m_widget->buttonInvestGroup->setId(m_widget->radioInvest3, 2); // prompt for each stock

  m_widget->buttonGroup5->setExclusive(false);
#ifdef HAVE_ALK_FINANCEQUOTE
  m_widget->checkFinanceQuote->setChecked(true);
#else
  m_widget->checkFinanceQuote->setVisible(false);
#endif
  m_widget->buttonGroup2->setExclusive(false);
  m_widget->checkSchedules->setChecked(false);

  buildCodecList();  // build list of codecs and insert into combo box

  m_widget->buttonGroup4->setExclusive(false);
  m_widget->checkDecode->setChecked(false);
  m_widget->comboDecode->setEnabled(false);

  m_widget->buttonGroup18->setExclusive(false);
  m_widget->checkTxNotes->setChecked(false);

  m_widget->buttonGroup3->setExclusive(false);
  m_widget->checkDebugGeneral->setChecked(false);
  m_widget->checkDebugXML->setChecked(false);
  m_widget->checkAnonymize->setChecked(false);

  connect(m_widget->checkDecode, SIGNAL(toggled(bool)), this, SLOT(slotDecodeOptionChanged(bool)));
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotHelp()));
}

KGncImportOptionsDlg::~KGncImportOptionsDlg() {}

// enable the combo box for selection if required
void KGncImportOptionsDlg::slotDecodeOptionChanged(bool isOn)
{
  if (isOn) {
    m_widget->comboDecode->setEnabled(true);
    m_widget->comboDecode->setCurrentItem(0);
  } else {
    m_widget->comboDecode->setEnabled(false);
  }
}
void KGncImportOptionsDlg::buildCodecList()
{
  m_localeCodec = QTextCodec::codecForLocale();
  QList<QByteArray> codecList = QTextCodec::availableCodecs();
  QList<QByteArray>::ConstIterator itc;
  for (itc = codecList.constBegin(); itc != codecList.constEnd(); ++itc) {
    if (*itc == m_localeCodec)
      m_widget->comboDecode->insertItem(0, QString(*itc));
    else
      m_widget->comboDecode->insertItem(9999, QString(*itc));
  }
}

// return selected codec or 0
QTextCodec* KGncImportOptionsDlg::decodeOption()
{
  if (!m_widget->checkDecode->isChecked()) {
    return (0);
  } else {
    return (QTextCodec::codecForName(m_widget->comboDecode->currentText().toUtf8()));
  }
}

void KGncImportOptionsDlg::slotHelp()
{
  KToolInvocation::invokeHelp("details.impexp.gncoptions");
}
