/***************************************************************************
                          keditloanwizard.h  -  description
                             -------------------
    begin                : Wed Nov 12 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KEDITLOANWIZARD_H
#define KEDITLOANWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "knewloanwizard.h"

/**
  * @author Thomas Baumgart
  */
class KEditLoanWizardPrivate;
class KEditLoanWizard : public KNewLoanWizard
{
  Q_OBJECT
public:
  explicit KEditLoanWizard(const MyMoneyAccount& account, QWidget *parent = nullptr);
  ~KEditLoanWizard() override;

  /**
    * This method returns the schedule for the payments. The account
    * where the amortization should be transferred to is the one
    * we currently edited with this wizard.
    *
    * @return MyMoneySchedule object for payments
    */
  const MyMoneySchedule schedule() const;

  /**
    * This method returns a MyMoneyAccount object with all data
    * filled out as provided by the wizard.
    *
    * @return updated MyMoneyAccount object
    */
  const MyMoneyAccount account() const;


  void loadWidgets(const MyMoneyAccount& acc);

  const MyMoneyTransaction transaction() const;

  bool validateCurrentPage() final override;

protected:
  void updateEditSummary();

private:
  Q_DISABLE_COPY(KEditLoanWizard)
  Q_DECLARE_PRIVATE(KEditLoanWizard)
};

#endif
