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

#include "kbudgetvalues.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QTimer>
#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QPushButton>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include "kmymoneyglobalsettings.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyedit.h"

KBudgetValues::KBudgetValues(QWidget* parent) :
    KBudgetValuesDecl(parent),
    m_currentTab(m_monthlyButton)
{
  m_budgetDate = QDate(2007, KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

  m_field[0] = m_amount1;
  m_field[1] = m_amount2;
  m_field[2] = m_amount3;
  m_field[3] = m_amount4;
  m_field[4] = m_amount5;
  m_field[5] = m_amount6;
  m_field[6] = m_amount7;
  m_field[7] = m_amount8;
  m_field[8] = m_amount9;
  m_field[9] = m_amount10;
  m_field[10] = m_amount11;
  m_field[11] = m_amount12;

  m_label[0] = m_label1;
  m_label[1] = m_label2;
  m_label[2] = m_label3;
  m_label[3] = m_label4;
  m_label[4] = m_label5;
  m_label[5] = m_label6;
  m_label[6] = m_label7;
  m_label[7] = m_label8;
  m_label[8] = m_label9;
  m_label[9] = m_label10;
  m_label[10] = m_label11;
  m_label[11] = m_label12;

  // fill with standard labels
  m_monthlyButton->setChecked(true);
  m_periodGroup->setId(m_monthlyButton, 0);
  m_periodGroup->setId(m_yearlyButton, 1);
  m_periodGroup->setId(m_individualButton, 2);
  slotChangePeriod(m_periodGroup->id(m_monthlyButton));

  // connect(m_budgetLevel, SIGNAL(currentChanged(QWidget*)), this, SIGNAL(valuesChanged()));
  connect(m_amountMonthly, SIGNAL(valueChanged(QString)), this, SLOT(slotNeedUpdate()));
  connect(m_amountYearly, SIGNAL(valueChanged(QString)), this, SLOT(slotNeedUpdate()));
  m_amountMonthly->installEventFilter(this);
  m_amountYearly->installEventFilter(this);

  for (int i = 0; i < 12; ++i) {
    connect(m_field[i], SIGNAL(valueChanged(QString)), this, SLOT(slotNeedUpdate()));
    m_field[i]->installEventFilter(this);
  }

  connect(m_clearButton, SIGNAL(clicked()), this, SLOT(slotClearAllValues()));
  connect(m_periodGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotChangePeriod(int)));
  connect(this, SIGNAL(valuesChanged()), this, SLOT(slotUpdateClearButton()));

  KGuiItem clearItem(KStandardGuiItem::clear());

  KGuiItem::assign(m_clearButton, clearItem);
  m_clearButton->setText("");
  m_clearButton->setToolTip(clearItem.toolTip());
}


KBudgetValues::~KBudgetValues()
{
}

bool KBudgetValues::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if (o->isWidgetType()
      && (e->type() == QEvent::KeyPress)) {
    QKeyEvent* k = dynamic_cast<QKeyEvent*>(e);
    if ((k->modifiers() & Qt::KeyboardModifierMask) == 0
        || (k->modifiers() & Qt::KeypadModifier) != 0) {
      QKeyEvent evt(e->type(),
                    Qt::Key_Tab, k->modifiers(), QString(),
                    k->isAutoRepeat(), k->count());
      switch (k->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          // send out a TAB key event
          QApplication::sendEvent(o, &evt);
          // don't process this one any further
          rc = true;
          break;
        default:
          break;
      }
    }
  }
  return rc;
}

void KBudgetValues::clear()
{
  blockSignals(true);
  for (int i = 0; i < 12; ++i)
    m_field[i]->setValue(MyMoneyMoney());
  m_amountMonthly->setValue(MyMoneyMoney());
  m_amountYearly->setValue(MyMoneyMoney());
  blockSignals(false);
}

