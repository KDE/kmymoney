/***************************************************************************
                          kmymoneycashflowcombo.h  -  description
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

#ifndef KMYMONEYCASHFLOWCOMBO_H
#define KMYMONEYCASHFLOWCOMBO_H

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
class KMM_WIDGETS_EXPORT KMyMoneyCashFlowCombo : public KMyMoneyMVCCombo
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

protected slots:
  void slotSetDirection(const QString& id);

signals:
  void directionSelected(eWidgets::eRegister::CashFlowDirection);

private:
  Q_DECLARE_PRIVATE(KMyMoneyCashFlowCombo)
};

#endif
