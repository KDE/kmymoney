/***************************************************************************
                          kloadtemplatedlg.h
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KLOADTEMPLATEDLG_H
#define KLOADTEMPLATEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloadtemplatedlgdecl.h"
#include <QList>

class MyMoneyTemplate;
class KLoadTemplateDlgDecl : public QDialog, public Ui::KLoadTemplateDlgDecl
{
public:
  KLoadTemplateDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

/// This dialog lets the user load more account templates
class KLoadTemplateDlg : public KLoadTemplateDlgDecl
{
  Q_OBJECT

public:
  KLoadTemplateDlg(QWidget *parent = 0);

  QList<MyMoneyTemplate> templates() const;

private slots:
  void slotHelp();
};

#endif