void KBudgetValues::slotClearAllValues()
{
  int tab = m_periodGroup->checkedId();
  if (tab == m_periodGroup->id(m_monthlyButton)) {
    m_amountMonthly->setValue(MyMoneyMoney());
  } else if (tab == m_periodGroup->id(m_yearlyButton)) {
    m_amountYearly->setValue(MyMoneyMoney());
  } else if (tab == m_periodGroup->id(m_individualButton)) {
    for (int i = 0; i < 12; ++i)
      m_field[i]->setValue(MyMoneyMoney());
  }
  emit valuesChanged();
}

void KBudgetValues::slotChangePeriod(int id)
{
  // Prevent a recursive entry of this method due to widget changes
  // performed during execution of this method
  static bool inside = false;
  if (inside)
    return;
  inside = true;

  QWidget *tab = m_periodGroup->button(id);
  fillMonthLabels();

  MyMoneyMoney newValue;
  if (tab == m_monthlyButton) {
    m_firstItemStack->setCurrentIndex(m_firstItemStack->indexOf(m_monthlyPage));
    enableMonths(false);
    m_label[0]->setText(" ");
    if (m_amountMonthly->value().isZero()) {
      if (m_currentTab == m_yearlyButton) {
        newValue = (m_amountYearly->value() / MyMoneyMoney(12, 1)).convert();

      } else if (m_currentTab == m_individualButton) {
        for (int i = 0; i < 12; ++i)
          newValue += m_field[i]->value();
        newValue = (newValue / MyMoneyMoney(12, 1)).convert();
      }
      if (!newValue.isZero()) {
        if (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("You have entered budget values using a different base which would result in a monthly budget of <b>%1</b>. Should this value be used to fill the monthly budget?", newValue.formatMoney("", 2)) + QString("</qt>"), i18nc("Auto assignment (caption)", "Auto assignment"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "use_previous_budget_values") == KMessageBox::Yes) {
          m_amountMonthly->setValue(newValue);
        }
      }
    }

  } else if (tab == m_yearlyButton) {
    m_firstItemStack->setCurrentIndex(m_firstItemStack->indexOf(m_yearlyPage));
    enableMonths(false);
    m_label[0]->setText(" ");
    if (m_amountYearly->value().isZero()) {
      if (m_currentTab == m_monthlyButton) {
        newValue = (m_amountMonthly->value() * MyMoneyMoney(12, 1)).convert();

      } else if (m_currentTab == m_individualButton) {
        for (int i = 0; i < 12; ++i)
          newValue += m_field[i]->value();
      }
      if (!newValue.isZero()) {
        if (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("You have entered budget values using a different base which would result in a yearly budget of <b>%1</b>. Should this value be used to fill the monthly budget?", newValue.formatMoney("", 2)) + QString("</qt>"), i18nc("Auto assignment (caption)", "Auto assignment"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "use_previous_budget_values") == KMessageBox::Yes) {
          m_amountYearly->setValue(newValue);
        }
      }
    }

  } else if (tab == m_individualButton) {
    m_firstItemStack->setCurrentIndex(m_firstItemStack->indexOf(m_individualPage));
    enableMonths(true);
    for (int i = 0; i < 12; ++i)
      newValue += m_field[i]->value();
    if (newValue.isZero()) {
      if (m_currentTab == m_monthlyButton) {
        newValue = m_amountMonthly->value();
      } else if (m_currentTab == m_yearlyButton) {
        newValue = (m_amountYearly->value() / MyMoneyMoney(12, 1)).convert();
      }

      if (!newValue.isZero()) {
        if (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("You have entered budget values using a different base which would result in an individual monthly budget of <b>%1</b>. Should this value be used to fill the monthly budgets?", newValue.formatMoney("", 2)) + QString("</qt>"), i18nc("Auto assignment (caption)", "Auto assignment"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "use_previous_budget_values") == KMessageBox::Yes) {
          for (int i = 0; i < 12; ++i)
            m_field[i]->setValue(newValue);
        }
      }
    }
  }

  slotNeedUpdate();
  m_currentTab = tab;
  inside = false;
}

