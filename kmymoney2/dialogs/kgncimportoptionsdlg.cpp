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
#include <QLineEdit>
#include <QLayout>
#include <QApplication>
#include <QComboBox>

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
KGncImportOptionsDlg::KGncImportOptionsDlg(QWidget *parent)
 : KGncImportOptionsDlgDecl(parent)
{
  buttonInvestGroup->setId(radioInvest1, 0); // one invest acct per stock
  buttonInvestGroup->setId(radioInvest2, 1); // one invest acct for all stocks
  buttonInvestGroup->setId(radioInvest3, 2); // prompt for each stock

  buttonGroup5->setExclusive(false);
  checkFinanceQuote->setChecked(true);

  buttonGroup2->setExclusive(false);
  checkSchedules->setChecked (false);

  buildCodecList (); // build list of codecs and insert into combo box

  buttonGroup4->setExclusive(false);
  checkDecode->setChecked (false);
  comboDecode->setEnabled (false);

  buttonGroup18->setExclusive(false);
  checkTxNotes->setChecked (false);

  buttonGroup3->setExclusive(false);
  checkDebugGeneral->setChecked (false);
  checkDebugXML->setChecked (false);
  checkAnonymize->setChecked (false);

  connect (checkDecode, SIGNAL(toggled(bool)), this, SLOT(slotDecodeOptionChanged(bool)));
}

KGncImportOptionsDlg::~KGncImportOptionsDlg()
{
}

// enable the combo box for selection if required
void KGncImportOptionsDlg::slotDecodeOptionChanged(bool isOn) {
  if (isOn) {
    comboDecode->setEnabled (true);
    comboDecode->setCurrentItem (0);
  } else {
    comboDecode->setEnabled (false);
  }
}

// build a list of known codecs and sort it so that the locale codec is first
// try to get the others in some sort of order of likelihood
void KGncImportOptionsDlg::buildCodecList () {

  m_localeCodec = QTextCodec::codecForLocale();
  m_codecList.setAutoDelete (true);
  // retrieve all codec pointers
  QTextCodec *codec;
  unsigned int i;
  int len = qstrlen(m_localeCodec->name());
  for (i = 0; (codec = QTextCodec::codecForName(QTextCodec::availableCodecs().value(i))); i++) {
    int rank;
    if (codec == m_localeCodec) rank = 999; // ensure locale rank comes first
    else rank = qstrlen(codec->name()) - abs(len - qstrlen(codec->name()));

    // We are really just guessing at the order here, but...
    // This used to be else rank = codec->heuristicNameMatch(m_localeCodec->name())
    // in Qt3 heuristicNameMatch returned...
    // if (qstricmp(codec->name(), m_localeCode->name()) == 0)
    //   qstrlen(m_localeCodec->name());
    // else if letters and numbers match (only letters and numbers, adding a space at character class transition)
    //   qstrlen(m_localeCodec->name())-1;
    // else if letters and numbers strings match after stripping whitespace
    //   qstrlen(m_localeCodec->name())-2;
    // else 0

    codecData *p = new codecData(rank, codec);
    m_codecList.append (p);
  }
  m_codecList.sort();
  for (i = 0; i < m_codecList.count(); ++i) {
    QString name (m_codecList.at(i)->second->name());
    comboDecode->addItem (name);
  }
}

// this routine sorts the codec list on 1) rank descending 2) codec name ascending
int codecDataList::compareItems (void *a, void *b) {
  codecData *pa = reinterpret_cast<codecData *>(a);
  codecData *pb = reinterpret_cast<codecData *>(b);

  if (pa->first > pb->first) {
    return (-1); // greater rank is treated as less-than so gets sorted first
  } else { if (pb->first > pa->first)
        return (1);
  }
  // ranks are equal, sort on name, case insensitive
  QString sa(pa->second->name());
  QString sb(pb->second->name());
  if (sa.toLower() > sb.toLower()) {
    return (1);
  } else {
    return (-1);
  }
}

// return selected codec or 0
QTextCodec* KGncImportOptionsDlg::decodeOption(void) {
  if (!checkDecode->isChecked()) {
    return (0);
  } else {
    return (m_codecList.at(comboDecode->currentIndex())->second);
  }
}

void KGncImportOptionsDlg::slotHelp(void)
{
  KToolInvocation::invokeHelp ("details.impexp.gncoptions");
}

#include "kgncimportoptionsdlg.moc"
