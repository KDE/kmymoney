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

#ifndef KNEWEQUITYENTRYDLG_H
#define KNEWEQUITYENTRYDLG_H

#include <QDialog>

/**
  *
  * Dialog to allow user to enter all data for a stock or mutual fund investment type.
  *
  * @author Kevin Tambascio
  *
  */

class KNewEquityEntryDlgPrivate;
class KNewEquityEntryDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KNewEquityEntryDlg)

public:
  explicit KNewEquityEntryDlg(QWidget *parent = nullptr);
  virtual ~KNewEquityEntryDlg();

  void setSymbolName(const QString& str);
  QString symbolName() const;

  void setName(const QString& str);
  QString name() const;

  int fraction() const;

protected Q_SLOTS:
  void onOKClicked();
  void slotDataChanged();

private:
  KNewEquityEntryDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KNewEquityEntryDlg)
};

#endif