void KBudgetValues::slotNeedUpdate()
{
  if (!signalsBlocked())
    QTimer::singleShot(0, this, SIGNAL(valuesChanged()));
}

void KBudgetValues::enableMonths(bool enabled)
{
  for (int i = 1; i < 12; ++i) {
    m_label[i]->setEnabled(enabled);
    m_field[i]->setEnabled(enabled);
  }
}

void KBudgetValues::fillMonthLabels()
{
  QDate date(m_budgetDate);
  for (int i = 0; i < 12; ++i) {
    m_label[i]->setText(QLocale().standaloneMonthName(date.month(), QLocale::ShortFormat));
    date = date.addMonths(1);
  }
}

void KBudgetValues::setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount)
{
  MyMoneyBudget::PeriodGroup period;
  m_budgetDate = budget.budgetStart();
  QDate date;

  // make sure all values are zero so that slotChangePeriod()
  // doesn't check for anything.
  clear();

  blockSignals(true);
  switch (budgetAccount.budgetLevel()) {
    case MyMoneyBudget::AccountGroup::eMonthly:
    default:
      m_monthlyButton->setChecked(true);
      slotChangePeriod(m_periodGroup->id(m_monthlyButton));
      m_amountMonthly->setValue(budgetAccount.period(m_budgetDate).amount());
      break;
    case MyMoneyBudget::AccountGroup::eYearly:
      m_yearlyButton->setChecked(true);
      slotChangePeriod(m_periodGroup->id(m_yearlyButton));
      m_amountYearly->setValue(budgetAccount.period(m_budgetDate).amount());
      break;
    case MyMoneyBudget::AccountGroup::eMonthByMonth:
      m_individualButton->setChecked(true);
      slotChangePeriod(m_periodGroup->id(m_individualButton));
      date.setDate(m_budgetDate.year(), m_budgetDate.month(), m_budgetDate.day());
      for (int i = 0; i < 12; ++i) {
        m_field[i]->setValue(budgetAccount.period(date).amount());
        date = date.addMonths(1);
      }
      break;
  }
  slotUpdateClearButton();
  blockSignals(false);
}

void KBudgetValues::budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount)
{
  MyMoneyBudget::PeriodGroup period;
  m_budgetDate = budget.budgetStart();
  period.setStartDate(m_budgetDate);
  QDate date;

  budgetAccount.clearPeriods();
  int tab = m_periodGroup->checkedId();
  if (tab == m_periodGroup->id(m_monthlyButton)) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
    period.setAmount(m_amountMonthly->value());
    budgetAccount.addPeriod(m_budgetDate, period);
  } else if (tab == m_periodGroup->id(m_yearlyButton)) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eYearly);
    period.setAmount(m_amountYearly->value());
    budgetAccount.addPeriod(m_budgetDate, period);
  } else if (tab == m_periodGroup->id(m_individualButton)) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthByMonth);
    date.setDate(m_budgetDate.year(), m_budgetDate.month(), m_budgetDate.day());
    for (int i = 0; i < 12; ++i) {
      period.setStartDate(date);
      period.setAmount(m_field[i]->value());
      budgetAccount.addPeriod(date, period);
      date = date.addMonths(1);
    }
  }
}

void KBudgetValues::slotUpdateClearButton()
{
  bool rc = false;
  int tab = m_periodGroup->checkedId();
  if (tab == m_periodGroup->id(m_monthlyButton)) {
    rc = !m_amountMonthly->value().isZero();
  } else if (tab == m_periodGroup->id(m_yearlyButton)) {
    rc = !m_amountYearly->value().isZero();
  } else if (tab == m_periodGroup->id(m_individualButton)) {
    for (int i = 0; (i < 12) && (rc == false); ++i) {
      rc |= !m_field[i]->value().isZero();
    }
  }
  m_clearButton->setEnabled(rc);
}
