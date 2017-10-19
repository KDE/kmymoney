/***************************************************************************
                          ksettingsreports.h
                             -------------------
    copyright            : (C) 2010 by Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSREPORTS_H
#define KSETTINGSREPORTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsreportsdecl.h"

class KSettingsReportsDecl : public QWidget, public Ui::KSettingsReportsDecl
{
public:
  KSettingsReportsDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KSettingsReports : public KSettingsReportsDecl
{
  Q_OBJECT

public:
  KSettingsReports(QWidget* parent = 0);
  ~KSettingsReports();

protected slots:
  void slotCssUrlSelected(const QUrl&);
  void slotEditingFinished();

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

};
#endif
