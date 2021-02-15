/*
    SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
