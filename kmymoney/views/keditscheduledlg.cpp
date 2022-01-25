/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keditscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "comboboxmodels.h"
#include "delegateproxy.h"
#include "journaldelegate.h"
#include "keditloanwizard.h"
#include "kguiutils.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneydateinput.h"
#include "kmymoneylineedit.h"
#include "kmymoneymvccombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "knewaccountdlg.h"
#include "knewinvestmentwizard.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "newtransactioneditor.h"
#include "ui_keditscheduledlg.h"
#include "widgetenums.h"

using namespace eMyMoney;

class KEditScheduleDlgPrivate
{
    Q_DISABLE_COPY(KEditScheduleDlgPrivate)
    Q_DECLARE_PUBLIC(KEditScheduleDlg)

public:
    explicit KEditScheduleDlgPrivate(KEditScheduleDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KEditScheduleDlg)
        , tabOrderUi(nullptr)
        , m_requiredFields(nullptr)
        , transactionEditor(nullptr)
    {
    }

    ~KEditScheduleDlgPrivate()
    {
        delete ui;
        delete tabOrderUi;
    }

    void init()
    {
        Q_Q(KEditScheduleDlg);
        ui->setupUi(q);

        transactionEditor = ui->transactionEditor;
        transactionEditor->setShowAccountCombo(true);

        transactionEditor->setShowButtons(false);
        transactionEditor->layout()->setMargin(0);

        m_requiredFields = new KMandatoryFieldGroup(q);
        m_requiredFields->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present

        // add the required fields to the mandatory group
        m_requiredFields->add(ui->nameEdit);
    }

