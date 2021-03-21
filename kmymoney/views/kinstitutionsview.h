/*
* SPDX-FileCopyrightText: 2007-2019 Thomas Baumgart <tbaumgart@kde.org>
* SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINSTITUTIONSVIEW_H
#define KINSTITUTIONSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class MyMoneyInstitution;
class MyMoneyMoney;
class SelectedObjects;

/**
  * @author Thomas Baumgart
  */
/**
  * This class implements the institutions hierarchical 'view'.
  */
class KInstitutionsViewPrivate;
class KInstitutionsView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit KInstitutionsView(QWidget *parent = nullptr);
    ~KInstitutionsView();

    void executeCustomAction(eView::Action action) override;

public Q_SLOTS:
    void slotNetWorthChanged(const MyMoneyMoney &netWorth, bool isApproximate);
    void slotEditInstitution();

    void slotSettingsChanged() override;
    void updateActions(const SelectedObjects& selections) override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(KInstitutionsView)

private Q_SLOTS:
    void slotNewInstitution();
    void slotDeleteInstitution();
};

#endif
