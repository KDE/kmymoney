 /*
 * Copyright 2007-2019  Thomas Baumgart <tbaumgart@kde.org>
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
  void refresh();
  void updateActions(const MyMoneyObject &obj) Q_DECL_DEPRECATED;

public Q_SLOTS:
  void slotNetWorthChanged(const MyMoneyMoney &netWorth, bool isApproximate);
  void slotEditInstitution();

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

  void slotSettingsChanged() override;
  void updateActions(const SelectedObjects& selections) override;

protected:
  void showEvent(QShowEvent * event) override;

private:
  Q_DECLARE_PRIVATE(KInstitutionsView)

private Q_SLOTS:
  void slotNewInstitution();
  void slotDeleteInstitution();
};

#endif
