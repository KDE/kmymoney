/***************************************************************************
                          kgncimportoptions.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>
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

#ifndef KGNCIMPORTOPTIONSDLG_H
#define KGNCIMPORTOPTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTextCodec;

class KGncImportOptionsDlgPrivate;
class KGncImportOptionsDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KGncImportOptionsDlg)

public:
  explicit KGncImportOptionsDlg(QWidget *parent = nullptr);
  ~KGncImportOptionsDlg();

  int investmentOption() const;
  bool quoteOption() const;
  bool scheduleOption() const;
  QTextCodec* decodeOption();
  bool txNotesOption() const;
  bool generalDebugOption() const;
  bool xmlDebugOption() const;
  bool anonymizeOption() const;

public slots:
  void slotDecodeOptionChanged(bool);
  void slotHelp();

private:
  KGncImportOptionsDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KGncImportOptionsDlg)
};

#endif
