/***************************************************************************
                             ksettingsicons.cpp
                             --------------------
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

#include "ksettingsicons.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QDir>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KSettingsIcons::KSettingsIcons(QWidget* parent) :
    KSettingsIconsDecl(parent)
{
  // hide the internally used holidayRegion field
  kcfg_IconsTheme->hide();

  loadList();

  // setup connections so that region gets selected once field is filled
  connect(kcfg_IconsTheme, SIGNAL(textChanged(QString)), this, SLOT(slotLoadTheme(QString)));

  // setup connections so that changes are forwarded to the field
  connect(m_IconsTheme, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetTheme(int)));
}

void KSettingsIcons::loadList()
{
  QStringList themes {QStringLiteral("oxygen"), QStringLiteral("Tango")};
  QStringList searchPaths = QIcon::themeSearchPaths();
  m_IconsTheme->addItem(QStringLiteral("system"));
  m_themesMap.insert(0, QStringLiteral("system"));
  for (int i = 0; i < searchPaths.count(); ++i) {
    for (int j = 0; j < themes.count(); ++j) {
      QDir themeDir = QDir(searchPaths.at(i)).filePath(themes.at(j));
      if (themeDir.exists()) {
        m_IconsTheme->addItem(themes.at(j));
        m_themesMap.insert(m_themesMap.count(), themes.at(j));
      }
    }
  }
}

void KSettingsIcons::slotSetTheme(const int &theme)
{
  kcfg_IconsTheme->setText(m_themesMap.value(theme));
}

void KSettingsIcons::slotLoadTheme(const QString &theme)
{
  // only need this once
  disconnect(kcfg_IconsTheme, SIGNAL(textChanged(QString)), this, SLOT(slotLoadTheme(QString)));
  int i = 0;
  if (!theme.isEmpty())
    i = m_IconsTheme->findText(theme);
  if ((i > -1) && (i != m_IconsTheme->currentIndex())) {
    m_IconsTheme->blockSignals(true);
    m_IconsTheme->setCurrentIndex(i);
    m_IconsTheme->blockSignals(false);
  }
}

void KSettingsIcons::slotResetTheme()
{
  slotLoadTheme(kcfg_IconsTheme->text());
}

KSettingsIcons::~KSettingsIcons()
{
}
