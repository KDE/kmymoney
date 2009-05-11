/***************************************************************************
                          kgncimportoptions.h
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

#ifndef KGNCIMPORTOPTIONSDLG_H
#define KGNCIMPORTOPTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qtextcodec.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3PtrCollection>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../dialogs/kgncimportoptionsdlgdecl.h"

typedef QPair<int, QTextCodec*> codecData;

// class to sort codec list
class codecDataList : public Q3PtrList<codecData> {
    int compareItems (Q3PtrCollection::Item a, Q3PtrCollection::Item b);
};

class KGncImportOptionsDlg : public KGncImportOptionsDlgDecl
{
Q_OBJECT
public:
  KGncImportOptionsDlg(QWidget *parent = 0, const char *name = 0);
  ~KGncImportOptionsDlg();

  int investmentOption () const {return (buttonInvestGroup->selectedId());};
  bool quoteOption() const {return (checkFinanceQuote->isChecked());};
  bool scheduleOption () const {return (checkSchedules->isChecked());};
  QTextCodec* decodeOption ();
  bool txNotesOption () const {return (checkTxNotes->isChecked());};
  bool generalDebugOption () const {return (checkDebugGeneral->isChecked());};
  bool xmlDebugOption () const {return (checkDebugXML->isChecked());};
  bool anonymizeOption () const {return (checkAnonymize->isChecked());};

public slots:
  void slotDecodeOptionChanged (bool);
  void slotHelp();

private:
  void buildCodecList ();

  QTextCodec* m_localeCodec;
  codecDataList m_codecList;

};

#endif
