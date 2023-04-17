/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbudgetvalues.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
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

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetvalues.h"

#include "mymoneybudget.h"
#include "kmymoneysettings.h"
#include "mymoneyenums.h"

#include "kmmyesno.h"

class KBudgetValuesPrivate
{
    Q_DISABLE_COPY(KBudgetValuesPrivate)

public:
    KBudgetValuesPrivate() :
        ui(new Ui::KBudgetValues),
        m_currentTab(nullptr)
    {
        for (int i = 0; i < 12; ++i) {
            m_label[i] = nullptr;
            m_field[i] = nullptr;
        }
    }

    ~KBudgetValuesPrivate()
    {
        delete ui;
    }

    void enableMonths(bool enabled)
    {
        for (int i = 1; i < 12; ++i) {
            m_label[i]->setEnabled(enabled);
            m_field[i]->setEnabled(enabled);
        }
    }

    void fillMonthLabels(QWidget* tab)
    {
        QDate date(m_budgetDate);
        for (auto i = 0; i < 12; ++i) {
            m_label[i]->setText(QLocale().standaloneMonthName(date.month(), QLocale::ShortFormat));
            date = date.addMonths(1);
        }
        // for monthly and yearly entries we remove the label of the first field
        if (tab != ui->m_individualButton) {
            m_label[0]->setText(" ");
        }
    }

    Ui::KBudgetValues *ui;
    AmountEdit*     m_field[12];
    QLabel*         m_label[12];
    QWidget*        m_currentTab;
    QDate           m_budgetDate;
};

KBudgetValues::KBudgetValues(QWidget* parent) :
    QWidget(parent),
    d_ptr(new KBudgetValuesPrivate)
{
    Q_D(KBudgetValues);
    d->ui->setupUi(this);
    d->m_currentTab = d->ui->m_monthlyButton;
    d->m_budgetDate = QDate(QDate::currentDate().year(), KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());

    d->m_field[0] = d->ui->m_amount1;
    d->m_field[1] = d->ui->m_amount2;
    d->m_field[2] = d->ui->m_amount3;
    d->m_field[3] = d->ui->m_amount4;
    d->m_field[4] = d->ui->m_amount5;
    d->m_field[5] = d->ui->m_amount6;
    d->m_field[6] = d->ui->m_amount7;
    d->m_field[7] = d->ui->m_amount8;
    d->m_field[8] = d->ui->m_amount9;
    d->m_field[9] = d->ui->m_amount10;
    d->m_field[10] = d->ui->m_amount11;
    d->m_field[11] = d->ui->m_amount12;

    d->m_label[0] = d->ui->m_label1;
    d->m_label[1] = d->ui->m_label2;
    d->m_label[2] = d->ui->m_label3;
    d->m_label[3] = d->ui->m_label4;
    d->m_label[4] = d->ui->m_label5;
    d->m_label[5] = d->ui->m_label6;
    d->m_label[6] = d->ui->m_label7;
    d->m_label[7] = d->ui->m_label8;
    d->m_label[8] = d->ui->m_label9;
    d->m_label[9] = d->ui->m_label10;
    d->m_label[10] = d->ui->m_label11;
    d->m_label[11] = d->ui->m_label12;

    // fill with standard labels
    d->ui->m_monthlyButton->setChecked(true);
    d->ui->m_periodGroup->setId(d->ui->m_monthlyButton, 0);
    d->ui->m_periodGroup->setId(d->ui->m_yearlyButton, 1);
    d->ui->m_periodGroup->setId(d->ui->m_individualButton, 2);
    slotChangePeriod(d->ui->m_periodGroup->id(d->ui->m_monthlyButton));

    connect(d->ui->m_amountMonthly, &AmountEdit::amountChanged, this, &KBudgetValues::slotNeedUpdate);
    connect(d->ui->m_amountYearly, &AmountEdit::amountChanged, this, &KBudgetValues::slotNeedUpdate);
    d->ui->m_amountMonthly->installEventFilter(this);
    d->ui->m_amountYearly->installEventFilter(this);

    for (auto i = 0; i < 12; ++i) {
        connect(d->m_field[i], &AmountEdit::amountChanged, this, &KBudgetValues::slotNeedUpdate);
        d->m_field[i]->installEventFilter(this);
    }

    connect(d->ui->m_clearButton, &QAbstractButton::clicked, this, &KBudgetValues::slotClearAllValues);
    connect(d->ui->m_periodGroup, &QButtonGroup::idClicked, this, &KBudgetValues::slotChangePeriod);
    connect(this, &KBudgetValues::valuesChanged, this, &KBudgetValues::slotUpdateClearButton);

    KGuiItem clearItem(KStandardGuiItem::clear());

    KGuiItem::assign(d->ui->m_clearButton, clearItem);
    d->ui->m_clearButton->setText(QString());
    d->ui->m_clearButton->setToolTip(clearItem.toolTip());
}

KBudgetValues::~KBudgetValues()
{
    Q_D(KBudgetValues);
    delete d;
}

