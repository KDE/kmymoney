/***************************************************************************
                          kgncimportoptions.h
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

#ifndef KGNCIMPORTOPTIONSDLG_H
#define KGNCIMPORTOPTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "ui_kgncimportoptionsdlgdecl.h"

class QTextCodec;
class KGncImportOptionsDlgDecl : public QWidget, public Ui::KGncImportOptionsDlgDecl
{
public:
  KGncImportOptionsDlgDecl() {
    setupUi(this);
  }
};

class KGncImportOptionsDlg : public QDialog
{
  Q_OBJECT
public:
  KGncImportOptionsDlg(QWidget *parent = 0);
  ~KGncImportOptionsDlg();

  int investmentOption() const {
    return (m_widget->buttonInvestGroup->checkedId());
  };
  bool quoteOption() const {
    return (m_widget->checkFinanceQuote->isChecked());
  };
  bool scheduleOption() const {
    return (m_widget->checkSchedules->isChecked());
  };
  QTextCodec* decodeOption();
  bool txNotesOption() const {
    return (m_widget->checkTxNotes->isChecked());
  };
  bool generalDebugOption() const {
    return (m_widget->checkDebugGeneral->isChecked());
  };
  bool xmlDebugOption() const {
    return (m_widget->checkDebugXML->isChecked());
  };
  bool anonymizeOption() const {
    return (m_widget->checkAnonymize->isChecked());
  };

public slots:
  void slotDecodeOptionChanged(bool);
  void slotHelp();

private:
  void buildCodecList();

  QTextCodec* m_localeCodec;
  KGncImportOptionsDlgDecl* m_widget;
};

#endif
