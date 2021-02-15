/***************************************************************************
                             kinstitutionssview.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KINSTITUTIONSVIEW_H
#define KINSTITUTIONSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

class MyMoneyInstitution;
class MyMoneyMoney;

/**
  * @author Thomas Baumgart
  */
/**
  * This class implements the institutions hierarchical 'view'.
  */
class KInstitutionsViewPrivate;
class KInstitutionsView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KInstitutionsView(QWidget *parent = nullptr);
  ~KInstitutionsView();

  void executeCustomAction(eView::Action action) override;
  void refresh();
  void updateActions(const MyMoneyObject &obj);

public Q_SLOTS:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotShowInstitutionsMenu(const MyMoneyInstitution& inst);
  void slotEditInstitution();

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

protected:
  void showEvent(QShowEvent * event) override;

private:
  Q_DECLARE_PRIVATE(KInstitutionsView)

private Q_SLOTS:
  void slotNewInstitution();
  void slotDeleteInstitution();
};

#endif
