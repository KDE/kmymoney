/***************************************************************************
                          ksettingsicons.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSICONS_H
#define KSETTINGSICONS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsiconsdecl.h"

class KSettingsIconsDecl : public QWidget, public Ui::KSettingsIconsDecl
{
public:
  KSettingsIconsDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KSettingsIcons : public KSettingsIconsDecl
{
  Q_OBJECT

public:
  KSettingsIcons(QWidget* parent = 0);
  ~KSettingsIcons();

public slots:
  void slotResetTheme();

protected slots:
  void slotLoadTheme(const QString &theme);
  void slotSetTheme(const int &theme);

protected:
  void loadList();
private:
  QMap<int, QString> m_themesMap;
};
#endif

