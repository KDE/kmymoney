/***************************************************************************
                          kenterscheduledlg.cpp
                             -------------------
    begin                : Sat Apr  7 2007
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

#include "kenterscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>
#include <KLocalizedString>
#include <KGuiItem>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kenterscheduledlg.h"
#include "mymoneyfile.h"
#include "mymoneyschedule.h"
#include "register.h"
#include "transactionform.h"
#include "transaction.h"
#include "transactioneditor.h"
#include "kmymoneyutils.h"
#include "kmymoneylineedit.h"
#include "kmymoneydateinput.h"
#include "kmymoney.h"
#include "icons/icons.h"
#include "mymoneyenums.h"
#include "dialogenums.h"

using namespace Icons;

class KEnterScheduleDlgPrivate
{
  Q_DISABLE_COPY(KEnterScheduleDlgPrivate)

public:
  KEnterScheduleDlgPrivate() :
    ui(new Ui::KEnterScheduleDlg),
    m_item(nullptr),
    m_showWarningOnce(true)
  {
  }

  ~KEnterScheduleDlgPrivate()
  {
    delete ui;
  }
  
  Ui::KEnterScheduleDlg         *ui;
  MyMoneySchedule                m_schedule;
  KMyMoneyRegister::Transaction* m_item;
  QWidgetList                    m_tabOrderWidgets;
  bool                           m_showWarningOnce;
  eDialogs::ScheduleResultCode m_extendedReturnCode;
};

KEnterScheduleDlg::KEnterScheduleDlg(QWidget *parent, const MyMoneySchedule& schedule) :
  QDialog(parent),
  d_ptr(new KEnterScheduleDlgPrivate)
{
  Q_D(KEnterScheduleDlg);
  d->ui->setupUi(this);
  d->m_schedule = schedule;
  d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Enter;
  d->ui->buttonOk->setIcon(QIcon::fromTheme(g_Icons[Icon::KeyEnter]));
  d->ui->buttonSkip->setIcon(QIcon::fromTheme(g_Icons[Icon::MediaSeekForward]));
  KGuiItem::assign(d->ui->buttonCancel, KStandardGuiItem::cancel());
  KGuiItem::assign(d->ui->buttonHelp, KStandardGuiItem::help());
  d->ui->buttonIgnore->setHidden(true);
  d->ui->buttonSkip->setHidden(true);

  // make sure, we have a tabbar with the form
  KMyMoneyTransactionForm::TabBar* tabbar = d->ui->m_form->tabBar(d->ui->m_form->parentWidget());

  // we never need to see the register
  d->ui->m_register->hide();

  // ... setup the form ...
  d->ui->m_form->setupForm(d->m_schedule.account());

  // ... and the register ...
  d->ui->m_register->clear();

  // ... now add the transaction to register and form ...
  MyMoneyTransaction t = transaction();
  d->m_item = KMyMoneyRegister::Register::transactionFactory(d->ui->m_register, t,
              d->m_schedule.transaction().splits().isEmpty() ? MyMoneySplit() : d->m_schedule.transaction().splits().front(), 0);
  d->ui->m_register->selectItem(d->m_item);
  // show the account row
  d->m_item->setShowRowInForm(0, true);

  d->ui->m_form->slotSetTransaction(d->m_item);

  // no need to see the tabbar
  tabbar->hide();

  // setup name and type
  d->ui->m_scheduleName->setText(d->m_schedule.name());
  d->ui->m_type->setText(KMyMoneyUtils::scheduleTypeToString(d->m_schedule.type()));

  connect(d->ui->buttonHelp, &QAbstractButton::clicked, this, &KEnterScheduleDlg::slotShowHelp);
  connect(d->ui->buttonIgnore, &QAbstractButton::clicked, this, &KEnterScheduleDlg::slotIgnore);
  connect(d->ui->buttonSkip, &QAbstractButton::clicked, this, &KEnterScheduleDlg::slotSkip);
}

KEnterScheduleDlg::~KEnterScheduleDlg()
{
  Q_D(KEnterScheduleDlg);
  delete d;
}

eDialogs::ScheduleResultCode KEnterScheduleDlg::resultCode() const
{
  Q_D(const KEnterScheduleDlg);
  if (result() == QDialog::Accepted)
    return d->m_extendedReturnCode;
  return eDialogs::ScheduleResultCode::Cancel;
}

void KEnterScheduleDlg::showExtendedKeys(bool visible)
{
  Q_D(KEnterScheduleDlg);
  d->ui->buttonIgnore->setVisible(visible);
  d->ui->buttonSkip->setVisible(visible);
}

void KEnterScheduleDlg::slotIgnore()
{
  Q_D(KEnterScheduleDlg);
  d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Ignore;
  accept();
}

void KEnterScheduleDlg::slotSkip()
{
  Q_D(KEnterScheduleDlg);
  d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Skip;
  accept();
}

MyMoneyTransaction KEnterScheduleDlg::transaction()
{
  Q_D(KEnterScheduleDlg);
  auto t = d->m_schedule.transaction();

  try {
    if (d->m_schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
      KMyMoneyUtils::calculateAutoLoan(d->m_schedule, t, QMap<QString, MyMoneyMoney>());
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedError(this, i18n("Unable to load schedule details"), e.what());
  }

  t.clearId();
  t.setEntryDate(QDate());
  return t;
}

QDate KEnterScheduleDlg::date(const QDate& _date) const
{
  Q_D(const KEnterScheduleDlg);
  auto date(_date);
  return d->m_schedule.adjustedDate(date, d->m_schedule.weekendOption());
}

void KEnterScheduleDlg::resizeEvent(QResizeEvent* ev)
{
  Q_UNUSED(ev)
  Q_D(KEnterScheduleDlg);
  d->ui->m_register->resize(KMyMoneyRegister::DetailColumn);
  d->ui->m_form->resize(KMyMoneyTransactionForm::ValueColumn1);
  QDialog::resizeEvent(ev);
}

void KEnterScheduleDlg::slotSetupSize()
{
  resize(width(), minimumSizeHint().height());
}

int KEnterScheduleDlg::exec()
{
  Q_D(KEnterScheduleDlg);
  if (d->m_showWarningOnce) {
    d->m_showWarningOnce = false;
    KMessageBox::information(this, QString("<qt>") + i18n("<p>Please check that all the details in the following dialog are correct and press OK.</p><p>Editable data can be changed and can either be applied to just this occurrence or for all subsequent occurrences for this schedule.  (You will be asked what you intend after pressing OK in the following dialog)</p>") + QString("</qt>"), i18n("Enter scheduled transaction"), "EnterScheduleDlgInfo");
  }

  // force the initial height to be as small as possible
  QTimer::singleShot(0, this, SLOT(slotSetupSize()));
  return QDialog::exec();
}

TransactionEditor* KEnterScheduleDlg::startEdit()
{
  Q_D(KEnterScheduleDlg);
  KMyMoneyRegister::SelectedTransactions list(d->ui->m_register);
  TransactionEditor* editor = d->m_item->createEditor(d->ui->m_form, list, QDate());
  editor->setScheduleInfo(d->m_schedule.name());
  editor->setPaymentMethod(d->m_schedule.paymentType());

  // check that we use the same transaction commodity in all selected transactions
  // if not, we need to update this in the editor's list. The user can also bail out
  // of this operation which means that we have to stop editing here.
  if (editor) {
    if (!editor->fixTransactionCommodity(d->m_schedule.account())) {
      // if the user wants to quit, we need to destroy the editor
      // and bail out
      delete editor;
      editor = 0;
    }
  }

  if (editor) {
    connect(editor, &TransactionEditor::transactionDataSufficient, d->ui->buttonOk, &QWidget::setEnabled);
    connect(editor, &TransactionEditor::escapePressed, d->ui->buttonCancel, &QAbstractButton::animateClick);
    connect(editor, &TransactionEditor::returnPressed, d->ui->buttonOk, &QAbstractButton::animateClick);

    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, editor, &TransactionEditor::slotReloadEditWidgets);
    // connect(editor, SIGNAL(finishEdit(KMyMoneyRegister::SelectedTransactions)), this, SLOT(slotLeaveEditMode(KMyMoneyRegister::SelectedTransactions)));
    connect(editor, &TransactionEditor::createPayee, kmymoney, static_cast<void (KMyMoneyApp::*)(const QString&, QString&)>(&KMyMoneyApp::slotPayeeNew));
    connect(editor, &TransactionEditor::createTag, kmymoney, static_cast<void (KMyMoneyApp::*)(const QString&, QString&)>(&KMyMoneyApp::slotTagNew));
    connect(editor, &TransactionEditor::createCategory, kmymoney, static_cast<void (KMyMoneyApp::*)(MyMoneyAccount&,const MyMoneyAccount&)>(&KMyMoneyApp::slotCategoryNew));
    connect(editor, &TransactionEditor::createSecurity, kmymoney, static_cast<void (KMyMoneyApp::*)(MyMoneyAccount&,const MyMoneyAccount&)>(&KMyMoneyApp::slotInvestmentNew));
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, editor, &TransactionEditor::slotReloadEditWidgets);

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    KMyMoneyRegister::Action action = KMyMoneyRegister::ActionWithdrawal;
    switch (d->m_schedule.type()) {
      case eMyMoney::Schedule::Type::Transfer:
        action = KMyMoneyRegister::ActionTransfer;
        break;
      case eMyMoney::Schedule::Type::Deposit:
        action = KMyMoneyRegister::ActionDeposit;
        break;
      case eMyMoney::Schedule::Type::LoanPayment:
        switch (d->m_schedule.paymentType()) {
          case eMyMoney::Schedule::PaymentType::DirectDeposit:
          case eMyMoney::Schedule::PaymentType::ManualDeposit:
            action = KMyMoneyRegister::ActionDeposit;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    editor->setup(d->m_tabOrderWidgets, d->m_schedule.account(), action);

    MyMoneyTransaction t = d->m_schedule.transaction();
    QString num = t.splits().first().number();
    QWidget* w = editor->haveWidget("number");
    if (d->m_schedule.paymentType() == eMyMoney::Schedule::PaymentType::WriteChecque) {
      auto file = MyMoneyFile::instance();
      if (file->checkNoUsed(d->m_schedule.account().id(), num)) {
        //  increment and try again
        num = KMyMoneyUtils::getAdjacentNumber(num);
      }
      num = KMyMoneyUtils::nextCheckNumber(d->m_schedule.account());
      KMyMoneyUtils::updateLastNumberUsed(d->m_schedule.account(), num);
      d->m_schedule.account().setValue("lastNumberUsed", num);
      if (w) {
        dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(num);
      }
    } else {
      // if it's not a check, then we need to clear
      // a possibly assigned check number
      if (w)
        dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(QString());
    }

    Q_ASSERT(!d->m_tabOrderWidgets.isEmpty());

    // editor->setup() leaves the tabbar as the last widget in the stack, but we
    // need it as first here. So we move it around.
    w = editor->haveWidget("tabbar");
    if (w) {
      int idx = d->m_tabOrderWidgets.indexOf(w);
      if (idx != -1) {
        d->m_tabOrderWidgets.removeAt(idx);
        d->m_tabOrderWidgets.push_front(w);
      }
    }

    // don't forget our three buttons
    d->m_tabOrderWidgets.append(d->ui->buttonOk);
    d->m_tabOrderWidgets.append(d->ui->buttonCancel);
    d->m_tabOrderWidgets.append(d->ui->buttonHelp);

    for (auto i = 0; i < d->m_tabOrderWidgets.size(); ++i) {
      QWidget* w = d->m_tabOrderWidgets.at(i);
      if (w) {
        w->installEventFilter(this);
        w->installEventFilter(editor);
      }
    }
    // Check if the editor has some preference on where to set the focus
    // If not, set the focus to the first widget in the tab order
    QWidget* focusWidget = editor->firstWidget();
    if (!focusWidget)
      focusWidget = d->m_tabOrderWidgets.first();
    focusWidget->setFocus();

    // Make sure, we use the adjusted date
    kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(editor->haveWidget("postdate"));
    if (dateEdit) {
      dateEdit->setDate(d->m_schedule.adjustedNextDueDate());
    }
  }

  return editor;
}

bool KEnterScheduleDlg::focusNextPrevChild(bool next)
{
  Q_D(KEnterScheduleDlg);
  auto rc = false;
  
  auto w = qApp->focusWidget();
  int currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
  while (w && currentWidgetIndex == -1) {
    // qDebug("'%s' not in list, use parent", w->className());
    w = w->parentWidget();
    currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
  }

  if (currentWidgetIndex != -1) {
    // if(w) qDebug("tab order is at '%s'", w->className());
    currentWidgetIndex += next ? 1 : -1;
    if (currentWidgetIndex < 0)
      currentWidgetIndex = d->m_tabOrderWidgets.size() - 1;
    else if (currentWidgetIndex >= d->m_tabOrderWidgets.size())
      currentWidgetIndex = 0;

    w = d->m_tabOrderWidgets[currentWidgetIndex];
    // qDebug("currentWidgetIndex = %d, w = %p", currentWidgetIndex, w);

    if (((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus) && w->isVisible() && w->isEnabled()) {
      // qDebug("Selecting '%s' as focus", w->className());
      w->setFocus();
      rc = true;
    }
  }
  return rc;
}

void KEnterScheduleDlg::slotShowHelp()
{
  KHelpClient::invokeHelp("details.schedules.entering");
}
