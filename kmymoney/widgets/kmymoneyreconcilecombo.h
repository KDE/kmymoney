/***************************************************************************
                          kmymoneyreconcilecombo.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
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

#ifndef KMYMONEYRECONCILECOMBO_H
#define KMYMONEYRECONCILECOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneymvccombo.h"

namespace eMyMoney { namespace Split { enum class State; } }

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * reconciliation.
  */

class KMM_WIDGETS_EXPORT KMyMoneyReconcileCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyReconcileCombo)

public:
  explicit KMyMoneyReconcileCombo(QWidget *w = 0);
  ~KMyMoneyReconcileCombo() override;

  void setState(eMyMoney::Split::State state);
  eMyMoney::Split::State state() const;
  void removeDontCare();

protected Q_SLOTS:
  void slotSetState(const QString&);
};

#endif
