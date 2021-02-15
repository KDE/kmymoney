/***************************************************************************
                             khierarchypage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KHIERARCHYPAGE_P_H
#define KHIERARCHYPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_khierarchypage.h"

#include "wizardpage_p.h"
#include "mymoneyaccount.h"

namespace NewAccountWizard
{
  class Wizard;
  class HierarchyFilterProxyModel;

  class HierarchyPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(HierarchyPagePrivate)

  public:
    explicit HierarchyPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KHierarchyPage),
      m_filterProxyModel(nullptr)
    {
    }

    ~HierarchyPagePrivate()
    {
      delete ui;
    }

    Ui::KHierarchyPage        *ui;
    HierarchyFilterProxyModel *m_filterProxyModel;
    MyMoneyAccount             m_parentAccount;
  };
}

#endif