    void loadWidgets()
    {
        Q_Q(KEditScheduleDlg);
        auto accountCombo = transactionEditor->findChild<KMyMoneyAccountCombo*>(QLatin1String("accountCombo"));
        if (accountCombo) {
            m_requiredFields->add(accountCombo);
        }
        const auto account = m_schedule.account();
        if (!account.id().isEmpty()) {
            accountCombo->setSelected(account.id());
        }

        accountCombo = transactionEditor->findChild<KMyMoneyAccountCombo*>(QLatin1String("categoryCombo"));
        if (accountCombo) {
            m_requiredFields->add(accountCombo);
        }

        transactionEditor->loadSchedule(m_schedule);

        // setup widget contents
        ui->nameEdit->setText(m_schedule.name());

        ui->frequencyEdit->setModel(&m_frequencyModel);
        ui->frequencyEdit->setCurrentIndex(m_frequencyModel.indexByOccurrence(m_schedule.occurrence()).row());

        if (ui->frequencyEdit->currentData(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>() == Schedule::Occurrence::Any)
            ui->frequencyEdit->setCurrentIndex(m_frequencyModel.indexByOccurrence(Schedule::Occurrence::Monthly).row());
        // q->slotFrequencyChanged((int)ui->m_frequencyEdit->currentItem());
        ui->frequencyNoEdit->setValue(m_schedule.occurrenceMultiplier());

        // load option widgets
        ui->paymentMethodCombo->setModel(&m_paymentMethodModel);

        auto method = m_schedule.paymentType();
        if (method == Schedule::PaymentType::Any)
            method = Schedule::PaymentType::Other;
        ui->paymentMethodCombo->setCurrentIndex(m_paymentMethodModel.indexByPaymentMethod(method).row());

        switch (m_schedule.weekendOption()) {
        case Schedule::WeekendOption::MoveNothing:
            ui->weekendOptionCombo->setCurrentIndex(0);
            break;
        case Schedule::WeekendOption::MoveBefore:
            ui->weekendOptionCombo->setCurrentIndex(1);
            break;
        case Schedule::WeekendOption::MoveAfter:
            ui->weekendOptionCombo->setCurrentIndex(2);
            break;
        }
        ui->estimateOption->setChecked(!m_schedule.isFixed());
        ui->lastDayInMonthOption->setChecked(m_schedule.lastDayInMonth());
        ui->autoEnterOption->setChecked(m_schedule.autoEnter());
        ui->endSeriesOption->setChecked(m_schedule.willEnd());

        ui->endOptionsFrame->setEnabled(m_schedule.willEnd());
        if (m_schedule.willEnd()) {
            ui->remainingEdit->setValue(m_schedule.transactionsRemaining());
            ui->finalPaymentDateEdit->setDate(m_schedule.endDate());
        }

        q->setModal(true);

        // we just hide the variation field for now and enable the logic
        // once we have a respective member in the MyMoneySchedule object
        ui->variationEdit->hide();
    }

    void setupTabOrder()
    {
        Q_Q(KEditScheduleDlg);

        const auto defaultTabOrder = QStringList{
            QLatin1String("nameEdit"),
            QLatin1String("frequencyNoEdit"),
            QLatin1String("frequencyEdit"),
            QLatin1String("paymentMethodCombo"),
            // the std transaction editor (see also newtransactioneditor.cpp
            QLatin1String("accountCombo"),
            QLatin1String("dateEdit"),
            QLatin1String("creditDebitEdit"),
            QLatin1String("payeeEdit"),
            QLatin1String("numberEdit"),
            QLatin1String("categoryCombo"),
            QLatin1String("costCenterCombo"),
            QLatin1String("tagContainer"),
            QLatin1String("statusCombo"),
            QLatin1String("memoEdit"),
            // the schedule options
            QLatin1String("weekendOptionCombo"),
            QLatin1String("estimateOption"),
            QLatin1String("variationEdit"),
            QLatin1String("lastDayInMonthOption"),
            QLatin1String("autoEnterOption"),
            QLatin1String("endSeriesOption"),
            QLatin1String("remainingEdit"),
            QLatin1String("finalPaymentDateEdit"),
            QLatin1String("buttonBox"),
        };
        q->setProperty("kmm_defaulttaborder", defaultTabOrder);
        q->setProperty("kmm_currenttaborder", KMyMoneyUtils::tabOrder(QLatin1String("scheduleTransactionEditor"), defaultTabOrder));
        // let the transaction editor part use the same tab order information
        transactionEditor->setProperty("kmm_currenttaborder", KMyMoneyUtils::tabOrder(QLatin1String("scheduleTransactionEditor"), defaultTabOrder));

        KMyMoneyUtils::setupTabOrder(q, q->property("kmm_currenttaborder").toStringList());
    }

    /**
     * Helper method to recalculate and update Transactions Remaining
     * when other values are changed
     */
    void updateTransactionsRemaining()
    {
        auto remain = m_schedule.transactionsRemaining();
        if (remain != ui->remainingEdit->value()) {
            QSignalBlocker blocked(ui->remainingEdit);
            ui->remainingEdit->setValue(remain);
        }
    }

    MyMoneyTransaction transaction() const
    {
        auto t = m_schedule.transaction();

        if (transactionEditor) {
            t = transactionEditor->transaction();
        }

        t.clearId();
        t.setEntryDate(QDate());
        return t;
    }

    void setScheduleOccurrencePeriod()
    {
        const auto row = ui->frequencyEdit->currentIndex();
        const auto idx = ui->frequencyEdit->model()->index(row, 0);
        const auto occurrence = idx.data(eMyMoney::Model::ScheduleFrequencyRole).value<Schedule::Occurrence>();
        m_schedule.setOccurrencePeriod(occurrence);
    }

    KEditScheduleDlg* q_ptr;
    Ui::KEditScheduleDlg* ui;
    Ui::KEditScheduleDlg* tabOrderUi;
    KMandatoryFieldGroup* m_requiredFields;
    NewTransactionEditor* transactionEditor;
    MyMoneySchedule m_schedule;
    QWidgetList m_tabOrderWidgets;
    OccurrencesModel m_frequencyModel;
    PaymentMethodModel m_paymentMethodModel;
};

KEditScheduleDlg::KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KEditScheduleDlgPrivate(this))
{
    Q_D(KEditScheduleDlg);
    d->m_schedule = schedule;
    d->init();
    setSizeGripEnabled(true);

    connect(d->ui->remainingEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int value) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        d->m_schedule.setNextDueDate(d->transactionEditor->postDate());
        d->setScheduleOccurrencePeriod();
        d->m_schedule.setOccurrenceMultiplier(d->ui->frequencyNoEdit->value());

        if (d->m_schedule.transactionsRemaining() != value) {
            QSignalBlocker blocked(d->ui->finalPaymentDateEdit);
            d->ui->finalPaymentDateEdit->setDate(d->m_schedule.dateAfter(value));
        }
    });

    connect(d->ui->finalPaymentDateEdit, &KMyMoneyDateInput::dateChanged, this, [&](const QDate& date) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        d->m_schedule.setNextDueDate(d->transactionEditor->postDate());
        d->setScheduleOccurrencePeriod();
        d->m_schedule.setOccurrenceMultiplier(d->ui->frequencyNoEdit->value());

        if (d->m_schedule.endDate() != date) {
            d->m_schedule.setEndDate(date);
            d->updateTransactionsRemaining();
        }
    });

    connect(d->ui->frequencyEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KEditScheduleDlg);
        const auto model = d->ui->frequencyEdit->model();
        const auto paymentType = model->index(idx, 0).data(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>();

        d->ui->endSeriesOption->setEnabled(paymentType != Schedule::Occurrence::Once);
        bool isEndSeries = d->ui->endSeriesOption->isChecked();
        if (isEndSeries)
            d->ui->endOptionsFrame->setEnabled(paymentType != Schedule::Occurrence::Once);
        switch (paymentType) {
        case Schedule::Occurrence::Daily:
        case Schedule::Occurrence::Weekly:
            d->ui->frequencyNoEdit->setEnabled(true);
            d->ui->lastDayInMonthOption->setEnabled(false);
            break;

        case Schedule::Occurrence::EveryHalfMonth:
        case Schedule::Occurrence::Monthly:
        case Schedule::Occurrence::Yearly:
            // Supports Frequency Number
            d->ui->frequencyNoEdit->setEnabled(true);
            d->ui->lastDayInMonthOption->setEnabled(true);
            break;

        default:
            // Multiplier is always 1
            d->ui->frequencyNoEdit->setEnabled(false);
            d->ui->frequencyNoEdit->setValue(1);
            d->ui->lastDayInMonthOption->setEnabled(true);
            break;
        }
        if (isEndSeries && (paymentType != Schedule::Occurrence::Once)) {
            // Changing the frequency changes the number
            // of remaining transactions
            d->m_schedule.setNextDueDate(d->transactionEditor->postDate());
            d->m_schedule.setOccurrenceMultiplier(d->ui->frequencyNoEdit->value());
            d->m_schedule.setOccurrencePeriod(paymentType);
            d->m_schedule.setEndDate(d->ui->finalPaymentDateEdit->date());
            d->updateTransactionsRemaining();
        }
    });

    connect(d->ui->frequencyNoEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int multiplier) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        auto oldOccurrenceMultiplier = d->m_schedule.occurrenceMultiplier();
        if (multiplier != oldOccurrenceMultiplier) {
            if (d->ui->endOptionsFrame->isEnabled()) {
                d->m_schedule.setNextDueDate(d->transactionEditor->postDate());
                d->m_schedule.setOccurrenceMultiplier(multiplier);
                d->setScheduleOccurrencePeriod();
                d->m_schedule.setEndDate(d->ui->finalPaymentDateEdit->date());
                d->updateTransactionsRemaining();
            }
        }
    });

    connect(d->ui->paymentMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KEditScheduleDlg);
        const auto model = d->ui->paymentMethodCombo->model();
        const auto paymentType = model->index(idx, 0).data(eMyMoney::Model::SchedulePaymentTypeRole).value<eMyMoney::Schedule::PaymentType>();
        const bool isWriteCheck = paymentType == Schedule::PaymentType::WriteChecque;
        d->transactionEditor->setShowNumberWidget(isWriteCheck);
    });

    connect(d->transactionEditor, &NewTransactionEditor::postDateChanged, this, [&](const QDate& date) {
        Q_D(KEditScheduleDlg);
        if (d->m_schedule.nextDueDate() != date) {
            if (d->ui->endOptionsFrame->isEnabled()) {
                d->m_schedule.setNextDueDate(date);
                d->m_schedule.setStartDate(date);
                d->m_schedule.setOccurrenceMultiplier(d->ui->frequencyNoEdit->value());

                d->setScheduleOccurrencePeriod();

                d->m_schedule.setEndDate(d->ui->finalPaymentDateEdit->date());
                d->updateTransactionsRemaining();
            }
        }
    });

    connect(d->ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, [&]() {
        KHelpClient::invokeHelp("details.schedules.intro");
    });

    d->loadWidgets();

    const auto dateEdit = d->transactionEditor->findChild<QWidget*>("dateEdit");
    if (dateEdit) {
        connect(d->ui->lastDayInMonthOption, &QCheckBox::stateChanged, dateEdit, &QWidget::setDisabled);
        dateEdit->setDisabled(d->ui->lastDayInMonthOption->isChecked());
    }

    // we delay setting up the tab order until we drop back to the event loop.
    // this will make sure that all widgets are visible and the tab order logic
    // works properly
    QMetaObject::invokeMethod(
        this,
        [&]() {
            Q_D(KEditScheduleDlg);
            d->setupTabOrder();
            // set focus to first tab field once we return to event loop
            const auto tabOrder = property("kmm_currenttaborder").toStringList();
            if (!tabOrder.isEmpty()) {
                const auto focusWidget = findChild<QWidget*>(tabOrder.first());
                if (focusWidget) {
                    QMetaObject::invokeMethod(focusWidget, "setFocus", Qt::QueuedConnection);
                }
            }
        },
        Qt::QueuedConnection);
}

