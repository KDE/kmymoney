/***************************************************************************
                          kmymoneyactivitycombo.h  -  description
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

#ifndef KMYMONEYACTIVITYCOMBO_H
#define KMYMONEYACTIVITYCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneymvccombo.h"

namespace eMyMoney { namespace Split { enum class InvestmentTransactionType; } }

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible activities
  * for investment transactions (buy, sell, dividend, etc.)
  */
class KMyMoneyActivityComboPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyActivityCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyActivityCombo)

public:
  /**
    * Create a combo box that contains the entries "Buy", "Sell" etc.
    */
  explicit KMyMoneyActivityCombo(QWidget *w = 0);
  ~KMyMoneyActivityCombo() override;

  void setActivity(eMyMoney::Split::InvestmentTransactionType activity);
  eMyMoney::Split::InvestmentTransactionType activity() const;

protected slots:
  void slotSetActivity(const QString& id);

signals:
  void activitySelected(eMyMoney::Split::InvestmentTransactionType);

private:
  Q_DECLARE_PRIVATE(KMyMoneyActivityCombo)
};

#endif
