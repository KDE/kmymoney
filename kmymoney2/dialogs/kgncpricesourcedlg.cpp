/***************************************************************************
                          kgncpricesourcedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Ace Jones
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <qlabel.h>
#include <q3listbox.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qlayout.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <ktoolinvocation.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kgncpricesourcedlg.h"
#include "webpricequote.h"

KGncPriceSourceDlg::KGncPriceSourceDlg(QWidget *parent)
 : KGncPriceSourceDlgDecl(parent)
{
}
KGncPriceSourceDlg::KGncPriceSourceDlg(const QString &stockName, const QString &gncSource,QWidget * parent)
: KGncPriceSourceDlgDecl(parent)
{
  // signals and slots connections
  connect( buttonGroup5, SIGNAL( released(int) ), this, SLOT( buttonPressed(int) ) );
  connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( slotHelp() ) );
  // initialize data fields
  textStockName->setText (i18n ("Investment: %1",stockName));
  textGncSource->setText (i18n ("Quote source: %1",gncSource));
  listKnownSource->insertStringList (WebPriceQuote::quoteSources());
  lineUserSource->setText (gncSource);
  checkAlwaysUse->setChecked(true);
  buttonGroup5->setButton (0);
  buttonPressed (0);
  return;
}

KGncPriceSourceDlg::~KGncPriceSourceDlg()
{
}

enum ButtonIds {NOSOURCE = 0, KMMSOURCE, USERSOURCE};

void KGncPriceSourceDlg::buttonPressed (int buttonId) {
  m_currentButton = buttonId;
  switch (m_currentButton) {
    case NOSOURCE:
      listKnownSource->clearSelection();
      listKnownSource->setEnabled (false);
      lineUserSource->deselect();
      lineUserSource->setEnabled (false);
      break;
    case KMMSOURCE:
      lineUserSource->deselect ();
      lineUserSource->setEnabled (false);
      listKnownSource->setEnabled (true);
      listKnownSource->setFocus();
      listKnownSource->setSelected (0, true);
      break;
    case USERSOURCE:
      listKnownSource->clearSelection();
      listKnownSource->setEnabled (false);
      lineUserSource->setEnabled (true);
      lineUserSource->selectAll();
      lineUserSource->setFocus ();
      break;
  }
}

QString KGncPriceSourceDlg::selectedSource() const {
  QString s;
  switch (m_currentButton) {
    case NOSOURCE: s = ""; break;
    case KMMSOURCE: s = listKnownSource->currentText(); break;
    case USERSOURCE: s = lineUserSource->text(); break;
  }
  return (s);
}

void KGncPriceSourceDlg::slotHelp(void)
{
  KToolInvocation::invokeHelp ("details.impexp.gncquotes");
}

#include "kgncpricesourcedlg.moc"

