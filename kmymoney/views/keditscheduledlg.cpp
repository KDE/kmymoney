/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keditscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QDebug>
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
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "newtransactioneditor.h"
#include "selectedtransactions.h"
#include "transaction.h"
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
        , m_requiredFields(nullptr)
        , m_editor(nullptr)
    {
    }

    ~KEditScheduleDlgPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KEditScheduleDlg);
        ui->setupUi(q);

        m_editor = ui->m_editor;
        m_editor->setShowAccountCombo(true);

        m_editor->setShowButtons(false);
        m_editor->setFrameShape(QFrame::NoFrame);
        m_editor->setFrameShadow(QFrame::Plain);
        m_editor->layout()->setMargin(0);

        m_requiredFields = new KMandatoryFieldGroup(q);
        m_requiredFields->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present

        // add the required fields to the mandatory group
        m_requiredFields->add(ui->m_nameEdit);

        auto accountCombo = m_editor->findChild<KMyMoneyAccountCombo*>(QLatin1String("accountCombo"));
        if (accountCombo) {
            m_requiredFields->add(accountCombo);
        }
        const auto account = m_schedule.account();
        if (!account.id().isEmpty()) {
            accountCombo->setSelected(account.id());
        }

        accountCombo = m_editor->findChild<KMyMoneyAccountCombo*>(QLatin1String("categoryCombo"));
        if (accountCombo) {
            m_requiredFields->add(accountCombo);
        }

        m_editor->loadSchedule(m_schedule);

        // setup widget contents
        ui->m_nameEdit->setText(m_schedule.name());

        ui->m_frequencyEdit->setModel(&m_frequencyModel);
        ui->m_frequencyEdit->setCurrentIndex(m_frequencyModel.indexByOccurrence(m_schedule.occurrence()).row());

        if (ui->m_frequencyEdit->currentData(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>() == Schedule::Occurrence::Any)
            ui->m_frequencyEdit->setCurrentIndex(m_frequencyModel.indexByOccurrence(Schedule::Occurrence::Monthly).row());
        // q->slotFrequencyChanged((int)ui->m_frequencyEdit->currentItem());
        ui->m_frequencyNoEdit->setValue(m_schedule.occurrenceMultiplier());

        // load option widgets
        ui->m_paymentMethodEdit->setModel(&m_paymentMethodModel);

        auto method = m_schedule.paymentType();
        if (method == Schedule::PaymentType::Any)
            method = Schedule::PaymentType::Other;
        ui->m_paymentMethodEdit->setCurrentIndex(m_paymentMethodModel.indexByPaymentMethod(method).row());

        switch (m_schedule.weekendOption()) {
        case Schedule::WeekendOption::MoveNothing:
            ui->m_weekendOptionEdit->setCurrentIndex(0);
            break;
        case Schedule::WeekendOption::MoveBefore:
            ui->m_weekendOptionEdit->setCurrentIndex(1);
            break;
        case Schedule::WeekendOption::MoveAfter:
            ui->m_weekendOptionEdit->setCurrentIndex(2);
            break;
        }
        ui->m_estimateEdit->setChecked(!m_schedule.isFixed());
        ui->m_lastDayInMonthEdit->setChecked(m_schedule.lastDayInMonth());
        ui->m_autoEnterEdit->setChecked(m_schedule.autoEnter());
        ui->m_endSeriesEdit->setChecked(m_schedule.willEnd());

        ui->m_endOptionsFrame->setEnabled(m_schedule.willEnd());
        if (m_schedule.willEnd()) {
            ui->m_RemainingEdit->setValue(m_schedule.transactionsRemaining());
            ui->m_FinalPaymentEdit->setDate(m_schedule.endDate());
        }

        q->setModal(true);

        // we just hide the variation field for now and enable the logic
        // once we have a respective member in the MyMoneySchedule object
        ui->m_variation->hide();
    }

    /**
     * Helper method to recalculate and update Transactions Remaining
     * when other values are changed
     */
    void updateTransactionsRemaining()
    {
        auto remain = m_schedule.transactionsRemaining();
        if (remain != ui->m_RemainingEdit->value()) {
            QSignalBlocker blocked(ui->m_RemainingEdit);
            ui->m_RemainingEdit->setValue(remain);
        }
    }

    MyMoneyTransaction transaction() const
    {
        auto t = m_schedule.transaction();

        if (m_editor) {
            t = m_editor->transaction();
        }

        t.clearId();
        t.setEntryDate(QDate());
        return t;
    }

    void setScheduleOccurrencePeriod()
    {
        const auto row = ui->m_frequencyEdit->currentIndex();
        const auto idx = ui->m_frequencyEdit->model()->index(row, 0);
        const auto occurrence = idx.data(eMyMoney::Model::ScheduleFrequencyRole).value<Schedule::Occurrence>();
        m_schedule.setOccurrencePeriod(occurrence);
    }

    KEditScheduleDlg* q_ptr;
    Ui::KEditScheduleDlg* ui;
    KMandatoryFieldGroup* m_requiredFields;
    NewTransactionEditor* m_editor;
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

    connect(d->ui->m_RemainingEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int value) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        d->m_schedule.setNextDueDate(d->m_editor->postDate());
        d->setScheduleOccurrencePeriod();
        d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

        if (d->m_schedule.transactionsRemaining() != value) {
            QSignalBlocker blocked(d->ui->m_FinalPaymentEdit);
            d->ui->m_FinalPaymentEdit->setDate(d->m_schedule.dateAfter(value));
        }
    });

    connect(d->ui->m_FinalPaymentEdit, &KMyMoneyDateInput::dateChanged, this, [&](const QDate& date) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        d->m_schedule.setNextDueDate(d->m_editor->postDate());
        d->setScheduleOccurrencePeriod();
        d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

        if (d->m_schedule.endDate() != date) {
            d->m_schedule.setEndDate(date);
            d->updateTransactionsRemaining();
        }
    });

    connect(d->ui->m_frequencyEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KEditScheduleDlg);
        const auto model = d->ui->m_frequencyEdit->model();
        const auto paymentType = model->index(idx, 0).data(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>();

        d->ui->m_endSeriesEdit->setEnabled(paymentType != Schedule::Occurrence::Once);
        bool isEndSeries = d->ui->m_endSeriesEdit->isChecked();
        if (isEndSeries)
            d->ui->m_endOptionsFrame->setEnabled(paymentType != Schedule::Occurrence::Once);
        switch (paymentType) {
        case Schedule::Occurrence::Daily:
        case Schedule::Occurrence::Weekly:
            d->ui->m_frequencyNoEdit->setEnabled(true);
            d->ui->m_lastDayInMonthEdit->setEnabled(false);
            break;

        case Schedule::Occurrence::EveryHalfMonth:
        case Schedule::Occurrence::Monthly:
        case Schedule::Occurrence::Yearly:
            // Supports Frequency Number
            d->ui->m_frequencyNoEdit->setEnabled(true);
            d->ui->m_lastDayInMonthEdit->setEnabled(true);
            break;

        default:
            // Multiplier is always 1
            d->ui->m_frequencyNoEdit->setEnabled(false);
            d->ui->m_frequencyNoEdit->setValue(1);
            d->ui->m_lastDayInMonthEdit->setEnabled(true);
            break;
        }
        if (isEndSeries && (paymentType != Schedule::Occurrence::Once)) {
            // Changing the frequency changes the number
            // of remaining transactions
            d->m_schedule.setNextDueDate(d->m_editor->postDate());
            d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());
            d->m_schedule.setOccurrencePeriod(paymentType);
            d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
            d->updateTransactionsRemaining();
        }
    });

    connect(d->ui->m_frequencyNoEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int multiplier) {
        Q_D(KEditScheduleDlg);
        // Make sure the required fields are set
        auto oldOccurrenceMultiplier = d->m_schedule.occurrenceMultiplier();
        if (multiplier != oldOccurrenceMultiplier) {
            if (d->ui->m_endOptionsFrame->isEnabled()) {
                d->m_schedule.setNextDueDate(d->m_editor->postDate());
                d->m_schedule.setOccurrenceMultiplier(multiplier);
                d->setScheduleOccurrencePeriod();
                d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
                d->updateTransactionsRemaining();
            }
        }
    });

    connect(d->ui->m_paymentMethodEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KEditScheduleDlg);
        const auto model = d->ui->m_paymentMethodEdit->model();
        const auto paymentType = model->index(idx, 0).data(eMyMoney::Model::SchedulePaymentTypeRole).value<eMyMoney::Schedule::PaymentType>();
        const bool isWriteCheck = paymentType == Schedule::PaymentType::WriteChecque;
        d->m_editor->setShowNumberWidget(isWriteCheck);
    });

    connect(d->m_editor, &NewTransactionEditor::postDateChanged, this, [&](const QDate& date) {
        Q_D(KEditScheduleDlg);
        if (d->m_schedule.nextDueDate() != date) {
            if (d->ui->m_endOptionsFrame->isEnabled()) {
                d->m_schedule.setNextDueDate(date);
                d->m_schedule.setStartDate(date);
                d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

                d->setScheduleOccurrencePeriod();

                d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
                d->updateTransactionsRemaining();
            }
        }
    });

    connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, [&]() {
        KHelpClient::invokeHelp("details.schedules.intro");
    });
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
    d->m_schedule.setName(d->ui->m_nameEdit->text());
    d->m_schedule.setFixed(!d->ui->m_estimateEdit->isChecked());

    auto model = d->ui->m_frequencyEdit->model();
    const auto frequency =
        model->index(d->ui->m_frequencyEdit->currentIndex(), 0).data(eMyMoney::Model::ScheduleFrequencyRole).value<eMyMoney::Schedule::Occurrence>();
    d->m_schedule.setOccurrencePeriod(frequency);
    d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

    switch (d->ui->m_weekendOptionEdit->currentIndex()) {
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

    const auto amount = d->m_editor->transactionAmount();
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

    if (d->ui->m_lastDayInMonthEdit->isEnabled())
        d->m_schedule.setLastDayInMonth(d->ui->m_lastDayInMonthEdit->isChecked());
    else
        d->m_schedule.setLastDayInMonth(false);
    d->m_schedule.setAutoEnter(d->ui->m_autoEnterEdit->isChecked());

    model = d->ui->m_paymentMethodEdit->model();
    const auto paymentType =
        model->index(d->ui->m_paymentMethodEdit->currentIndex(), 0).data(eMyMoney::Model::SchedulePaymentTypeRole).value<eMyMoney::Schedule::PaymentType>();
    d->m_schedule.setPaymentType(paymentType);
    if (d->ui->m_endSeriesEdit->isEnabled() && d->ui->m_endSeriesEdit->isChecked()) {
        d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
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
