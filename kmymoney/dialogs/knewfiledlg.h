/***************************************************************************
                          knewfiledlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWFILEDLG_H
#define KNEWFILEDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "ui_knewfiledlgdecl.h"

// This dialog lets the user create/edit a file.
// Use the second constructor to edit a file.

class KNewFileDlgDecl : public QDialog, public Ui::KNewFileDlgDecl
{
public:
  KNewFileDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KNewFileDlg : public KNewFileDlgDecl
{
  Q_OBJECT
public:
  explicit KNewFileDlg(QWidget *parent = 0, const QString& title = QString());
  explicit KNewFileDlg(QString userName, QString userStreet,
                       QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
                       QString userEmail, QWidget *parent = 0, const QString& title = QString());
  ~KNewFileDlg();

  KPushButton* cancelButton(void) {
    return cancelBtn;
  };

public:
  QString userNameText;
  QString userStreetText;
  QString userTownText;
  QString userCountyText;
  QString userPostcodeText;
  QString userTelephoneText;
  QString userEmailText;

protected:
  /// helper method for constructors
  void init(const QString& title);

protected slots:
  void okClicked();
  void loadFromKABC(void);
};

#endif