KEditScheduleDlg::~KEditScheduleDlg()
{
    Q_D(KEditScheduleDlg);
    delete d;
}

void KEditScheduleDlg::accept()
{
    Q_D(KEditScheduleDlg);
    // Force the focus to be on the OK button. This will trigger creation
    // of any unknown objects (payees, categories etc.)
    d->ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    // only accept if the button is really still enabled. We could end
    // up here, if the user filled all fields, the focus is on the category
    // field, but the category is not yet existent. When the user presses the
    // OK button in this context, he will be asked if he wants to create
    // the category or not. In case he decides no, we end up here with no
    // category filled in, so we don't run through the final acceptance.
    if (d->ui->buttonBox->button(QDialogButtonBox::Ok)->isEnabled())
        QDialog::accept();
}

const MyMoneySchedule& KEditScheduleDlg::schedule()
{
    Q_D(KEditScheduleDlg);
    auto t = d->transaction();
    if (d->m_schedule.nextDueDate() != t.postDate()) {
        d->m_schedule.setNextDueDate(t.postDate());
        d->m_schedule.setStartDate(t.postDate());
    }
    d->m_schedule.setTransaction(t);
    d->m_schedule.setName(d->ui->nameEdit->text());
    d->m_schedule.setFixed(!d->ui->estimateOption->isChecked());

    auto model = d->ui->frequencyEdit->model();
    const auto frequency =
        model->index(d->ui->frequencyEdit->currentIndex(), 0).data(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>();
    d->m_schedule.setOccurrencePeriod(frequency);
    d->m_schedule.setOccurrenceMultiplier(d->ui->frequencyNoEdit->value());

    switch (d->ui->weekendOptionCombo->currentIndex()) {
    case 0:
        d->m_schedule.setWeekendOption(Schedule::WeekendOption::MoveNothing);
        break;
    case 1:
        d->m_schedule.setWeekendOption(Schedule::WeekendOption::MoveBefore);
        break;
    case 2:
        d->m_schedule.setWeekendOption(Schedule::WeekendOption::MoveAfter);
        break;
    }

    d->m_schedule.setType(Schedule::Type::Bill);

    const auto amount = d->transactionEditor->transactionAmount();
    if (d->m_schedule.transaction().splitCount() == 2) {
        const auto splits = d->m_schedule.transaction().splits();
        bool isTransfer = true;
        for (const auto& split : splits) {
            const auto idx = MyMoneyFile::instance()->accountsModel()->indexById(split.accountId());
            if (idx.data(eMyMoney::Model::AccountIsAssetLiabilityRole).toBool() == false) {
                isTransfer = false;
                break;
            }
        }
        if (isTransfer) {
            d->m_schedule.setType(Schedule::Type::Transfer);
        }
    }

    if (d->m_schedule.type() != Schedule::Type::Transfer) {
        if (!amount.isNegative()) {
            d->m_schedule.setType(Schedule::Type::Deposit);
        }
    }

    if (d->ui->lastDayInMonthOption->isEnabled())
        d->m_schedule.setLastDayInMonth(d->ui->lastDayInMonthOption->isChecked());
    else
        d->m_schedule.setLastDayInMonth(false);
    d->m_schedule.setAutoEnter(d->ui->autoEnterOption->isChecked());

    model = d->ui->paymentMethodCombo->model();
    const auto paymentType =
        model->index(d->ui->paymentMethodCombo->currentIndex(), 0).data(eMyMoney::Model::SchedulePaymentTypeRole).value<eMyMoney::Schedule::PaymentType>();
    d->m_schedule.setPaymentType(paymentType);
    if (d->ui->endSeriesOption->isEnabled() && d->ui->endSeriesOption->isChecked()) {
        d->m_schedule.setEndDate(d->ui->finalPaymentDateEdit->date());
    } else {
        d->m_schedule.setEndDate(QDate());
    }
    return d->m_schedule;
}

void KEditScheduleDlg::newSchedule(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence)
{
    MyMoneySchedule schedule;
    schedule.setOccurrence(occurrence);

    // if the schedule is based on an existing transaction,
    // we take the post date and project it to the next
    // schedule in a month.
    if (_t != MyMoneyTransaction()) {
        MyMoneyTransaction t(_t);
        schedule.setTransaction(t);
        if (occurrence != eMyMoney::Schedule::Occurrence::Once)
            schedule.setNextDueDate(schedule.nextPayment(t.postDate()));
    }

    bool committed;
    do {
        committed = true;
        QPointer<KEditScheduleDlg> dlg = new KEditScheduleDlg(schedule, nullptr);
        // QPointer<TransactionEditor> transactionEditor = dlg->startEdit();
        KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
        if (dlg->exec() == QDialog::Accepted && dlg != 0) {
            MyMoneyFileTransaction ft;
            try {
                schedule = dlg->schedule();
                MyMoneyFile::instance()->addSchedule(schedule);
                ft.commit();

            } catch (const MyMoneyException& e) {
                KMessageBox::error(nullptr, i18n("Unable to add scheduled transaction: %1", QString::fromLatin1(e.what())), i18n("Add scheduled transaction"));
                committed = false;
            }
        }
        // delete transactionEditor;
        delete dlg;
    } while (!committed);
}

void KEditScheduleDlg::editSchedule(const MyMoneySchedule& inputSchedule)
{
    try {
        auto schedule = MyMoneyFile::instance()->schedule(inputSchedule.id());

        switch (schedule.type()) {
        case eMyMoney::Schedule::Type::Bill:
        case eMyMoney::Schedule::Type::Deposit:
        case eMyMoney::Schedule::Type::Transfer: {
            QScopedPointer<KEditScheduleDlg> dlg(new KEditScheduleDlg(schedule, nullptr));
            KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg.data(), !KMyMoneySettings::stringMatchFromStart());
            if (dlg->exec() == QDialog::Accepted && dlg != 0) {
                MyMoneyFileTransaction ft;
                try {
                    MyMoneySchedule sched = dlg->schedule();
                    // Check whether the new Schedule Date
                    // is at or before the lastPaymentDate
                    // If it is, ask the user whether to clear the
                    // lastPaymentDate
                    const auto& next = sched.nextDueDate();
                    const auto& last = sched.lastPayment();
                    if (next.isValid() && last.isValid() && next <= last) {
                        // Entered a date effectively no later
                        // than previous payment.  Date would be
                        // updated automatically so we probably
                        // want to clear it.  Let's ask the user.
                        QString questionText = i18n(
                            "<qt>You have entered a scheduled transaction date of <b>%1</b>.  Because the scheduled transaction was last paid on <b>%2</b>, "
                            "KMyMoney will automatically adjust the scheduled transaction date to the next date unless the last payment date is reset.  Do you "
                            "want to reset the last payment date?</qt>",
                            QLocale().toString(next, QLocale::ShortFormat),
                            QLocale().toString(last, QLocale::ShortFormat));
                        if (KMessageBox::questionYesNo(nullptr, questionText, i18n("Reset Last Payment Date"), KStandardGuiItem::yes(), KStandardGuiItem::no())
                            == KMessageBox::Yes) {
                            sched.setLastPayment(QDate());
                        }
                    }
                    MyMoneyFile::instance()->modifySchedule(sched);
                    ft.commit();
                } catch (const MyMoneyException& e) {
                    KMessageBox::detailedSorry(nullptr,
                                               i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()),
                                               QString::fromLatin1(e.what()));
                }
            }
            break;
        }
        case eMyMoney::Schedule::Type::LoanPayment: {
            QScopedPointer<KEditLoanWizard> dlg(new KEditLoanWizard(schedule.account(2)));
            if (dlg->exec() == QDialog::Accepted) {
                MyMoneyFileTransaction ft;
                try {
                    MyMoneyFile::instance()->modifySchedule(dlg->schedule());
                    MyMoneyFile::instance()->modifyAccount(dlg->account());
                    ft.commit();
                } catch (const MyMoneyException& e) {
                    KMessageBox::detailedSorry(nullptr,
                                               i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()),
                                               QString::fromLatin1(e.what()));
                }
            }
            break;
        }
        case eMyMoney::Schedule::Type::Any:
            break;
        }
    } catch (const MyMoneyException& e) {
        KMessageBox::detailedSorry(nullptr, i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()), QString::fromLatin1(e.what()));
    }
}

