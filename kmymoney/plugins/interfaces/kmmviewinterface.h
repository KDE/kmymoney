/***************************************************************************
                          kmmviewinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KMMVIEWINTERFACE_H
#define KMMVIEWINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KMyMoneyView;

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"

namespace KMyMoneyPlugin
{

/**
  * This class represents the implementation of the
  * ViewInterface.
  */
class KMMViewInterface : public ViewInterface
{
  Q_OBJECT

public:
  KMMViewInterface(KMyMoneyView* view, QObject* parent, const char* name = 0);
  ~KMMViewInterface() {}

  void slotRefreshViews() override;

  void addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon) override;
  void removeView(View idView) override;

private:
  KMyMoneyView* m_view;
};

} // namespace
#endif
