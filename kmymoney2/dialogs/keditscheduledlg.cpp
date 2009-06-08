/***************************************************************************
                          keditscheduledlg.cpp  -  description
                             -------------------
    begin                : Mon Sep  3 2007
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <qwidget.h>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <q3valuevector.h>
//Added by qt3to4:
#include <QList>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <klineedit.h>
#include <knuminput.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <register.h>
#include <transactionform.h>
#include <transaction.h>
#include <transactioneditor.h>
#include <kmymoneylineedit.h>
#include <kmymoneydateinput.h>
#include <kmymoneycombo.h>
#include <kguiutils.h>
#include <kmymoneyutils.h>
#include <KToolInvocation>

#include "keditscheduledlg.h"
#include "kmymoney2.h"

class KEditScheduleDlg::Private {
public:
  MyMoneySchedule                m_schedule;
  KMyMoneyRegister::Transaction* m_item;
  QWidgetList                    m_tabOrderWidgets;
  TransactionEditor*             m_editor;
  kMandatoryFieldGroup*          m_requiredFields;
};

KEditScheduleDlg::KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent) :
  KEditScheduleDlgDecl(parent),
  d(new Private)
{
    setModal( true );
  d->m_schedule = schedule;
  d->m_editor = 0;

  buttonOk->setGuiItem(KStandardGuiItem::ok());
  buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  buttonHelp->setGuiItem(KStandardGuiItem::help());

  d->m_requiredFields = new kMandatoryFieldGroup (this);
  d->m_requiredFields->setOkButton(buttonOk); // button to be enabled when all fields present
#warning "port to kde4"
#if 0
  // make sure, we have a tabbar with the form
  // insert it after the horizontal line
  m_paymentInformationLayout->insertWidget(2, m_form->tabBar(m_form->parentWidget()));
#endif
  // we never need to see the register
  m_register->hide();

  // ... setup the form ...
  m_form->setupForm(d->m_schedule.account());

  // ... and the register ...
  m_register->clear();

  // ... now add the transaction to register and form ...
  MyMoneyTransaction t = transaction();
  if( d->m_schedule.transaction().splits().isEmpty())
	  d->m_item = KMyMoneyRegister::Register::transactionFactory(m_register, t, MyMoneySplit(), 0);
  else
          d->m_item = KMyMoneyRegister::Register::transactionFactory(m_register, t, d->m_schedule.transaction().splits()[0], 0);
  m_register->selectItem(d->m_item);
  // show the account row
  d->m_item->setShowRowInForm(0, true);

  m_form->slotSetTransaction(d->m_item);

  // setup widget contents
  m_nameEdit->setText(d->m_schedule.name());

  m_frequencyEdit->setCurrentItem(d->m_schedule.occurencePeriod());
  if(m_frequencyEdit->currentItem() == -1)
    m_frequencyEdit->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);
  slotFrequencyChanged(m_frequencyEdit->currentItem());
  m_frequencyNoEdit->setValue(d->m_schedule.occurenceMultiplier());

  // load option widgets
  m_paymentMethodEdit->insertItem(i18n("Direct deposit"), MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  m_paymentMethodEdit->insertItem(i18n("Manual deposit"), MyMoneySchedule::STYPE_MANUALDEPOSIT);
  m_paymentMethodEdit->insertItem(i18n("Direct debit"), MyMoneySchedule::STYPE_DIRECTDEBIT);
  m_paymentMethodEdit->insertItem(i18n("Standing order"), MyMoneySchedule::STYPE_STANDINGORDER);
  m_paymentMethodEdit->insertItem(i18n("Bank transfer"), MyMoneySchedule::STYPE_BANKTRANSFER);
  m_paymentMethodEdit->insertItem(i18n("Write check"), MyMoneySchedule::STYPE_WRITECHEQUE);
  m_paymentMethodEdit->insertItem(i18n("Other"), MyMoneySchedule::STYPE_OTHER);

  MyMoneySchedule::paymentTypeE method = d->m_schedule.paymentType();
  if(method == MyMoneySchedule::STYPE_ANY)
    method = MyMoneySchedule::STYPE_OTHER;
  m_paymentMethodEdit->setCurrentItem(method);

  switch(d->m_schedule.weekendOption()) {
    case MyMoneySchedule::MoveNothing:
      m_weekendOptionEdit->setCurrentIndex(0);
      break;
    case MyMoneySchedule::MoveFriday:
      m_weekendOptionEdit->setCurrentIndex(1);
      break;
    case MyMoneySchedule::MoveMonday:
      m_weekendOptionEdit->setCurrentIndex(2);
      break;
  }
  m_estimateEdit->setChecked(!d->m_schedule.isFixed());
  m_autoEnterEdit->setChecked(d->m_schedule.autoEnter());
  m_endSeriesEdit->setChecked(d->m_schedule.willEnd());

  m_endOptionsFrame->setEnabled(d->m_schedule.willEnd());
  if(d->m_schedule.willEnd()) {
    m_RemainingEdit->setValue(d->m_schedule.transactionsRemaining());
    m_FinalPaymentEdit->setDate(d->m_schedule.endDate());
  }

  connect(m_RemainingEdit, SIGNAL(valueChanged(int)),
    this, SLOT(slotRemainingChanged(int)));
  connect(m_FinalPaymentEdit, SIGNAL(dateChanged(const QDate&)),
    this, SLOT(slotEndDateChanged(const QDate&)));
  connect(m_frequencyEdit, SIGNAL(itemSelected(int)),
    this, SLOT(slotFrequencyChanged(int)));
  connect(m_frequencyNoEdit, SIGNAL(valueChanged(int)),
    this, SLOT(slotOccurenceMultiplierChanged(int)));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotShowHelp()));

  // force the initial height to be as small as possible
  QTimer::singleShot(0, this, SLOT(slotSetupSize()));

  // we just hide the variation field for now and enable the logic
  // once we have a respective member in the MyMoneySchedule object
  m_variation->hide();
}

KEditScheduleDlg::~KEditScheduleDlg()
{
  delete d;
}

void KEditScheduleDlg::slotSetupSize(void)
{
  resize(width(), minimumSizeHint().height());
}

TransactionEditor* KEditScheduleDlg::startEdit(void)
{
  KMyMoneyRegister::SelectedTransactions list(m_register);
  TransactionEditor* editor = d->m_item->createEditor(m_form, list, QDate());

  // check that we use the same transaction commodity in all selected transactions
  // if not, we need to update this in the editor's list. The user can also bail out
  // of this operation which means that we have to stop editing here.
  if(editor) {
    if(!editor->fixTransactionCommodity(d->m_schedule.account())) {
      // if the user wants to quit, we need to destroy the editor
      // and bail out
      delete editor;
      editor = 0;
    }
  }

  if(editor) {
    connect(editor, SIGNAL(transactionDataSufficient(bool)), buttonOk, SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(escapePressed()), buttonCancel, SLOT(animateClick()));
    connect(editor, SIGNAL(returnPressed()), buttonOk, SLOT(animateClick()));

    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));
    // connect(editor, SIGNAL(finishEdit(const KMyMoneyRegister::SelectedTransactions&)), this, SLOT(slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions&)));
    connect(editor, SIGNAL(createPayee(const QString&, QString&)), kmymoney2, SLOT(slotPayeeNew(const QString&, QString&)));
    connect(editor, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));
    connect(editor, SIGNAL(createSecurity(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotInvestmentNew(MyMoneyAccount&, const MyMoneyAccount&)));
    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    KMyMoneyRegister::Action action = KMyMoneyRegister::ActionWithdrawal;
    switch(d->m_schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        action = KMyMoneyRegister::ActionDeposit;
        break;
      case MyMoneySchedule::TYPE_BILL:
        action = KMyMoneyRegister::ActionWithdrawal;
        break;
      case MyMoneySchedule::TYPE_TRANSFER:
        action = KMyMoneyRegister::ActionTransfer;
        break;
      default:
        // if we end up here, we don't have a known schedule type (yet). in this case, we just glimpse
        // into the transaction and determine the type. in case we don't have a transaction with splits
        // we stick with the default action already set up
        if(d->m_schedule.transaction().splits().count() > 0) {
          QList<MyMoneySplit>::const_iterator it_s;
          bool isDeposit = false;
          bool isTransfer = false;
          for(it_s = d->m_schedule.transaction().splits().begin(); it_s != d->m_schedule.transaction().splits().end(); ++it_s) {
            if((*it_s).accountId() == d->m_schedule.account().id()) {
              isDeposit = !((*it_s).shares().isNegative());
            } else {
              MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
              if(acc.isAssetLiability() && d->m_schedule.transaction().splits().count() == 2) {
                isTransfer = true;
              }
            }
          }

          if(isTransfer)
            action = KMyMoneyRegister::ActionTransfer;
          else if(isDeposit)
            action = KMyMoneyRegister::ActionDeposit;
        }
        break;
    }
    editor->setup(d->m_tabOrderWidgets, d->m_schedule.account(), action);

    // if it's not a check, then we need to clear
    // a possibly assigned check number
    if(d->m_schedule.paymentType() != MyMoneySchedule::STYPE_WRITECHEQUE) {
      QWidget* w = editor->haveWidget("number");
      if(w)
        dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(QString());
    }

    Q_ASSERT(!d->m_tabOrderWidgets.isEmpty());

    // don't forget our three buttons and additional widgets
    d->m_tabOrderWidgets.append(m_weekendOptionEdit);
    d->m_tabOrderWidgets.append(m_estimateEdit);
    d->m_tabOrderWidgets.append(m_variation);
    d->m_tabOrderWidgets.append(m_autoEnterEdit);
    d->m_tabOrderWidgets.append(m_endSeriesEdit);
    d->m_tabOrderWidgets.append(m_RemainingEdit);
    d->m_tabOrderWidgets.append(m_FinalPaymentEdit);

    d->m_tabOrderWidgets.append(buttonOk);
    d->m_tabOrderWidgets.append(buttonCancel);
    d->m_tabOrderWidgets.append(buttonHelp);
    d->m_tabOrderWidgets.append(m_nameEdit);
    d->m_tabOrderWidgets.append(m_frequencyNoEdit);
    d->m_tabOrderWidgets.append(m_frequencyEdit);
    d->m_tabOrderWidgets.append(m_paymentMethodEdit);
    d->m_tabOrderWidgets.append(m_form);
    for (int i = 0; i < d->m_tabOrderWidgets.size(); ++i) {
        QWidget* w = d->m_tabOrderWidgets.at(i);
	if(w) {
           w->installEventFilter(this);
           w->installEventFilter(editor);
        }
     }

    // connect the postdate modification signal to our update routine
    kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(editor->haveWidget("postdate"));
    if(dateEdit)
      connect(dateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotPostDateChanged(const QDate&)));


    m_nameEdit->setFocus();

    // add the required fields to the mandatory group
    d->m_requiredFields->add(m_nameEdit);
    d->m_requiredFields->add(editor->haveWidget("account"));
    d->m_requiredFields->add(editor->haveWidget("category"));

    // fix labels
    QLabel* label = dynamic_cast<QLabel*>(editor->haveWidget("date-label"));
    if(label) {
      label->setText(i18n("Next due date"));
    }

    d->m_editor = editor;
    slotSetPaymentMethod(d->m_schedule.paymentType());

    connect(m_paymentMethodEdit, SIGNAL(itemSelected(int)), this, SLOT(slotSetPaymentMethod(int)));
  }

  return editor;
}

void KEditScheduleDlg::accept(void)
{
  // Force the focus to be on the OK button. This will trigger creation
  // of any unknown objects (payees, categories etc.)
  buttonOk->setFocus();

  // only accept if the button is really still enabled. We could end
  // up here, if the user filled all fields, the focus is on the category
  // field, but the category is not yet existant. When the user presses the
  // OK button in this context, he will be asked if he wants to create
  // the category or not. In case he decides no, we end up here with no
  // category filled in, so we don't run through the final acceptance.
  if(buttonOk->isEnabled())
    KEditScheduleDlgDecl::accept();
}

const MyMoneySchedule& KEditScheduleDlg::schedule(void) const
{
  if(d->m_editor) {
    MyMoneyTransaction t = transaction();
    if(d->m_schedule.nextDueDate() != t.postDate())
        d->m_schedule.setNextDueDate(t.postDate());
    d->m_schedule.setTransaction(t);
    d->m_schedule.setName(m_nameEdit->text());
    d->m_schedule.setFixed(!m_estimateEdit->isChecked());
    d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));
    d->m_schedule.setOccurenceMultiplier( m_frequencyNoEdit->value() );

    switch(m_weekendOptionEdit->currentItem())  {
      case 0:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveNothing);
        break;
      case 1:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveFriday);
        break;
      case 2:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveMonday);
        break;
    }

    d->m_schedule.setType(MyMoneySchedule::TYPE_BILL);
#warning "port to kde4"
#if 0
    KMyMoneyTransactionForm::TabBar* tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(d->m_editor->haveWidget("tabbar"));
    if(tabbar) {
      switch(static_cast<KMyMoneyRegister::Action>(tabbar->currentTab())) {
        case KMyMoneyRegister::ActionDeposit:
          d->m_schedule.setType(MyMoneySchedule::TYPE_DEPOSIT);
          break;
        default:
        case KMyMoneyRegister::ActionWithdrawal:
          d->m_schedule.setType(MyMoneySchedule::TYPE_BILL);
          break;
        case KMyMoneyRegister::ActionTransfer:
          d->m_schedule.setType(MyMoneySchedule::TYPE_TRANSFER);
          break;
      }
    } else {
      qDebug("No tabbar found in KEditScheduleDlg::schedule(). Defaulting type to BILL");
    }
#endif
    d->m_schedule.setAutoEnter(m_autoEnterEdit->isChecked());
    d->m_schedule.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(m_paymentMethodEdit->currentItem()));
    if(m_endSeriesEdit->isEnabled() && m_endSeriesEdit->isChecked()) {
      d->m_schedule.setEndDate(m_FinalPaymentEdit->date());
    } else {
      d->m_schedule.setEndDate(QDate());
    }
  }
  return d->m_schedule;
}

MyMoneyTransaction KEditScheduleDlg::transaction(void) const
{
  MyMoneyTransaction t = d->m_schedule.transaction();

  if(d->m_editor) {
    d->m_editor->createTransaction(t, d->m_schedule.transaction(), d->m_schedule.transaction().splits()[0], false);
  }

  t.clearId();
  t.setEntryDate(QDate());
  return t;
}
#if 0
QDate KEditScheduleDlg::adjustDate(const QDate& _date) const
{
  QDate date(_date);
  if (d->m_schedule.weekendOption() != MyMoneySchedule::MoveNothing) {
    int dayOfWeek = date.dayOfWeek();
    if (dayOfWeek >= 6) {
      if (d->m_schedule.weekendOption() == MyMoneySchedule::MoveFriday) {
        if (dayOfWeek == 7)
          date = date.addDays(-2);
        else
          date = date.addDays(-1);
      } else {
        if (dayOfWeek == 6)
          date = date.addDays(2);
        else
          date = date.addDays(1);
      }
    }
  }
  return date;
}
#endif

bool KEditScheduleDlg::focusNextPrevChild(bool next)
{
  bool  rc = false;
#warning "port to kde4"
#if 0
  // qDebug("KEditScheduleDlg::focusNextPrevChild(editmode=%s)", m_inEditMode ? "true" : "false");
  QWidget *w = 0;
  QWidget *currentWidget;

  w = qApp->focusWidget();
  while(w && d->m_tabOrderWidgets.find(w) == -1) {
    // qDebug("'%s' not in list, use parent", w->className());
    w = w->parentWidget();
  }
  // if(w) qDebug("tab order is at '%s'", w->className());
  currentWidget = d->m_tabOrderWidgets.current();
  w = next ? d->m_tabOrderWidgets.next() : d->m_tabOrderWidgets.prev();

  do {
    if(!w) {
      w = next ? d->m_tabOrderWidgets.first() : d->m_tabOrderWidgets.last();
    }

    if(w != currentWidget
    && ((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
    && w->isVisible() && w->isEnabled()) {
      // qDebug("Selecting '%s' as focus", w->className());
      w->setFocus();
      rc = true;
      break;
    }
    w = next ? d->m_tabOrderWidgets.next() : d->m_tabOrderWidgets.prev();
  } while(w != currentWidget);
#endif
  return rc;
}

void KEditScheduleDlg::resizeEvent(QResizeEvent* ev)
{
  m_register->resize(KMyMoneyRegister::DetailColumn);
  m_form->resize(KMyMoneyTransactionForm::ValueColumn1);
  KEditScheduleDlgDecl::resizeEvent(ev);
}


void KEditScheduleDlg::slotRemainingChanged(int value)
{
  // Make sure the required fields are set
  kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setNextDueDate(dateEdit->date());
  d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));
  d->m_schedule.setOccurenceMultiplier(m_frequencyNoEdit->value());

  if(d->m_schedule.transactionsRemaining() != value) {
    m_FinalPaymentEdit->blockSignals(true);
    m_FinalPaymentEdit->setDate(d->m_schedule.dateAfter(value));
    m_FinalPaymentEdit->blockSignals(false);
  }
}

void KEditScheduleDlg::slotEndDateChanged(const QDate& date)
{
  // Make sure the required fields are set
  kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setNextDueDate(dateEdit->date());
  d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));
  d->m_schedule.setOccurenceMultiplier(m_frequencyNoEdit->value());

  if(d->m_schedule.endDate() != date) {
    d->m_schedule.setEndDate(date);
    updateTransactionsRemaining();
  }
}

void KEditScheduleDlg::slotPostDateChanged(const QDate& date)
{
  if(d->m_schedule.nextDueDate() != date) {
    if (m_endOptionsFrame->isEnabled()) {
      d->m_schedule.setNextDueDate(date);
      d->m_schedule.setOccurenceMultiplier(m_frequencyNoEdit->value());
      d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));
      d->m_schedule.setEndDate(m_FinalPaymentEdit->date());
      updateTransactionsRemaining();
    }
  }
}

void KEditScheduleDlg::slotSetPaymentMethod(int item)
{
  kMyMoneyLineEdit* dateEdit = dynamic_cast<kMyMoneyLineEdit*>(d->m_editor->haveWidget("number"));
  if(dateEdit) {
    dateEdit->setShown(item == MyMoneySchedule::STYPE_WRITECHEQUE);

    // hiding the label does not work, because the label underneath will shine
    // through. So we either write the label or a blank
    QLabel* label = dynamic_cast<QLabel *>(d->m_editor->haveWidget("number-label"));
    if(label) {
      label->setText((item == MyMoneySchedule::STYPE_WRITECHEQUE) ? i18n("Number") : " ");
    }
  }
}

void KEditScheduleDlg::slotFrequencyChanged(int item)
{
  m_endSeriesEdit->setEnabled(item != MyMoneySchedule::OCCUR_ONCE);
  bool isEndSeries = m_endSeriesEdit->isChecked();
  if(isEndSeries )
    m_endOptionsFrame->setEnabled(item != MyMoneySchedule::OCCUR_ONCE);
  switch( item )
  {
    case MyMoneySchedule::OCCUR_DAILY:
    case MyMoneySchedule::OCCUR_WEEKLY:
    case MyMoneySchedule::OCCUR_EVERYHALFMONTH:
    case MyMoneySchedule::OCCUR_MONTHLY:
    case MyMoneySchedule::OCCUR_YEARLY:
      // Supports Frequency Number
      m_frequencyNoEdit->setEnabled(true);
      break;
    default:
      // Multiplier is always 1
      m_frequencyNoEdit->setEnabled(false);
      m_frequencyNoEdit->setValue(1);
      break;
  }
  if ( isEndSeries && ( item != MyMoneySchedule::OCCUR_ONCE ) )
  {
    // Changing the frequency changes the number
    // of remaining transactions
    kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
    d->m_schedule.setNextDueDate(dateEdit->date());
    d->m_schedule.setOccurenceMultiplier(m_frequencyNoEdit->value());
    d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(item));
    d->m_schedule.setEndDate(m_FinalPaymentEdit->date());
    updateTransactionsRemaining();
  }
}

void KEditScheduleDlg::slotOccurenceMultiplierChanged(int multiplier)
{
  // Make sure the required fields are set
  int oldOccurenceMultiplier = d->m_schedule.occurenceMultiplier();
  if ( multiplier != oldOccurenceMultiplier )
  {
    if (m_endOptionsFrame->isEnabled())
    {
      kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
      d->m_schedule.setNextDueDate(dateEdit->date());
      d->m_schedule.setOccurenceMultiplier(multiplier);
      d->m_schedule.setOccurencePeriod(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));
      d->m_schedule.setEndDate(m_FinalPaymentEdit->date());
      updateTransactionsRemaining();
    }
  }
}

void KEditScheduleDlg::updateTransactionsRemaining(void)
{
  int remain = d->m_schedule.transactionsRemaining();
  if ( remain != m_RemainingEdit->value() )
  {
    m_RemainingEdit->blockSignals(true);
    m_RemainingEdit->setValue(remain);
    m_RemainingEdit->blockSignals(false);
  }
}

void KEditScheduleDlg::slotShowHelp(void)
{
  KToolInvocation::invokeHelp("details.schedules.intro");
}

#include <keditscheduledlg.moc>
#include <ktoolinvocation.h>