void KEditScheduleDlg::keyPressEvent(QKeyEvent* event)
{
    const auto keySeq = QKeySequence(event->modifiers() + event->key());

    if (keySeq.matches(pActions[eMenu::Action::EditTabOrder]->shortcut())) {
        QPointer<TabOrderDialog> tabOrderDialog = new TabOrderDialog(this);
        auto tabOrderWidget = static_cast<TabOrderEditorInterface*>(qt_metacast("TabOrderEditorInterface"));
        if (tabOrderWidget) {
            tabOrderDialog->setTarget(tabOrderWidget);

            // the account combo is created invisible by the TransactionEditor ctor. Since we
            // need it in the schedule dialog editor, we make sure to see it before we start
            // to setup the tab order and modify it. At the same time, we need to hide the
            // additional buttons provided by the TransactionEditor.
            auto w = tabOrderDialog->findChild<QWidget*>("accountCombo");
            if (w) {
                w->setVisible(true);
            }
            const QStringList buttonNames{QLatin1String("enterButton"), QLatin1String("cancelButton")};
            for (const auto& widgetName : buttonNames) {
                auto w = tabOrderDialog->findChild<QWidget*>(widgetName);
                if (w) {
                    w->setVisible(false);
                }
            }

            auto tabOrder = property("kmm_defaulttaborder").toStringList();
            tabOrderDialog->setDefaultTabOrder(tabOrder);
            tabOrder = property("kmm_currenttaborder").toStringList();
            tabOrderDialog->setTabOrder(tabOrder);

            if ((tabOrderDialog->exec() == QDialog::Accepted) && tabOrderDialog) {
                Q_D(KEditScheduleDlg);
                tabOrderWidget->storeTabOrder(tabOrderDialog->tabOrder());
                d->setupTabOrder();
            }
        }
        tabOrderDialog->deleteLater();
    }
}

