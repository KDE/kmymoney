/*
 * Copyright 2009-2010  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2011-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYCASHFLOWCOMBO_H
#define KMYMONEYCASHFLOWCOMBO_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneymvccombo.h"

namespace eMyMoney { namespace Account { enum class Type; } }
namespace eWidgets { namespace eRegister { enum class CashFlowDirection; } }

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * actions (Deposit, Withdrawal, etc.).
  */
class KMyMoneyCashFlowComboPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyCashFlowCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCashFlowCombo)

public:
  /**
    * Create a combo box that contains the entries "Pay to", "From" and
    * "  " for don't care.
    */
  explicit KMyMoneyCashFlowCombo(eMyMoney::Account::Type type, QWidget *parent = nullptr);
  ~KMyMoneyCashFlowCombo() override;

  void setDirection(eWidgets::eRegister::CashFlowDirection dir);
  eWidgets::eRegister::CashFlowDirection direction() const;

  void removeDontCare();

protected Q_SLOTS:
  void slotSetDirection(const QString& id);

Q_SIGNALS:
  void directionSelected(eWidgets::eRegister::CashFlowDirection);

private:
  Q_DECLARE_PRIVATE(KMyMoneyCashFlowCombo)
};

#endif
