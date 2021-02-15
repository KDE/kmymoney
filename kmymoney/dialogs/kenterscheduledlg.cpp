/*
 * SPDX-FileCopyrightText: 2007-2012 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kenterscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QWindow>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>
#include <KLocalizedString>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kenterscheduledlg.h"

#include "tabbar.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "register.h"
#include "transactionform.h"
#include "transaction.h"
#include "selectedtransactions.h"
#include "transactioneditor.h"
#include "kmymoneyutils.h"
#include "kmymoneylineedit.h"
#include "kmymoneydateinput.h"
#include "knewaccountdlg.h"
#include "knewinvestmentwizard.h"
#include "mymoneyexception.h"
#include "icons/icons.h"
#include "mymoneyenums.h"
#include "dialogenums.h"
#include "widgetenums.h"

using namespace Icons;

class KEnterScheduleDlgPrivate
{
  Q_DISABLE_COPY(KEnterScheduleDlgPrivate)

public:
  KEnterScheduleDlgPrivate() :
    ui(new Ui::KEnterScheduleDlg),
    m_item(nullptr),
    m_showWarningOnce(true),
    m_extendedReturnCode(eDialogs::ScheduleResultCode::Cancel)
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

  // restore the last used dialog size
  KConfigGroup grp = KSharedConfig::openConfig()->group("KEnterScheduleDlg");
  if (grp.isValid()) {
    KWindowConfig::restoreWindowSize(windowHandle(), grp);
  }
  // let the minimum size be 780x410
  resize(QSize(780, 410).expandedTo(windowHandle() ? windowHandle()->size() : QSize()));

  // position the dialog centered on the application (for some reason without
  // a call to winId() the dialog is positioned in the upper left corner of
  // the screen, but winId() crashes on MS-Windows ...
  if (parent)
    move(parent->pos() + QPoint(parent->width()/2, parent->height()/2) - QPoint(width()/2, height()/2));

  d->ui->setupUi(this);
  d->m_schedule = schedule;
  d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Enter;
  d->ui->buttonOk->setIcon(Icons::get(Icon::KeyEnter));
  d->ui->buttonSkip->setIcon(Icons::get(Icon::SeekForward));
  KGuiItem::assign(d->ui->buttonCancel, KStandardGuiItem::cancel());
  KGuiItem::assign(d->ui->buttonHelp, KStandardGuiItem::help());
  d->ui->buttonIgnore->setHidden(true);
  d->ui->buttonSkip->setHidden(true);

  // make sure, we have a tabbar with the form
  KMyMoneyTransactionForm::TabBar* tabbar = d->ui->m_form->getTabBar(d->ui->m_form->parentWidget());

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

  // store the last used dialog size
  KConfigGroup grp = KSharedConfig::openConfig()->group("KEnterScheduleDlg");
  if (grp.isValid()) {
    KWindowConfig::saveWindowSize(windowHandle(), grp);
  }

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
    KMessageBox::detailedError(this, i18n("Unable to load schedule details"), QString::fromLatin1(e.what()));
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
  d->ui->m_register->resize((int)eWidgets::eTransaction::Column::Detail);
  d->ui->m_form->resize((int)eWidgets::eTransactionForm::Column::Value1);
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
    KMessageBox::information(parentWidget(), QString("<qt>") + i18n("<p>Please check that all the details in the following dialog are correct and press OK.</p><p>Editable data can be changed and can either be applied to just this occurrence or for all subsequent occurrences for this schedule.  (You will be asked what you intend after pressing OK in the following dialog)</p>") + QString("</qt>"), i18n("Enter scheduled transaction"), "EnterScheduleDlgInfo");
  }

  // force the initial height to be as small as possible
  QTimer::singleShot(0, this, SLOT(slotSetupSize()));
  return QDialog::exec();
}

TransactionEditor* KEnterScheduleDlg::startEdit()
{
  Q_D(KEnterScheduleDlg);
  KMyMoneyRegister::SelectedTransactions list(d->ui->m_register);
  auto editor = d->m_item->createEditor(d->ui->m_form, list, QDate());
  if (editor) {
    editor->setScheduleInfo(d->m_schedule.name());
    editor->setPaymentMethod(d->m_schedule.paymentType());
  }

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

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    eWidgets::eRegister::Action action = eWidgets::eRegister::Action::Withdrawal;
    switch (d->m_schedule.type()) {
      case eMyMoney::Schedule::Type::Transfer:
        action = eWidgets::eRegister::Action::Transfer;
        break;
      case eMyMoney::Schedule::Type::Deposit:
        action = eWidgets::eRegister::Action::Deposit;
        break;
      case eMyMoney::Schedule::Type::LoanPayment:
        switch (d->m_schedule.paymentType()) {
          case eMyMoney::Schedule::PaymentType::DirectDeposit:
          case eMyMoney::Schedule::PaymentType::ManualDeposit:
            action = eWidgets::eRegister::Action::Deposit;
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
      num = KMyMoneyUtils::nextFreeCheckNumber(d->m_schedule.account());
      d->m_schedule.account().setValue("lastNumberUsed", num);
      if (w)
        if (auto numberWidget = dynamic_cast<KMyMoneyLineEdit*>(w))
          numberWidget->loadText(num);
    } else {
      // if it's not a check, then we need to clear
      // a possibly assigned check number
      if (w)
        if (auto numberWidget = dynamic_cast<KMyMoneyLineEdit*>(w))
          numberWidget->loadText(QString());
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
      w = d->m_tabOrderWidgets.at(i);
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
    if (auto dateEdit = dynamic_cast<KMyMoneyDateInput*>(editor->haveWidget("postdate")))
      dateEdit->setDate(d->m_schedule.adjustedNextDueDate());
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
      w->setFocus(next ? Qt::TabFocusReason: Qt::BacktabFocusReason);
      rc = true;
    }
  }
  return rc;
}

void KEnterScheduleDlg::slotShowHelp()
{
  KHelpClient::invokeHelp("details.schedules.entering");
}
