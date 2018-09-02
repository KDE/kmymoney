/*
 * Copyright 2009-2010  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2011-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