void KEditScheduleDlg::setupUi(QWidget* parent)
{
    Q_D(KEditScheduleDlg);
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::KEditScheduleDlg;
    }
    d->tabOrderUi->setupUi(parent);
}

void KEditScheduleDlg::storeTabOrder(const QStringList& tabOrder)
{
    KMyMoneyUtils::storeTabOrder(QLatin1String("scheduleTransactionEditor"), tabOrder);
}

bool KEditScheduleDlg::focusNextPrevChild(bool next)
{
    auto rc = KMyMoneyUtils::tabFocusHelper(this, next);

    if (rc == false) {
        // In case we're going backward from the buttonbox at the bottom,
        // the default focusNextPrevChild() implementation sets the focus
        // to the account combo box instead of the last option. We catch
        // this here and set the focus accordingly.
        const auto pushButton = focusWidget();
        const auto dialogButtonBox = focusWidget()->parentWidget();
        if (pushButton->qt_metacast("QPushButton") && dialogButtonBox->qt_metacast("QDialogButtonBox") && (!next)) {
            if (dialogButtonBox->nextInFocusChain() == pushButton) {
                const auto widgetName = focusWidget()->parentWidget()->objectName();
                const QStringList tabOrder(property("kmm_currenttaborder").toStringList());
                auto idx = tabOrder.indexOf(widgetName);
                if (idx >= 0) {
                    QWidget* w = nullptr;
                    auto previousTabWidget = [&]() {
                        --idx;
                        if (idx < 0) {
                            idx = tabOrder.count() - 1;
                        }
                        w = findChild<QWidget*>(tabOrder.at(idx));
                    };
                    previousTabWidget();
                    while (w && !w->isEnabled()) {
                        previousTabWidget();
                    }
                    if (w) {
                        w->setFocus();
                        return true;
                    }
                }
            }
        }
        rc = QWidget::focusNextPrevChild(next);
    }
    return rc;
}
