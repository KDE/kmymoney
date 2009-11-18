/***************************************************************************
                          konlinequoteconfigurationdlg.h  -  description
                             -------------------
    begin                : Tuesday July 1st, 2004
    copyright            : (C) 2004 by Kevin Tambascio
    email                : ktambascio@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONLINEQUOTECONFIGURATIONDLG_H
#define KONLINEQUOTECONFIGURATIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_konlinequoteconfigurationdecl.h"

/**
  * @author Kevin Tambascio
  */

class kOnlineQuoteConfigurationDecl : public QDialog, public Ui::kOnlineQuoteConfigurationDecl
{
public:
  kOnlineQuoteConfigurationDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};
class KOnlineQuoteConfigurationDlg : public kOnlineQuoteConfigurationDecl
{
  Q_OBJECT
public:
  KOnlineQuoteConfigurationDlg(QWidget *parent);
  ~KOnlineQuoteConfigurationDlg();

};

#endif
