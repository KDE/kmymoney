/*
    SPDX-FileCopyrightText: 2009-2010 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

namespace eMyMoney {
namespace Split {
enum class InvestmentTransactionType;
}
}

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