bool KBudgetValues::eventFilter(QObject* o, QEvent* e)
{
    auto rc = false;

    if (o->isWidgetType()
            && (e->type() == QEvent::KeyPress)) {
        if (auto k = dynamic_cast<QKeyEvent*>(e)) {
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
    }
    return rc;
}

void KBudgetValues::clear()
{
    Q_D(KBudgetValues);
    blockSignals(true);
    for (auto i = 0; i < 12; ++i)
        d->m_field[i]->setValue(MyMoneyMoney());
    d->ui->m_amountMonthly->setValue(MyMoneyMoney());
    d->ui->m_amountYearly->setValue(MyMoneyMoney());
    blockSignals(false);
}

void KBudgetValues::slotClearAllValues()
{
    Q_D(KBudgetValues);
    int tab = d->ui->m_periodGroup->checkedId();
    if (tab == d->ui->m_periodGroup->id(d->ui->m_monthlyButton)) {
        d->ui->m_amountMonthly->setValue(MyMoneyMoney());
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_yearlyButton)) {
        d->ui->m_amountYearly->setValue(MyMoneyMoney());
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_individualButton)) {
        for (auto i = 0; i < 12; ++i)
            d->m_field[i]->setValue(MyMoneyMoney());
    }
    Q_EMIT valuesChanged();
}

