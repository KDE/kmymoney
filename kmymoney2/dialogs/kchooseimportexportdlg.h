/***************************************************************************
                          kchooseimportexportdlg.h  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
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

#include <qwidget.h>
#include "ui_kchooseimportexportdlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KChooseImportExportDlgDecl : public QDialog, public Ui::KChooseImportExportDlgDecl
{
public:
  KChooseImportExportDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KChooseImportExportDlg : public KChooseImportExportDlgDecl  {
   Q_OBJECT
private:
  void readConfig(void);
  void writeConfig(void);
  QString m_lastType;

protected slots:
  void slotTypeActivated(const QString& text);

public:
	KChooseImportExportDlg(int type, QWidget *parent=0);
	~KChooseImportExportDlg();
	QString importExportType(void);
};

#endif
