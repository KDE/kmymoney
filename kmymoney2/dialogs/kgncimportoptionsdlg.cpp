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
{
  setButtons(Ok | Cancel | Help);
  m_widget = new KGncImportOptionsDlgDecl();
  setMainWidget(m_widget);

  m_widget->buttonInvestGroup->setId(m_widget->radioInvest1, 0); // one invest acct per stock
  m_widget->buttonInvestGroup->setId(m_widget->radioInvest2, 1); // one invest acct for all stocks
  m_widget->buttonInvestGroup->setId(m_widget->radioInvest3, 2); // prompt for each stock

  m_widget->buttonGroup5->setExclusive(false);
  m_widget->checkFinanceQuote->setChecked(true);

  m_widget->buttonGroup2->setExclusive(false);
  m_widget->checkSchedules->setChecked (false);

  buildCodecList (); // build list of codecs and insert into combo box

  m_widget->buttonGroup4->setExclusive(false);
  m_widget->checkDecode->setChecked (false);
  m_widget->comboDecode->setEnabled (false);

  m_widget->buttonGroup18->setExclusive(false);
  m_widget->checkTxNotes->setChecked (false);

  m_widget->buttonGroup3->setExclusive(false);
  m_widget->checkDebugGeneral->setChecked (false);
  m_widget->checkDebugXML->setChecked (false);
  m_widget->checkAnonymize->setChecked (false);

  connect (m_widget->checkDecode, SIGNAL(toggled(bool)), this, SLOT(slotDecodeOptionChanged(bool)));
  connect (this, SIGNAL(helpClicked()), this, SLOT(slotHelp()));
}

KGncImportOptionsDlg::~KGncImportOptionsDlg()
{
}

// enable the combo box for selection if required
void KGncImportOptionsDlg::slotDecodeOptionChanged(bool isOn) {
  if (isOn) {
    m_widget->comboDecode->setEnabled (true);
    m_widget->comboDecode->setCurrentItem (0);
  } else {
    m_widget->comboDecode->setEnabled (false);
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
    m_widget->comboDecode->addItem (name);
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
  if (!m_widget->checkDecode->isChecked()) {
    return (0);
  } else {
    return (m_codecList.at(m_widget->comboDecode->currentIndex())->second);
  }
}

void KGncImportOptionsDlg::slotHelp(void)
{
  KToolInvocation::invokeHelp ("details.impexp.gncoptions");
}
