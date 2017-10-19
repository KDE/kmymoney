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

#include <QDate>
class QLabel;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetvaluesdecl.h"
#include <mymoneybudget.h>
class kMyMoneyEdit;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */

class KBudgetValuesDecl : public QWidget, public Ui::KBudgetValuesDecl
{
public:
  KBudgetValuesDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KBudgetValues : public KBudgetValuesDecl
{
  Q_OBJECT
public:
  KBudgetValues(QWidget* parent = 0);
  ~KBudgetValues();

  void setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount);
  void budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount);
  void clear();

private:
  void enableMonths(bool enabled);
  void fillMonthLabels();

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
  bool eventFilter(QObject* o, QEvent* e);

private:
  kMyMoneyEdit*   m_field[12];
  QLabel*         m_label[12];
  QWidget*        m_currentTab;
  QDate           m_budgetDate;

signals:
  void valuesChanged();
};

#endif
