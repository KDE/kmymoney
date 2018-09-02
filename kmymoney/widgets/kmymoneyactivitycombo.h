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

protected Q_SLOTS:
  void slotSetActivity(const QString& id);

Q_SIGNALS:
  void activitySelected(eMyMoney::Split::InvestmentTransactionType);

private:
  Q_DECLARE_PRIVATE(KMyMoneyActivityCombo)
};

#endif
