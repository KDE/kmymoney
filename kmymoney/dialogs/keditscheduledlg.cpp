/***************************************************************************
                          keditscheduledlg.cpp  -  description
                             -------------------
    begin                : Mon Sep  3 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "keditscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KStandardGuiItem>
#include <KLineEdit>
#include <KHelpClient>
#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_keditscheduledlg.h"

#include "tabbar.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "register.h"
#include "transactionform.h"
#include "transaction.h"
#include "selectedtransactions.h"
#include "transactioneditor.h"
#include "kmymoneylineedit.h"
#include "kmymoneydateinput.h"
#include "kmymoneymvccombo.h"
#include "kguiutils.h"
#include "kmymoneyutils.h"
#include "knewaccountdlg.h"
#include "knewinvestmentwizard.h"
#include "keditloanwizard.h"
#include "kmymoneysettings.h"
#include "mymoneyenums.h"
#include "widgetenums.h"

using namespace eMyMoney;

class KEditScheduleDlgPrivate
{
  Q_DISABLE_COPY(KEditScheduleDlgPrivate)
  Q_DECLARE_PUBLIC(KEditScheduleDlg)

public:
  explicit KEditScheduleDlgPrivate(KEditScheduleDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KEditScheduleDlg)
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

    m_requiredFields = new KMandatoryFieldGroup(q);
    m_requiredFields->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present

    // make sure, we have a tabbar with the form
    // insert it after the horizontal line
    ui->m_paymentInformationLayout->insertWidget(2, ui->m_form->getTabBar(ui->m_form->parentWidget()));

    // we never need to see the register
    ui->m_register->hide();

    // ... setup the form ...
    ui->m_form->setupForm(m_schedule.account());

    // ... and the register ...
    ui->m_register->clear();

    // ... now add the transaction to register and form ...
    auto t = transaction();
    if (m_schedule.transaction().splits().isEmpty())
      m_item = KMyMoneyRegister::Register::transactionFactory(ui->m_register, t, MyMoneySplit(), 0);
    else
      m_item = KMyMoneyRegister::Register::transactionFactory(ui->m_register, t,
                  m_schedule.transaction().splits().isEmpty() ? MyMoneySplit() : m_schedule.transaction().splits().front(), 0);
    ui->m_register->selectItem(m_item);
    // show the account row
    m_item->setShowRowInForm(0, true);

    ui->m_form->slotSetTransaction(m_item);

    // setup widget contents
    ui->m_nameEdit->setText(m_schedule.name());

    ui->m_frequencyEdit->setCurrentItem((int)m_schedule.occurrencePeriod());
    if (ui->m_frequencyEdit->currentItem() == Schedule::Occurrence::Any)
      ui->m_frequencyEdit->setCurrentItem((int)Schedule::Occurrence::Monthly);
    q->slotFrequencyChanged((int)ui->m_frequencyEdit->currentItem());
    ui->m_frequencyNoEdit->setValue(m_schedule.occurrenceMultiplier());

    // load option widgets
    ui->m_paymentMethodEdit->insertItem(i18n("Direct deposit"), (int)Schedule::PaymentType::DirectDeposit);
    ui->m_paymentMethodEdit->insertItem(i18n("Manual deposit"), (int)Schedule::PaymentType::ManualDeposit);
    ui->m_paymentMethodEdit->insertItem(i18n("Direct debit"), (int)Schedule::PaymentType::DirectDebit);
    ui->m_paymentMethodEdit->insertItem(i18n("Standing order"), (int)Schedule::PaymentType::StandingOrder);
    ui->m_paymentMethodEdit->insertItem(i18n("Bank transfer"), (int)Schedule::PaymentType::BankTransfer);
    ui->m_paymentMethodEdit->insertItem(i18n("Write check"), (int)Schedule::PaymentType::WriteChecque);
    ui->m_paymentMethodEdit->insertItem(i18nc("Other payment method", "Other"), (int)Schedule::PaymentType::Other);

    auto method = m_schedule.paymentType();
    if (method == Schedule::PaymentType::Any)
      method = Schedule::PaymentType::Other;
    ui->m_paymentMethodEdit->setCurrentItem((int)method);

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

    q->connect(ui->m_RemainingEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            q, &KEditScheduleDlg::slotRemainingChanged);
    q->connect(ui->m_FinalPaymentEdit, &KMyMoneyDateInput::dateChanged,
            q, &KEditScheduleDlg::slotEndDateChanged);
    q->connect(ui->m_frequencyEdit, &KMyMoneyGeneralCombo::itemSelected,
            q, &KEditScheduleDlg::slotFrequencyChanged);
    q->connect(ui->m_frequencyNoEdit, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            q, &KEditScheduleDlg::slotOccurrenceMultiplierChanged);
    q->connect(ui->buttonBox, &QDialogButtonBox::helpRequested, q, &KEditScheduleDlg::slotShowHelp);

    q->setModal(true);
    // force the initial height to be as small as possible
    QTimer::singleShot(0, q, SLOT(slotSetupSize()));

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
      ui->m_RemainingEdit->blockSignals(true);
      ui->m_RemainingEdit->setValue(remain);
      ui->m_RemainingEdit->blockSignals(false);
    }
  }

  MyMoneyTransaction transaction() const
  {
    auto t = m_schedule.transaction();

    if (m_editor) {
      m_editor->createTransaction(t, m_schedule.transaction(), m_schedule.transaction().splits().isEmpty() ? MyMoneySplit() : m_schedule.transaction().splits().front(), false);
    }

    t.clearId();
    t.setEntryDate(QDate());
    return t;
  }
  
  KEditScheduleDlg              *q_ptr;
  Ui::KEditScheduleDlg          *ui;
  MyMoneySchedule                m_schedule;
  KMyMoneyRegister::Transaction* m_item;
  QWidgetList                    m_tabOrderWidgets;
  TransactionEditor*             m_editor;
  KMandatoryFieldGroup*          m_requiredFields;
};

KEditScheduleDlg::KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent) :
  QDialog(parent),
  d_ptr(new KEditScheduleDlgPrivate(this))
{
  Q_D(KEditScheduleDlg);
  d->m_schedule = schedule;
  d->m_editor = 0;
  d->init();
}

KEditScheduleDlg::~KEditScheduleDlg()
{
  Q_D(KEditScheduleDlg);
  delete d;
}

void KEditScheduleDlg::slotSetupSize()
{
  resize(width(), minimumSizeHint().height());
}

TransactionEditor* KEditScheduleDlg::startEdit()
{
  Q_D(KEditScheduleDlg);
  KMyMoneyRegister::SelectedTransactions list(d->ui->m_register);
  TransactionEditor* editor = d->m_item->createEditor(d->ui->m_form, list, QDate());

  // check that we use the same transaction commodity in all selected transactions
  // if not, we need to update this in the editor's list. The user can also bail out
  // of this operation which means that we have to stop editing here.
  if (editor && !d->m_schedule.account().id().isEmpty()) {
    if (!editor->fixTransactionCommodity(d->m_schedule.account())) {
      // if the user wants to quit, we need to destroy the editor
      // and bail out
      delete editor;
      editor = 0;
    }
  }

  if (editor) {
    editor->setScheduleInfo(d->ui->m_nameEdit->text());
    connect(editor, &TransactionEditor::transactionDataSufficient, d->ui->buttonBox->button(QDialogButtonBox::Ok), &QWidget::setEnabled);
    connect(editor, &TransactionEditor::escapePressed, d->ui->buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::animateClick);
    connect(editor, &TransactionEditor::returnPressed, d->ui->buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::animateClick);

    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, editor, &TransactionEditor::slotReloadEditWidgets);
    // connect(editor, SIGNAL(finishEdit(KMyMoneyRegister::SelectedTransactions)), this, SLOT(slotLeaveEditMode(KMyMoneyRegister::SelectedTransactions)));
    connect(editor, &TransactionEditor::createPayee,    this, &KEditScheduleDlg::slotPayeeNew);
    connect(editor, &TransactionEditor::createTag,      this, &KEditScheduleDlg::slotTagNew);
    connect(editor, &TransactionEditor::createCategory, this, &KEditScheduleDlg::slotCategoryNew);
    connect(editor, &TransactionEditor::createSecurity, this, &KEditScheduleDlg::slotInvestmentNew);
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, editor, &TransactionEditor::slotReloadEditWidgets);

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    eWidgets::eRegister::Action action = eWidgets::eRegister::Action::Withdrawal;
    switch (d->m_schedule.type()) {
      case Schedule::Type::Deposit:
        action = eWidgets::eRegister::Action::Deposit;
        break;
      case Schedule::Type::Bill:
        action = eWidgets::eRegister::Action::Withdrawal;
        editor->setPaymentMethod(d->m_schedule.paymentType());
        break;
      case Schedule::Type::Transfer:
        action = eWidgets::eRegister::Action::Transfer;
        break;
      default:
        // if we end up here, we don't have a known schedule type (yet). in this case, we just glimpse
        // into the transaction and determine the type. in case we don't have a transaction with splits
        // we stick with the default action already set up
        if (d->m_schedule.transaction().splits().count() > 0) {
          auto isDeposit = false;
          auto isTransfer = false;
          auto splits = d->m_schedule.transaction().splits();
          foreach (const auto split, splits) {
            if (split.accountId() == d->m_schedule.account().id()) {
              isDeposit = !(split.shares().isNegative());
            } else {
              auto acc = MyMoneyFile::instance()->account(split.accountId());
              if (acc.isAssetLiability() && d->m_schedule.transaction().splits().count() == 2) {
                isTransfer = true;
              }
            }
          }

          if (isTransfer)
            action = eWidgets::eRegister::Action::Transfer;
          else if (isDeposit)
            action = eWidgets::eRegister::Action::Deposit;
        }
        break;
    }
    editor->setup(d->m_tabOrderWidgets, d->m_schedule.account(), action);

    // if it's not a check, then we need to clear
    // a possibly assigned check number
    if (d->m_schedule.paymentType() != Schedule::PaymentType::WriteChecque) {
      QWidget* w = editor->haveWidget("number");
      if (w)
        dynamic_cast<KMyMoneyLineEdit*>(w)->loadText(QString());
    }

    Q_ASSERT(!d->m_tabOrderWidgets.isEmpty());

    d->m_tabOrderWidgets.push_front(d->ui->m_paymentMethodEdit);

    // editor->setup() leaves the tabbar as the last widget in the stack, but we
    // need it as first here. So we move it around.
    QWidget* w = editor->haveWidget("tabbar");
    if (w) {
      int idx = d->m_tabOrderWidgets.indexOf(w);
      if (idx != -1) {
        d->m_tabOrderWidgets.removeAt(idx);
        d->m_tabOrderWidgets.push_front(w);
      }
    }

    // don't forget our three buttons and additional widgets
    // make sure to use the correct order
    d->m_tabOrderWidgets.push_front(d->ui->m_frequencyEdit);
    d->m_tabOrderWidgets.push_front(d->ui->m_frequencyNoEdit);
    d->m_tabOrderWidgets.push_front(d->ui->m_nameEdit);

    d->m_tabOrderWidgets.append(d->ui->m_weekendOptionEdit);
    d->m_tabOrderWidgets.append(d->ui->m_estimateEdit);
    d->m_tabOrderWidgets.append(d->ui->m_variation);
    d->m_tabOrderWidgets.append(d->ui->m_lastDayInMonthEdit);
    d->m_tabOrderWidgets.append(d->ui->m_autoEnterEdit);
    d->m_tabOrderWidgets.append(d->ui->m_endSeriesEdit);
    d->m_tabOrderWidgets.append(d->ui->m_RemainingEdit);
    d->m_tabOrderWidgets.append(d->ui->m_FinalPaymentEdit);

    d->m_tabOrderWidgets.append(d->ui->buttonBox->button(QDialogButtonBox::Ok));
    d->m_tabOrderWidgets.append(d->ui->buttonBox->button(QDialogButtonBox::Cancel));
    d->m_tabOrderWidgets.append(d->ui->buttonBox->button(QDialogButtonBox::Help));
    for (auto i = 0; i < d->m_tabOrderWidgets.size(); ++i) {
      QWidget* w = d->m_tabOrderWidgets.at(i);
      if (w) {
        w->installEventFilter(this);
        w->installEventFilter(editor);
      }
    }

    // connect the postdate modification signal to our update routine
    KMyMoneyDateInput* dateEdit = dynamic_cast<KMyMoneyDateInput*>(editor->haveWidget("postdate"));
    if (dateEdit)
      connect(dateEdit, &KMyMoneyDateInput::dateChanged, this, &KEditScheduleDlg::slotPostDateChanged);

    d->ui->m_nameEdit->setFocus();

    // add the required fields to the mandatory group
    d->m_requiredFields->add(d->ui->m_nameEdit);
    d->m_requiredFields->add(editor->haveWidget("account"));
    d->m_requiredFields->add(editor->haveWidget("category"));
    d->m_requiredFields->add(editor->haveWidget("amount"));

    // fix labels
    QLabel* label = dynamic_cast<QLabel*>(editor->haveWidget("date-label"));
    if (label) {
      label->setText(i18n("Next due date"));
    }

    d->m_editor = editor;
    slotSetPaymentMethod((int)d->m_schedule.paymentType());

    connect(d->ui->m_paymentMethodEdit, &KMyMoneyGeneralCombo::itemSelected, this, &KEditScheduleDlg::slotSetPaymentMethod);
    connect(editor, &TransactionEditor::operationTypeChanged, this, &KEditScheduleDlg::slotFilterPaymentType);
  }

  return editor;
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
  if (d->m_editor) {
    auto t = d->transaction();
    if (d->m_schedule.nextDueDate() != t.postDate()) {
      d->m_schedule.setNextDueDate(t.postDate());
      d->m_schedule.setStartDate(t.postDate());
    }
    d->m_schedule.setTransaction(t);
    d->m_schedule.setName(d->ui->m_nameEdit->text());
    d->m_schedule.setFixed(!d->ui->m_estimateEdit->isChecked());
    d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(d->ui->m_frequencyEdit->currentItem()));
    d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

    switch (d->ui->m_weekendOptionEdit->currentIndex())  {
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

    KMyMoneyTransactionForm::TabBar* tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(d->m_editor->haveWidget("tabbar"));
    if (tabbar) {
      switch (static_cast<eWidgets::eRegister::Action>(tabbar->currentIndex())) {
        case eWidgets::eRegister::Action::Deposit:
          d->m_schedule.setType(Schedule::Type::Deposit);
          break;
        default:
        case eWidgets::eRegister::Action::Withdrawal:
          d->m_schedule.setType(Schedule::Type::Bill);
          break;
        case eWidgets::eRegister::Action::Transfer:
          d->m_schedule.setType(Schedule::Type::Transfer);
          break;
      }
    } else {
      qDebug("No tabbar found in KEditScheduleDlg::schedule(). Defaulting type to BILL");
    }

    if(d->ui->m_lastDayInMonthEdit->isEnabled())
      d->m_schedule.setLastDayInMonth(d->ui->m_lastDayInMonthEdit->isChecked());
    else
      d->m_schedule.setLastDayInMonth(false);
    d->m_schedule.setAutoEnter(d->ui->m_autoEnterEdit->isChecked());
    d->m_schedule.setPaymentType(static_cast<Schedule::PaymentType>(d->ui->m_paymentMethodEdit->currentItem()));
    if (d->ui->m_endSeriesEdit->isEnabled() && d->ui->m_endSeriesEdit->isChecked()) {
      d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
    } else {
      d->m_schedule.setEndDate(QDate());
    }
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

  QPointer<KEditScheduleDlg> dlg = new KEditScheduleDlg(schedule, nullptr);
  QPointer<TransactionEditor> transactionEditor = dlg->startEdit();
  if (transactionEditor) {
    KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
    if (dlg->exec() == QDialog::Accepted && dlg != 0) {
      MyMoneyFileTransaction ft;
      try {
        schedule = dlg->schedule();
        MyMoneyFile::instance()->addSchedule(schedule);
        ft.commit();

      } catch (const MyMoneyException &e) {
        KMessageBox::error(nullptr, i18n("Unable to add scheduled transaction: %1", e.what()), i18n("Add scheduled transaction"));
      }
    }
  }
  delete transactionEditor;
  delete dlg;
}

void KEditScheduleDlg::editSchedule(const MyMoneySchedule& inputSchedule)
{
    try {
      auto schedule = MyMoneyFile::instance()->schedule(inputSchedule.id());

      KEditScheduleDlg* sched_dlg = nullptr;
      KEditLoanWizard* loan_wiz = nullptr;

      switch (schedule.type()) {
        case eMyMoney::Schedule::Type::Bill:
        case eMyMoney::Schedule::Type::Deposit:
        case eMyMoney::Schedule::Type::Transfer:
        {
          sched_dlg = new KEditScheduleDlg(schedule, nullptr);
          QPointer<TransactionEditor> transactionEditor = sched_dlg->startEdit();
          if (transactionEditor) {
            KMyMoneyMVCCombo::setSubstringSearchForChildren(sched_dlg, !KMyMoneySettings::stringMatchFromStart());
            if (sched_dlg->exec() == QDialog::Accepted) {
              MyMoneyFileTransaction ft;
              try {
                MyMoneySchedule sched = sched_dlg->schedule();
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
                  if (KMessageBox::questionYesNo(nullptr, i18n("<qt>You have entered a scheduled transaction date of <b>%1</b>.  Because the scheduled transaction was last paid on <b>%2</b>, KMyMoney will automatically adjust the scheduled transaction date to the next date unless the last payment date is reset.  Do you want to reset the last payment date?</qt>", QLocale().toString(next, QLocale::ShortFormat), QLocale().toString(last, QLocale::ShortFormat)), i18n("Reset Last Payment Date"), KStandardGuiItem::yes(), KStandardGuiItem::no()) == KMessageBox::Yes) {
                    sched.setLastPayment(QDate());
                  }
                }
                MyMoneyFile::instance()->modifySchedule(sched);
                // delete the editor before we emit the dataChanged() signal from the
                // engine. Calling this twice in a row does not hurt.
                delete transactionEditor;
                ft.commit();
              } catch (const MyMoneyException &e) {
                KMessageBox::detailedSorry(nullptr, i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()), e.what());
              }
            }
            delete transactionEditor;
          }
          delete sched_dlg;
          break;
        }
        case eMyMoney::Schedule::Type::LoanPayment:
        {
          loan_wiz = new KEditLoanWizard(schedule.account(2));
          if (loan_wiz->exec() == QDialog::Accepted) {
            MyMoneyFileTransaction ft;
            try {
              MyMoneyFile::instance()->modifySchedule(loan_wiz->schedule());
              MyMoneyFile::instance()->modifyAccount(loan_wiz->account());
              ft.commit();
            } catch (const MyMoneyException &e) {
              KMessageBox::detailedSorry(nullptr, i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()), e.what());
            }
          }
          delete loan_wiz;
          break;
        }
        case eMyMoney::Schedule::Type::Any:
          break;
      }

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(nullptr, i18n("Unable to modify scheduled transaction '%1'", inputSchedule.name()), e.what());
    }
}

bool KEditScheduleDlg::focusNextPrevChild(bool next)
{
  Q_D(KEditScheduleDlg);
  auto rc = false;

  auto w = qApp->focusWidget();
  auto currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
  while (w && currentWidgetIndex == -1) {
    // qDebug("'%s' not in list, use parent", qPrintable(w->objectName()));
    w = w->parentWidget();
    currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
  }

  if (currentWidgetIndex != -1) {
    do {
      // if(w) qDebug("tab order is at '%s (%d/%d)'", qPrintable(w->objectName()), currentWidgetIndex, d->m_tabOrderWidgets.size());
      currentWidgetIndex += next ? 1 : -1;
      if (currentWidgetIndex < 0)
        currentWidgetIndex = d->m_tabOrderWidgets.size() - 1;
      else if (currentWidgetIndex >= d->m_tabOrderWidgets.size())
        currentWidgetIndex = 0;

      w = d->m_tabOrderWidgets[currentWidgetIndex];
      // qDebug("currentWidgetIndex = %d, w = %p", currentWidgetIndex, w);

      if (((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus) && w->isVisible() && w->isEnabled()) {
        // qDebug("Selecting '%s' as focus", qPrintable(w->objectName()));
        w->setFocus();
        rc = true;
      }
    } while (rc == false);
  }
  return rc;
}

void KEditScheduleDlg::resizeEvent(QResizeEvent* ev)
{
  Q_D(KEditScheduleDlg);
  d->ui->m_register->resize((int)eWidgets::eTransaction::Column::Detail);
  d->ui->m_form->resize((int)eWidgets::eTransactionForm::Column::Value1);
  QDialog::resizeEvent(ev);
}


void KEditScheduleDlg::slotRemainingChanged(int value)
{
  Q_D(KEditScheduleDlg);
  // Make sure the required fields are set
  auto dateEdit = dynamic_cast<KMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setNextDueDate(dateEdit->date());
  d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(d->ui->m_frequencyEdit->currentItem()));
  d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

  if (d->m_schedule.transactionsRemaining() != value) {
    d->ui->m_FinalPaymentEdit->blockSignals(true);
    d->ui->m_FinalPaymentEdit->setDate(d->m_schedule.dateAfter(value));
    d->ui->m_FinalPaymentEdit->blockSignals(false);
  }
}

void KEditScheduleDlg::slotEndDateChanged(const QDate& date)
{
  Q_D(KEditScheduleDlg);
  // Make sure the required fields are set
  auto dateEdit = dynamic_cast<KMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setNextDueDate(dateEdit->date());
  d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(d->ui->m_frequencyEdit->currentItem()));
  d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());

  if (d->m_schedule.endDate() != date) {
    d->m_schedule.setEndDate(date);
    d->updateTransactionsRemaining();
  }
}

void KEditScheduleDlg::slotPostDateChanged(const QDate& date)
{
  Q_D(KEditScheduleDlg);
  if (d->m_schedule.nextDueDate() != date) {
    if (d->ui->m_endOptionsFrame->isEnabled()) {
      d->m_schedule.setNextDueDate(date);
      d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());
      d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(d->ui->m_frequencyEdit->currentItem()));
      d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
      d->updateTransactionsRemaining();
    }
  }
}

void KEditScheduleDlg::slotSetPaymentMethod(int item)
{
  Q_D(KEditScheduleDlg);
  auto dateEdit = dynamic_cast<KMyMoneyLineEdit*>(d->m_editor->haveWidget("number"));
  if (dateEdit) {
    dateEdit->setVisible(item == (int)Schedule::PaymentType::WriteChecque);

    // hiding the label does not work, because the label underneath will shine
    // through. So we either write the label or a blank
    QLabel* label = dynamic_cast<QLabel *>(d->m_editor->haveWidget("number-label"));
    if (label) {
      label->setText((item == (int)Schedule::PaymentType::WriteChecque) ? i18n("Number") : " ");
    }
  }
}

void KEditScheduleDlg::slotFrequencyChanged(int item)
{
  Q_D(KEditScheduleDlg);
  d->ui->m_endSeriesEdit->setEnabled(item != (int)Schedule::Occurrence::Once);
  bool isEndSeries = d->ui->m_endSeriesEdit->isChecked();
  if (isEndSeries)
    d->ui->m_endOptionsFrame->setEnabled(item != (int)Schedule::Occurrence::Once);
  switch (item) {
    case (int)Schedule::Occurrence::Daily:
    case (int)Schedule::Occurrence::Weekly:
      d->ui->m_frequencyNoEdit->setEnabled(true);
      d->ui->m_lastDayInMonthEdit->setEnabled(false);
      break;

    case (int)Schedule::Occurrence::EveryHalfMonth:
    case (int)Schedule::Occurrence::Monthly:
    case (int)Schedule::Occurrence::Yearly:
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
  if (isEndSeries && (item != (int)Schedule::Occurrence::Once)) {
    // Changing the frequency changes the number
    // of remaining transactions
    KMyMoneyDateInput* dateEdit = dynamic_cast<KMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
    d->m_schedule.setNextDueDate(dateEdit->date());
    d->m_schedule.setOccurrenceMultiplier(d->ui->m_frequencyNoEdit->value());
    d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(item));
    d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
    d->updateTransactionsRemaining();
  }
}

void KEditScheduleDlg::slotOccurrenceMultiplierChanged(int multiplier)
{
  Q_D(KEditScheduleDlg);
  // Make sure the required fields are set
  auto oldOccurrenceMultiplier = d->m_schedule.occurrenceMultiplier();
  if (multiplier != oldOccurrenceMultiplier) {
    if (d->ui->m_endOptionsFrame->isEnabled()) {
      KMyMoneyDateInput* dateEdit = dynamic_cast<KMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
      d->m_schedule.setNextDueDate(dateEdit->date());
      d->m_schedule.setOccurrenceMultiplier(multiplier);
      d->m_schedule.setOccurrencePeriod(static_cast<Schedule::Occurrence>(d->ui->m_frequencyEdit->currentItem()));
      d->m_schedule.setEndDate(d->ui->m_FinalPaymentEdit->date());
      d->updateTransactionsRemaining();
    }
  }
}

void KEditScheduleDlg::slotShowHelp()
{
  KHelpClient::invokeHelp("details.schedules.intro");
}

void KEditScheduleDlg::slotFilterPaymentType(int index)
{
  Q_D(KEditScheduleDlg);
  //save selected item to reload if possible
  auto selectedId = d->ui->m_paymentMethodEdit->itemData(d->ui->m_paymentMethodEdit->currentIndex(), Qt::UserRole).toInt();

  //clear and reload the widget with the correct items
  d->ui->m_paymentMethodEdit->clear();

  // load option widgets
  eWidgets::eRegister::Action action = static_cast<eWidgets::eRegister::Action>(index);
  if (action != eWidgets::eRegister::Action::Withdrawal) {
    d->ui->m_paymentMethodEdit->insertItem(i18n("Direct deposit"), (int)Schedule::PaymentType::DirectDeposit);
    d->ui->m_paymentMethodEdit->insertItem(i18n("Manual deposit"), (int)Schedule::PaymentType::ManualDeposit);
  }
  if (action != eWidgets::eRegister::Action::Deposit) {
    d->ui->m_paymentMethodEdit->insertItem(i18n("Direct debit"), (int)Schedule::PaymentType::DirectDebit);
    d->ui->m_paymentMethodEdit->insertItem(i18n("Write check"), (int)Schedule::PaymentType::WriteChecque);
  }
  d->ui->m_paymentMethodEdit->insertItem(i18n("Standing order"), (int)Schedule::PaymentType::StandingOrder);
  d->ui->m_paymentMethodEdit->insertItem(i18n("Bank transfer"), (int)Schedule::PaymentType::BankTransfer);
  d->ui->m_paymentMethodEdit->insertItem(i18nc("Other payment method", "Other"), (int)Schedule::PaymentType::Other);

  auto newIndex = d->ui->m_paymentMethodEdit->findData(QVariant(selectedId), Qt::UserRole, Qt::MatchExactly);
  if (newIndex > -1) {
    d->ui->m_paymentMethodEdit->setCurrentIndex(newIndex);
  } else {
    d->ui->m_paymentMethodEdit->setCurrentIndex(0);
  }

}

void KEditScheduleDlg::slotPayeeNew(const QString& newnameBase, QString& id)
{
  KMyMoneyUtils::newPayee(newnameBase, id);
}

void KEditScheduleDlg::slotTagNew(const QString& newnameBase, QString& id)
{
  KMyMoneyUtils::newTag(newnameBase, id);
}

void KEditScheduleDlg::slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewAccountDlg::newCategory(account, parent);
}

void KEditScheduleDlg::slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewInvestmentWizard::newInvestment(account, parent);
}

