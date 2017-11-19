/***************************************************************************
                             wizardpage.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include "wizardpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage.h"

class KMyMoneyWizard;

/**
 * The general base class for wizard pages
 *
 * @author Thomas Baumgart
 */

template <class T>
class WizardPage : public KMyMoneyWizardPage
{
public:
  WizardPage(uint step, QWidget* widget, T* parent) :
    KMyMoneyWizardPage(*new WizardPagePrivate<T>(widget), step, widget)
  {
    d_func()->m_wizard = parent;
    d_func()->m_wizardBase = parent;
  }

  ~WizardPage() override
  {
  }

  virtual KMyMoneyWizard* wizard() const
  {
    return d_func()->m_wizardBase;
  }

protected:
  using KMyMoneyWizardPage::d_ptr;
  inline WizardPagePrivate<T>* d_func() { return reinterpret_cast<WizardPagePrivate<T> *>(qGetPtrHelper(d_ptr)); }
  inline const WizardPagePrivate<T>* d_func() const { return reinterpret_cast<const WizardPagePrivate<T> *>(qGetPtrHelper(d_ptr)); }
  friend class WizardPagePrivate<T>;
  WizardPage(WizardPagePrivate<T> &dd, uint step, QWidget* widget, T* parent) :
    KMyMoneyWizardPage(dd, step, widget)
  {
    d_func()->m_wizard = parent;
    d_func()->m_wizardBase = parent;
  }
};

#endif
