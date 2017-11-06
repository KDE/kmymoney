/***************************************************************************
                          kchooseimportexportdlg.h  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
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

#ifndef KCHOOSEIMPORTEXPORTDLG_H
#define KCHOOSEIMPORTEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  *@author Michael Edwardes
  */

class KChooseImportExportDlgPrivate;
class KChooseImportExportDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KChooseImportExportDlg)

public:
  explicit KChooseImportExportDlg(int type, QWidget *parent = nullptr);
  ~KChooseImportExportDlg();
  QString importExportType() const;

protected slots:
  void slotTypeActivated(const QString& text);

private:
  KChooseImportExportDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KChooseImportExportDlg)
};

#endif
