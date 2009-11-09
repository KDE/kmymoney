/***************************************************************************
                          knewequityentrydlg.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWEQUITYENTRY_H
#define KNEWEQUITYENTRY_H

#include <QDialog>
#include <klocale.h>

#include "ui_knewequityentrydecl.h"

/**
  *
  * Dialog to allow user to enter all data for a stock or mutual fund investment type.
  *
  * @author Kevin Tambascio
  *
  */

class kNewEquityEntryDecl : public QDialog, public Ui::kNewEquityEntryDecl
{
public:
  kNewEquityEntryDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KNewEquityEntryDlg : public kNewEquityEntryDecl
{
  Q_OBJECT
public:
  KNewEquityEntryDlg(QWidget *parent = NULL);
  virtual ~KNewEquityEntryDlg();

  void setSymbolName(const QString& str);
  QString symbolName(void) const  { return m_strSymbolName; }

  void setName(const QString& str);
  QString name(void) const  { return m_strName; }

  int fraction(void) const { return m_fraction; }

protected slots:
  void onOKClicked(void);
  void slotDataChanged(void);

private:
  QString m_strSymbolName;
  QString m_strName;
  int     m_fraction;
};

#endif
