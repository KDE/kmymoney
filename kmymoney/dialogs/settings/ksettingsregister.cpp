/***************************************************************************
                             ksettingsregister.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include "ksettingsregister.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

KSettingsRegister::KSettingsRegister(QWidget* parent) :
    KSettingsRegisterDecl(parent)
{

  // hide the internally used text fields
  kcfg_sortNormalView->hide();
  kcfg_sortReconcileView->hide();
  kcfg_sortSearchView->hide();

  // setup connections, so that the sort optios get loaded once the edit fields are filled
  connect(kcfg_sortNormalView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadNormal(const QString&)));
  connect(kcfg_sortReconcileView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadReconcile(const QString&)));
  connect(kcfg_sortSearchView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadSearch(const QString&)));

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(m_sortNormalView, SIGNAL(settingsChanged(const QString&)), kcfg_sortNormalView, SLOT(setText(const QString&)));
  connect(m_sortReconcileView, SIGNAL(settingsChanged(const QString&)), kcfg_sortReconcileView, SLOT(setText(const QString&)));
  connect(m_sortSearchView, SIGNAL(settingsChanged(const QString&)), kcfg_sortSearchView, SLOT(setText(const QString&)));
}

KSettingsRegister::~KSettingsRegister()
{
}

void KSettingsRegister::slotLoadNormal(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortNormalView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadNormal(const QString&)));
  m_sortNormalView->setSettings(text);
}

void KSettingsRegister::slotLoadReconcile(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortReconcileView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadReconcile(const QString&)));
  m_sortReconcileView->setSettings(text);
}

void KSettingsRegister::slotLoadSearch(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortSearchView, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadSearch(const QString&)));
  m_sortSearchView->setSettings(text);
}

#include "ksettingsregister.moc"
