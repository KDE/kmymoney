/***************************************************************************
                             budgetview.h
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz
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

#ifndef BUDGETVIEW_H
#define BUDGETVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KBudgetView;

class BudgetView : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit BudgetView(QObject *parent, const QVariantList &args);
  ~BudgetView() override;

  void plug(KXMLGUIFactory* guiFactory) final override;
  void unplug() final override;

private:
  KBudgetView* m_view;
};

#endif
