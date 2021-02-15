/***************************************************************************
                             wizardpage_p.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef WIZARDPAGE_P_H
#define WIZARDPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage_p.h"

class KMyMoneyWizard;

template <class T>
class WizardPagePrivate : public KMyMoneyWizardPagePrivate
{
public:
  WizardPagePrivate(QObject* parent) :
    KMyMoneyWizardPagePrivate(parent),
    m_wizard(nullptr),
    m_wizardBase(nullptr)
  {
  }

  T*                m_wizard;
  KMyMoneyWizard*   m_wizardBase;
};

#endif
