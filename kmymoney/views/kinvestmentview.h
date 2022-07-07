/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINVESTMENTVIEW_H
#define KINVESTMENTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class SelectedObjects;

/**
  * @author Kevin Tambascio
  * @author Łukasz Wojniłowicz
  */
class KInvestmentViewPrivate;
class KInvestmentView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit KInvestmentView(QWidget *parent = nullptr);
    ~KInvestmentView() override;

    void setDefaultFocus() override;

    void updateActions(const SelectedObjects& selections) override;
    void executeAction(eMenu::Action action, const SelectedObjects& selections) override;

public Q_SLOTS:
    void slotSettingsChanged() override;

protected:
    void showEvent(QShowEvent* event) override;

private:
    Q_DECLARE_PRIVATE(KInvestmentView)

private Q_SLOTS:
    void slotNewInvestment();
    void slotEditInvestment();
    void slotDeleteInvestment();
    void slotUpdatePriceOnline();
    void slotUpdatePriceManually();

    void slotEditSecurity();
    void slotDeleteSecurity();
};

#endif
