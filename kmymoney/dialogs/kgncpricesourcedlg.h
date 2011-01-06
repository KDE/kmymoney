/***************************************************************************
                          kgncpricesourcedlg.h
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

#ifndef KGNCPRICESOURCEDLG_H
#define KGNCPRICESOURCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Includes

class KGncPriceSourceDlg : public KDialog
{
  Q_OBJECT
public:
  KGncPriceSourceDlg(QWidget *parent = 0);
  KGncPriceSourceDlg(const QString &stockName, const QString &gncSource , QWidget * parent = 0);
  ~KGncPriceSourceDlg();

  QString selectedSource() const;
  bool alwaysUse() const;

public slots:
  void buttonPressed(int);
  void slotHelp();

private:
  /// \internal d-pointer class.
  struct Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
