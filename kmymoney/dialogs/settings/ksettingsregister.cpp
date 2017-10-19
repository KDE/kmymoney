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

#include <KLineEdit>

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
  connect(kcfg_sortNormalView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadNormal(QString)));
  connect(kcfg_sortReconcileView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadReconcile(QString)));
  connect(kcfg_sortSearchView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadSearch(QString)));

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(m_sortNormalView, SIGNAL(settingsChanged(QString)), kcfg_sortNormalView, SLOT(setText(QString)));
  connect(m_sortReconcileView, SIGNAL(settingsChanged(QString)), kcfg_sortReconcileView, SLOT(setText(QString)));
  connect(m_sortSearchView, SIGNAL(settingsChanged(QString)), kcfg_sortSearchView, SLOT(setText(QString)));
}

KSettingsRegister::~KSettingsRegister()
{
}

void KSettingsRegister::slotLoadNormal(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortNormalView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadNormal(QString)));
  m_sortNormalView->setSettings(text);
}

void KSettingsRegister::slotLoadReconcile(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortReconcileView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadReconcile(QString)));
  m_sortReconcileView->setSettings(text);
}

void KSettingsRegister::slotLoadSearch(const QString& text)
{
  // only need this once
  disconnect(kcfg_sortSearchView, SIGNAL(textChanged(QString)), this, SLOT(slotLoadSearch(QString)));
  m_sortSearchView->setSettings(text);
}
