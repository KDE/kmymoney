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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingsregister.h"
#include <transactionsortoption.h>

KSettingsRegister::KSettingsRegister(QWidget* parent) :
  KSettingsRegisterDecl(parent)
{
  // hide the internally used text fields
  kcfg_sortNormalView->hide();
  kcfg_sortReconcileView->hide();
  kcfg_sortSearchView->hide();

  // setup connections, so that the sort optios get loaded once the edit fields are filled
  connect(kcfg_sortNormalView, SIGNAL(textChanged()), this, SLOT(slotLoadNormal()));
  connect(kcfg_sortReconcileView, SIGNAL(textChanged()), this, SLOT(slotLoadReconcile()));
  connect(kcfg_sortSearchView, SIGNAL(textChanged()), this, SLOT(slotLoadSearch()));

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(m_sortNormalView, SIGNAL(settingsChanged(const QString&)), kcfg_sortNormalView, SLOT(setText(const QString&)));
  connect(m_sortReconcileView, SIGNAL(settingsChanged(const QString&)), kcfg_sortReconcileView, SLOT(setText(const QString&)));
  connect(m_sortSearchView, SIGNAL(settingsChanged(const QString&)), kcfg_sortSearchView, SLOT(setText(const QString&)));
}

KSettingsRegister::~KSettingsRegister()
{
}

void KSettingsRegister::slotLoadNormal(void)
{
  // only need this once
  disconnect(kcfg_sortNormalView, SIGNAL(textChanged()), this, SLOT(slotLoadNormal()));
  m_sortNormalView->setSettings(kcfg_sortNormalView->text());
}

void KSettingsRegister::slotLoadReconcile(void)
{
  // only need this once
  disconnect(kcfg_sortReconcileView, SIGNAL(textChanged()), this, SLOT(slotLoadReconcile()));
  m_sortReconcileView->setSettings(kcfg_sortReconcileView->text());
}

void KSettingsRegister::slotLoadSearch(void)
{
  // only need this once
  disconnect(kcfg_sortSearchView, SIGNAL(textChanged()), this, SLOT(slotLoadSearch()));
  m_sortSearchView->setSettings(kcfg_sortSearchView->text());
}

#include "ksettingsregister.moc"
