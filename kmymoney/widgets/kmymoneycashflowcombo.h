/*
    SPDX-FileCopyrightText: 2009-2010 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

namespace eMyMoney {
namespace Account {
enum class Type;
}
}
namespace eWidgets {
namespace eRegister {
enum class CashFlowDirection;
}
}

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