void KBudgetValues::slotChangePeriod(int id)
{
    Q_D(KBudgetValues);
    // Prevent a recursive entry of this method due to widget changes
    // performed during execution of this method
    static bool inside = false;
    if (inside)
        return;
    inside = true;

    QWidget *tab = d->ui->m_periodGroup->button(id);
    d->fillMonthLabels(tab);

    MyMoneyMoney newValue;
    if (tab == d->ui->m_monthlyButton) {
        d->ui->m_firstItemStack->setCurrentIndex(d->ui->m_firstItemStack->indexOf(d->ui->m_monthlyPage));
        d->enableMonths(false);
        if (d->ui->m_amountMonthly->value().isZero()) {
            if (d->m_currentTab == d->ui->m_yearlyButton) {
                newValue = (d->ui->m_amountYearly->value() / MyMoneyMoney(12, 1)).convert();

            } else if (d->m_currentTab == d->ui->m_individualButton) {
                for (auto i = 0; i < 12; ++i)
                    newValue += d->m_field[i]->value();
                newValue = (newValue / MyMoneyMoney(12, 1)).convert();
            }
            if (!newValue.isZero()) {
                if (KMessageBox::questionTwoActions(this,
                                                    QString("<qt>")
                                                        + i18n("You have entered budget values using a different base which would result in a monthly budget "
                                                               "of <b>%1</b>. Should this value be used to fill the monthly budget?",
                                                               newValue.formatMoney(QString(), 2))
                                                        + QString("</qt>"),
                                                    i18nc("Auto assignment (caption)", "Auto assignment"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no(),
                                                    "use_previous_budget_values")
                    == KMessageBox::PrimaryAction) {
                    d->ui->m_amountMonthly->setValue(newValue);
                }
            }
        }

    } else if (tab == d->ui->m_yearlyButton) {
        d->ui->m_firstItemStack->setCurrentIndex(d->ui->m_firstItemStack->indexOf(d->ui->m_yearlyPage));
        d->enableMonths(false);
        if (d->ui->m_amountYearly->value().isZero()) {
            if (d->m_currentTab == d->ui->m_monthlyButton) {
                newValue = (d->ui->m_amountMonthly->value() * MyMoneyMoney(12, 1)).convert();

            } else if (d->m_currentTab == d->ui->m_individualButton) {
                for (auto i = 0; i < 12; ++i)
                    newValue += d->m_field[i]->value();
            }
            if (!newValue.isZero()) {
                if (KMessageBox::questionTwoActions(this,
                                                    QString("<qt>")
                                                        + i18n("You have entered budget values using a different base which would result in a yearly budget of "
                                                               "<b>%1</b>. Should this value be used to fill the monthly budget?",
                                                               newValue.formatMoney(QString(), 2))
                                                        + QString("</qt>"),
                                                    i18nc("Auto assignment (caption)", "Auto assignment"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no(),
                                                    "use_previous_budget_values")
                    == KMessageBox::PrimaryAction) {
                    d->ui->m_amountYearly->setValue(newValue);
                }
            }
        }

    } else if (tab == d->ui->m_individualButton) {
        d->ui->m_firstItemStack->setCurrentIndex(d->ui->m_firstItemStack->indexOf(d->ui->m_individualPage));
        d->enableMonths(true);
        for (auto i = 0; i < 12; ++i)
            newValue += d->m_field[i]->value();
        if (newValue.isZero()) {
            if (d->m_currentTab == d->ui->m_monthlyButton) {
                newValue = d->ui->m_amountMonthly->value();
            } else if (d->m_currentTab == d->ui->m_yearlyButton) {
                newValue = (d->ui->m_amountYearly->value() / MyMoneyMoney(12, 1)).convert();
            }

            if (!newValue.isZero()) {
                if (KMessageBox::questionTwoActions(this,
                                                    QString("<qt>")
                                                        + i18n("You have entered budget values using a different base which would result in an individual "
                                                               "monthly budget of <b>%1</b>. Should this value be used to fill the monthly budgets?",
                                                               newValue.formatMoney(QString(), 2))
                                                        + QString("</qt>"),
                                                    i18nc("Auto assignment (caption)", "Auto assignment"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no(),
                                                    "use_previous_budget_values")
                    == KMessageBox::PrimaryAction) {
                    for (auto i = 0; i < 12; ++i)
                        d->m_field[i]->setValue(newValue);
                }
            }
        }
    }

    slotNeedUpdate();
    d->m_currentTab = tab;
    inside = false;
}

void KBudgetValues::slotNeedUpdate()
{
    if (!signalsBlocked())
        QMetaObject::invokeMethod(this, "valuesChanged", Qt::QueuedConnection);
}

void KBudgetValues::setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount)
{
    Q_D(KBudgetValues);
    MyMoneyBudget::PeriodGroup period;
    d->m_budgetDate = budget.budgetStart();
    QDate date;

    // make sure all values are zero so that slotChangePeriod()
    // doesn't check for anything.
    clear();

    blockSignals(true);
    switch (budgetAccount.budgetLevel()) {
    case eMyMoney::Budget::Level::Monthly:
    default:
        d->ui->m_monthlyButton->setChecked(true);
        slotChangePeriod(d->ui->m_periodGroup->id(d->ui->m_monthlyButton));
        d->ui->m_amountMonthly->setValue(budgetAccount.period(d->m_budgetDate).amount());
        break;
    case eMyMoney::Budget::Level::Yearly:
        d->ui->m_yearlyButton->setChecked(true);
        slotChangePeriod(d->ui->m_periodGroup->id(d->ui->m_yearlyButton));
        d->ui->m_amountYearly->setValue(budgetAccount.period(d->m_budgetDate).amount());
        break;
    case eMyMoney::Budget::Level::MonthByMonth:
        d->ui->m_individualButton->setChecked(true);
        slotChangePeriod(d->ui->m_periodGroup->id(d->ui->m_individualButton));
        date.setDate(d->m_budgetDate.year(), d->m_budgetDate.month(), d->m_budgetDate.day());
        for (auto i = 0; i < 12; ++i) {
            d->m_field[i]->setValue(budgetAccount.period(date).amount());
            date = date.addMonths(1);
        }
        break;
    }
    slotUpdateClearButton();
    blockSignals(false);
}

void KBudgetValues::budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount)
{
    Q_D(KBudgetValues);
    MyMoneyBudget::PeriodGroup period;
    d->m_budgetDate = budget.budgetStart();
    period.setStartDate(d->m_budgetDate);
    QDate date;

    budgetAccount.clearPeriods();
    int tab = d->ui->m_periodGroup->checkedId();
    if (tab == d->ui->m_periodGroup->id(d->ui->m_monthlyButton)) {
        budgetAccount.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
        period.setAmount(d->ui->m_amountMonthly->value());
        budgetAccount.addPeriod(d->m_budgetDate, period);
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_yearlyButton)) {
        budgetAccount.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
        period.setAmount(d->ui->m_amountYearly->value());
        budgetAccount.addPeriod(d->m_budgetDate, period);
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_individualButton)) {
        budgetAccount.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
        date.setDate(d->m_budgetDate.year(), d->m_budgetDate.month(), d->m_budgetDate.day());
        for (auto i = 0; i < 12; ++i) {
            period.setStartDate(date);
            period.setAmount(d->m_field[i]->value());
            budgetAccount.addPeriod(date, period);
            date = date.addMonths(1);
        }
    }
}

void KBudgetValues::slotUpdateClearButton()
{
    Q_D(KBudgetValues);
    auto rc = false;
    int tab = d->ui->m_periodGroup->checkedId();
    if (tab == d->ui->m_periodGroup->id(d->ui->m_monthlyButton)) {
        rc = !d->ui->m_amountMonthly->value().isZero();
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_yearlyButton)) {
        rc = !d->ui->m_amountYearly->value().isZero();
    } else if (tab == d->ui->m_periodGroup->id(d->ui->m_individualButton)) {
        for (auto i = 0; (i < 12) && (rc == false); ++i) {
            rc |= !d->m_field[i]->value().isZero();
        }
    }
    d->ui->m_clearButton->setEnabled(rc);
}

void KBudgetValues::setBudgetStartDate(const QDate& date)
{
    Q_D(KBudgetValues);
    d->m_budgetDate = date;
    d->fillMonthLabels(d->m_currentTab);
}

QDate KBudgetValues::budgetStartDate() const
{
    Q_D(const KBudgetValues);
    return d->m_budgetDate;
}
