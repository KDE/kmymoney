/***************************************************************************
                          kmymoneyreconcilecombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyreconcilecombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

using namespace eMyMoney;

KMyMoneyReconcileCombo::KMyMoneyReconcileCombo(QWidget* w) :
    KMyMoneyMVCCombo(false, w)
{
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  addItem(i18n("Reconciled"), QVariant("R"));
  addItem(i18nc("Reconciliation state 'Cleared'", "Cleared"), QVariant("C"));
  addItem(i18n("Not reconciled"), QVariant(" "));
  addItem(" ", QVariant("U"));

  connect(this, &KMyMoneyMVCCombo::itemSelected, this, &KMyMoneyReconcileCombo::slotSetState);
}

KMyMoneyReconcileCombo::~KMyMoneyReconcileCombo()
{
}

void KMyMoneyReconcileCombo::slotSetState(const QString& state)
{
  setSelectedItem(state);
}

void KMyMoneyReconcileCombo::removeDontCare()
{
  //Remove unknown state
  removeItem(3);
}

void KMyMoneyReconcileCombo::setState(Split::State state)
{
  QString id;

  switch (state) {
    case Split::State::NotReconciled:
      id = ' ';
      break;
    case Split::State::Cleared:
      id = 'C';
      break;
    case Split::State::Reconciled:
      id = 'R';
      break;
    case Split::State::Frozen:
      id = 'F';
      break;
    case Split::State::Unknown:
      id = 'U';
      break;
    default:
      qDebug() << "Unknown reconcile state '" << (int)state << "' in KMyMoneyReconcileCombo::setState()\n";
      break;
  }
  setSelectedItem(id);
}

Split::State KMyMoneyReconcileCombo::state() const
{
  Split::State state = Split::State::NotReconciled;

  QVariant data = itemData(currentIndex());
  QString dataVal;
  if (data.isValid())
    dataVal = data.toString();
  else
    return state;

  if (!dataVal.isEmpty()) {
    if (dataVal == "C")
      state = Split::State::Cleared;
    if (dataVal == "R")
      state = Split::State::Reconciled;
    if (dataVal == "F")
      state = Split::State::Frozen;
    if (dataVal == "U")
      state = Split::State::Unknown;
  }
  return state;
}
