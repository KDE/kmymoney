/***************************************************************************
                          kbudgetvalues  -  description
                             -------------------
    begin                : Wed Nov 28 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBUDGETVALUES_H
#define KBUDGETVALUES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybudget.h"

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */

class KBudgetValuesPrivate;
class KBudgetValues : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KBudgetValues)

public:
  explicit KBudgetValues(QWidget* parent = nullptr);
  ~KBudgetValues();

  void setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount);
  void budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount);
  void clear();

signals:
  void valuesChanged();

protected slots:
  void slotChangePeriod(int id);

  /**
   * This slot clears the value in the value widgets of the selected budget type.
   * Values of the other types are unaffected.
   */
  void slotClearAllValues();

  /**
   * Helper slot used to postpone sending the valuesChanged() signal.
   */
  void slotNeedUpdate();

  void slotUpdateClearButton();

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  KBudgetValuesPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KBudgetValues)
};

#endif
