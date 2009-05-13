/***************************************************************************
                             ksettingsgeneral.cpp
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

#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneydateinput.h>
#include "ksettingsgeneral.h"

KSettingsGeneral::KSettingsGeneral(QWidget* parent) :
  KSettingsGeneralDecl(parent)
{
  // hide the internally used date field
  kcfg_StartDate->hide();
  kcfg_hiddenViews->hide();

  // for now, we don't show the widgets for view selection
  m_viewLabel->hide();
  m_viewList->hide();

  // setup connections, so that the sort optios get loaded once the edit fields are filled
  connect(kcfg_StartDate, SIGNAL(valueChanged(const QDate&)), this, SLOT(slotLoadStartDate(const QDate&)));

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(m_startDateEdit, SIGNAL(dateChanged(const QDate&)), kcfg_StartDate, SLOT(setDate(const QDate&)));
}

KSettingsGeneral::~KSettingsGeneral()
{
}

void KSettingsGeneral::slotLoadStartDate(const QDate&)
{
  // only need this once
  disconnect(kcfg_StartDate, SIGNAL(valueChanged(const QDate&)), this, SLOT(slotLoadStartDate(const QDate&)));
  m_startDateEdit->setDate(kcfg_StartDate->date());
}

#include "ksettingsgeneral.moc"
