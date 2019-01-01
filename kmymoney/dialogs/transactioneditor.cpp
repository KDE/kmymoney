/*
 * Copyright 2007-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "transactioneditor.h"
#include "transactioneditor_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QList>
#include <QEvent>
#include <QIcon>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytagcombo.h"
#include "knewinvestmentwizard.h"
#include "knewaccountdlg.h"
#include "ktagcontainer.h"
#include "tabbar.h"
#include "mymoneyutils.h"
#include "mymoneyexception.h"
#include "kmymoneycategory.h"
#include "kmymoneymvccombo.h"
#include "kmymoneyedit.h"
#include "kmymoneylineedit.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "kmymoneyutils.h"
#include "kmymoneycompletion.h"
#include "transaction.h"
#include "transactionform.h"
#include "kmymoneysettings.h"
#include "transactioneditorcontainer.h"

#include "kcurrencycalculator.h"
#include "icons.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;
using namespace Icons;

TransactionEditor::TransactionEditor() :
  d_ptr(new TransactionEditorPrivate(this))
{
  Q_D(TransactionEditor);
  d->init();
}

TransactionEditor::TransactionEditor(TransactionEditorPrivate &dd,
                                     TransactionEditorContainer* regForm,
                                     KMyMoneyRegister::Transaction* item,
                                     const KMyMoneyRegister::SelectedTransactions& list,
                                     const QDate& lastPostDate) :
  d_ptr(&dd)
//  d_ptr(new TransactionEditorPrivate)
{
  Q_D(TransactionEditor);
  d->m_paymentMethod = eMyMoney::Schedule::PaymentType::Any;
  d->m_transactions = list;
  d->m_regForm = regForm;
  d->m_item = item;
  d->m_transaction = item->transaction();
  d->m_split = item->split();
  d->m_lastPostDate = lastPostDate;
  d->m_initialAction = eWidgets::eRegister::Action::None;
  d->m_openEditSplits = false;
  d->m_memoChanged = false;
  d->m_item->startEditMode();
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, static_cast<void (TransactionEditor::*)()>(&TransactionEditor::slotUpdateAccount));
}

TransactionEditor::TransactionEditor(TransactionEditorPrivate &dd) :
  d_ptr(&dd)
{
  Q_D(TransactionEditor);
  d->init();
}

TransactionEditor::~TransactionEditor()
{
  Q_D(TransactionEditor);
  // Make sure the widgets do not send out signals to the editor anymore
  // After all, the editor is about to die

  //disconnect first tagCombo:
  auto w = dynamic_cast<KTagContainer*>(haveWidget("tag"));
  if (w && w->tagCombo()) {
    w->tagCombo()->disconnect(this);
  }

  QMap<QString, QWidget*>::iterator it_w;
  for (it_w = d->m_editWidgets.begin(); it_w != d->m_editWidgets.end(); ++it_w) {
    (*it_w)->disconnect(this);
  }

  d->m_regForm->removeEditWidgets(d->m_editWidgets);
  d->m_item->leaveEditMode();
  emit finishEdit(d->m_transactions);
}

void TransactionEditor::slotUpdateAccount(const QString& id)
{
  Q_D(TransactionEditor);
  d->m_account = MyMoneyFile::instance()->account(id);
  setupPrecision();
}

void TransactionEditor::slotUpdateAccount()
{
  Q_D(TransactionEditor);
  // reload m_account as it might have been changed
  d->m_account = MyMoneyFile::instance()->account(d->m_account.id());
  setupPrecision();
}

void TransactionEditor::setupPrecision()
{
  Q_D(TransactionEditor);
  const int prec = (d->m_account.id().isEmpty()) ? 2 : MyMoneyMoney::denomToPrec(d->m_account.fraction());
  QStringList widgets = QString("amount,deposit,payment").split(',');
  QStringList::const_iterator it_w;
  for (it_w = widgets.constBegin(); it_w != widgets.constEnd(); ++it_w) {
    QWidget * w;
    if ((w = haveWidget(*it_w)) != 0) {
      if (auto precisionWidget = dynamic_cast<KMyMoneyEdit*>(w))
        precisionWidget->setPrecision(prec);
    }
  }
}

void TransactionEditor::setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account, eWidgets::eRegister::Action action)
{
  Q_D(TransactionEditor);
  d->m_account = account;
  d->m_initialAction = action;
  createEditWidgets();
  d->m_regForm->arrangeEditWidgets(d->m_editWidgets, d->m_item);
  d->m_regForm->tabOrder(tabOrderWidgets, d->m_item);
  QWidget* w = haveWidget("tabbar");
  if (w) {
    tabOrderWidgets.append(w);
    auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(w);
    if ((tabbar) && (action == eWidgets::eRegister::Action::None)) {
      action = static_cast<eWidgets::eRegister::Action>(tabbar->currentIndex());
    }
  }
  loadEditWidgets(action);

  // remove all unused widgets and don't forget to remove them
  // from the tab order list as well
  d->m_editWidgets.removeOrphans();
  QWidgetList::iterator it_w;
  const QWidgetList editWidgets(d->m_editWidgets.values());
  for (it_w = tabOrderWidgets.begin(); it_w != tabOrderWidgets.end();) {
    if (editWidgets.contains(*it_w)) {
      ++it_w;
    } else {
      // before we remove the widget, we make sure it's not a part of a known one.
      // these could be a direct child in case of KMyMoneyDateInput and KMyMoneyEdit
      // where we store the pointer to the surrounding frame in editWidgets
      // or the parent is called "KMyMoneyCategoryFrame"
      if (*it_w) {
        if (editWidgets.contains((*it_w)->parentWidget())
            || ((*it_w)->parentWidget() && (*it_w)->parentWidget()->objectName() == QLatin1String("KMyMoneyCategoryFrame"))) {
          ++it_w;

        } else {
          // qDebug("Remove '%s' from taborder", qPrintable((*it_w)->objectName()));
          it_w = tabOrderWidgets.erase(it_w);
        }
      } else {
        it_w = tabOrderWidgets.erase(it_w);
      }
    }
  }

  clearFinalWidgets();
  setupFinalWidgets();
  slotUpdateButtonState();
}

void TransactionEditor::setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account)
{
  setup(tabOrderWidgets, account, eWidgets::eRegister::Action::None);
}

MyMoneyAccount TransactionEditor::account() const
{
  Q_D(const TransactionEditor);
  return d->m_account;
}

void TransactionEditor::setScheduleInfo(const QString& si)
{
  Q_D(TransactionEditor);
  d->m_scheduleInfo = si;
}

void TransactionEditor::setPaymentMethod(eMyMoney::Schedule::PaymentType pm)
{
  Q_D(TransactionEditor);
  d->m_paymentMethod = pm;
}

void TransactionEditor::clearFinalWidgets()
{
  Q_D(TransactionEditor);
  d->m_finalEditWidgets.clear();
}

void TransactionEditor::addFinalWidget(const QWidget* w)
{
  Q_D(TransactionEditor);
  if (w) {
    d->m_finalEditWidgets << w;
  }
}

void TransactionEditor::slotReloadEditWidgets()
{
}

bool TransactionEditor::eventFilter(QObject* o, QEvent* e)
{
  Q_D(TransactionEditor);
  bool rc = false;
  if (o == haveWidget("number")) {
    if (e->type() == QEvent::MouseButtonDblClick) {
      emit assignNumber();
      rc = true;
    }
  }

  // if the object is a widget, the event is a key press event and
  // the object is one of our edit widgets, then ....
  auto numberWiget = dynamic_cast<QWidget*>(o);
  if (o->isWidgetType()
      && (e->type() == QEvent::KeyPress)
      && numberWiget && d->m_editWidgets.values().contains(numberWiget)) {
    auto k = dynamic_cast<QKeyEvent*>(e);
    if ((k && (k->modifiers() & Qt::KeyboardModifierMask)) == 0
        || (k && (k->modifiers() & Qt::KeypadModifier)) != 0) {
      bool isFinal = false;
      QList<const QWidget*>::const_iterator it_w;
      switch (k->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          // we check, if the object is one of the m_finalEditWidgets and if it's
          // a KMyMoneyEdit object that the value is not 0. If any of that is the
          // case, it's the final object. In other cases, we convert the enter
          // key into a TAB key to move between the fields. Of course, we only need
          // to do this as long as the appropriate option is set. In all other cases,
          // we treat the return/enter key as such.
          if (KMyMoneySettings::enterMovesBetweenFields()) {
            for (it_w = d->m_finalEditWidgets.constBegin(); !isFinal && it_w != d->m_finalEditWidgets.constEnd(); ++it_w) {
              if (*it_w == o) {
                if (auto widget = dynamic_cast<const KMyMoneyEdit*>(*it_w)) {
                  isFinal = !(widget->value().isZero());
                } else
                  isFinal = true;
              }
            }
          } else
            isFinal = true;

          // for the non-final objects, we treat the return key as a TAB
          if (!isFinal) {
            QKeyEvent evt(e->type(),
                          Qt::Key_Tab, k->modifiers(), QString(),
                          k->isAutoRepeat(), k->count());

            QApplication::sendEvent(o, &evt);
            // in case of a category item and the split button is visible
            // send a second event so that we get passed the button.
            auto widget = dynamic_cast<KMyMoneyCategory*>(o);
            if (widget && widget->splitButton())
              QApplication::sendEvent(o, &evt);

          } else {
            QTimer::singleShot(0, this, SIGNAL(returnPressed()));
          }
          // don't process any further
          rc = true;
          break;

        case Qt::Key_Escape:
          QTimer::singleShot(0, this, SIGNAL(escapePressed()));
          break;
      }
    }
  }
  return rc;
}

void TransactionEditor::slotNumberChanged(const QString& txt)
{
  Q_D(TransactionEditor);
  auto next = txt;
  QString schedInfo;
  if (!d->m_scheduleInfo.isEmpty()) {
    schedInfo = i18n("<center>Processing schedule for %1.</center>", d->m_scheduleInfo);
  }

  while (MyMoneyFile::instance()->checkNoUsed(d->m_account.id(), next)) {
    if (KMessageBox::questionYesNo(d->m_regForm, QString("<qt>") + schedInfo + i18n("<center>Check number <b>%1</b> has already been used in account <b>%2</b>.</center>"
                                   "<center>Do you want to replace it with the next available number?</center>", next, d->m_account.name()) + QString("</qt>"), i18n("Duplicate number")) == KMessageBox::Yes) {
      assignNextNumber();
      next = KMyMoneyUtils::nextCheckNumber(d->m_account);
    } else if (auto number = dynamic_cast<KMyMoneyLineEdit*>(haveWidget("number"))) {
      number->setText(QString());
      break;
    }
  }
}

void TransactionEditor::slotUpdateMemoState()
{
  Q_D(TransactionEditor);
  KTextEdit* memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
  if (memo) {
    d->m_memoChanged = (memo->toPlainText() != d->m_memoText);
  }
}

void TransactionEditor::slotUpdateButtonState()
{
  QString reason;
  emit transactionDataSufficient(isComplete(reason));
}

QWidget* TransactionEditor::haveWidget(const QString& name) const
{
  Q_D(const TransactionEditor);
  return d->m_editWidgets.haveWidget(name);
}

int TransactionEditor::slotEditSplits()
{
  return QDialog::Rejected;
}

void TransactionEditor::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s)
{
  Q_D(TransactionEditor);
  d->m_transaction = t;
  d->m_split = s;
  loadEditWidgets();
}

bool TransactionEditor::isMultiSelection() const
{
  Q_D(const TransactionEditor);
  return d->m_transactions.count() > 1;
}

bool TransactionEditor::fixTransactionCommodity(const MyMoneyAccount& account)
{
  Q_D(TransactionEditor);
  bool rc = true;
  bool firstTimeMultiCurrency = true;
  d->m_account = account;

  auto file = MyMoneyFile::instance();

  // determine the max fraction for this account
  MyMoneySecurity sec = file->security(d->m_account.currencyId());
  int fract = d->m_account.fraction();

  // scan the list of selected transactions
  KMyMoneyRegister::SelectedTransactions::iterator it_t;
  for (it_t = d->m_transactions.begin(); (rc == true) && (it_t != d->m_transactions.end()); ++it_t) {
    // there was a time when the schedule editor did not setup the transaction commodity
    // let's give a helping hand here for those old schedules
    if ((*it_t).transaction().commodity().isEmpty())
      (*it_t).transaction().setCommodity(d->m_account.currencyId());
    // we need to check things only if a different commodity is used
    if (d->m_account.currencyId() != (*it_t).transaction().commodity()) {
      MyMoneySecurity osec = file->security((*it_t).transaction().commodity());
      switch ((*it_t).transaction().splitCount()) {
        case 0:
          // new transaction, guess nothing's here yet ;)
          break;

        case 1:
          try {
            // make sure, that the value is equal to the shares, don't forget our own copy
            MyMoneySplit& splitB = (*it_t).split();  // reference usage wanted here
            if (d->m_split == splitB)
              d->m_split.setValue(splitB.shares());
            splitB.setValue(splitB.shares());
            (*it_t).transaction().modifySplit(splitB);

          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", qPrintable((*it_t).transaction().id()), e.what());
          }
          break;

        case 2:
          // If we deal with multiple currencies we make sure, that for
          // transactions with two splits, the transaction's commodity is the
          // currency of the currently selected account. This saves us from a
          // lot of grieve later on.  We just have to switch the
          // transactions commodity. Let's assume the following scenario:
          // - transactions commodity is CA
          // - splitB and account's currencyId is CB
          // - splitA is of course in CA (otherwise we have a real problem)
          // - Value is V in both splits
          // - Shares in splitB is SB
          // - Shares in splitA is SA (and equal to V)
          //
          // We do the following:
          // - change transactions commodity to CB
          // - set V in both splits to SB
          // - modify the splits in the transaction
          try {
            // retrieve the splits
            MyMoneySplit& splitB = (*it_t).split();  // reference usage wanted here
            MyMoneySplit splitA = (*it_t).transaction().splitByAccount(d->m_account.id(), false);

            // - set V in both splits to SB. Don't forget our own copy
            if (d->m_split == splitB) {
              d->m_split.setValue(splitB.shares());
            }
            splitB.setValue(splitB.shares());
            splitA.setValue(-splitB.shares());
            (*it_t).transaction().modifySplit(splitA);
            (*it_t).transaction().modifySplit(splitB);

          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", qPrintable((*it_t).transaction().id()), e.what());
          }
          break;

        default:
          // TODO: use new logic by adjusting all splits by the price
          // extracted from the selected split. Inform the user that
          // this will happen and allow him to stop the processing (rc = false)

          try {
            QString msg;
            if (firstTimeMultiCurrency) {
              firstTimeMultiCurrency = false;
              if (!isMultiSelection()) {
                msg = i18n("This transaction has more than two splits and is originally based on a different currency (%1). Using this account to modify the transaction may result in rounding errors. Do you want to continue?", osec.name());
              } else {
                msg = i18n("At least one of the selected transactions has more than two splits and is originally based on a different currency (%1). Using this account to modify the transactions may result in rounding errors. Do you want to continue?", osec.name());
              }

              if (KMessageBox::warningContinueCancel(0, QString("<qt>%1</qt>").arg(msg)) == KMessageBox::Cancel) {
                rc = false;
              }
            }

            if (rc == true) {
              MyMoneyMoney price;
              if (!(*it_t).split().shares().isZero() && !(*it_t).split().value().isZero())
                price = (*it_t).split().shares() / (*it_t).split().value();
              MyMoneySplit& mySplit = (*it_t).split();
              foreach (const auto split, (*it_t).transaction().splits()) {
                auto s = split;
                if (s == mySplit) {
                  s.setValue(s.shares());
                  if (mySplit == d->m_split) {
                    d->m_split = s;
                  }
                  mySplit = s;
                } else {
                  s.setValue((s.value() * price).convert(fract));
                }
                (*it_t).transaction().modifySplit(s);
              }
            }
          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity of split currency in %s: '%s'", qPrintable((*it_t).transaction().id()), e.what());
          }
          break;
      }

      // set the transaction's ommodity to this account's currency
      (*it_t).transaction().setCommodity(d->m_account.currencyId());

      // update our copy of the transaction that has the focus
      if ((*it_t).transaction().id() == d->m_transaction.id()) {
        d->m_transaction = (*it_t).transaction();
      }
    }
  }
  return rc;
}

void TransactionEditor::assignNextNumber()
{
  Q_D(TransactionEditor);
  if (canAssignNumber()) {
    auto number = dynamic_cast<KMyMoneyLineEdit*>(haveWidget("number"));
    QString num = KMyMoneyUtils::nextCheckNumber(d->m_account);
    bool showMessage = true;
    int rc = KMessageBox::No;
    QString schedInfo;
    if (!d->m_scheduleInfo.isEmpty()) {
      schedInfo = i18n("<center>Processing schedule for %1.</center>", d->m_scheduleInfo);
    }
    while (MyMoneyFile::instance()->checkNoUsed(d->m_account.id(), num)) {
      if (showMessage) {
        rc = KMessageBox::questionYesNo(d->m_regForm, QString("<qt>") + schedInfo + i18n("Check number <b>%1</b> has already been used in account <b>%2</b>."
                                        "<center>Do you want to replace it with the next available number?</center>", num, d->m_account.name()) + QString("</qt>"), i18n("Duplicate number"));
        showMessage = false;
      }
      if (rc == KMessageBox::Yes) {
        num = KMyMoneyUtils::nextCheckNumber(d->m_account);
        KMyMoneyUtils::updateLastNumberUsed(d->m_account, num);
        d->m_account.setValue("lastNumberUsed", num);
        if (number)
          number->loadText(num);
      } else {
        num = QString();
        break;
      }
    }
    if (number)
      number->setText(num);
  }
}

bool TransactionEditor::canAssignNumber() const
{
  if (dynamic_cast<KMyMoneyLineEdit*>(haveWidget("number")))
    return true;
  return false;
}

void TransactionEditor::setupCategoryWidget(KMyMoneyCategory* category, const QList<MyMoneySplit>& splits, QString& categoryId, const char* splitEditSlot, bool /* allowObjectCreation */)
{
  disconnect(category, SIGNAL(focusIn()), this, splitEditSlot);
#if 0
  // FIXME must deal with the logic that suppressObjectCreation is
  // automatically turned off when the createItem() signal is connected
  if (allowObjectCreation)
    category->setSuppressObjectCreation(false);
#endif

  switch (splits.count()) {
    case 0:
      categoryId.clear();
      if (!category->currentText().isEmpty()) {
        //   category->clearEditText();  //  don't clear as could be from another widget - Bug 322768
        // make sure, we don't see the selector
        category->completion()->hide();
      }
      category->completion()->setSelected(QString());
      break;

    case 1:
      categoryId = splits[0].accountId();
      category->completion()->setSelected(categoryId);
      category->slotItemSelected(categoryId);
      break;

    default:
      categoryId.clear();
      category->setSplitTransaction();
      connect(category, SIGNAL(focusIn()), this, splitEditSlot);
#if 0
      // FIXME must deal with the logic that suppressObjectCreation is
      // automatically turned off when the createItem() signal is connected
      if (allowObjectCreation)
        category->setSuppressObjectCreation(true);
#endif
      break;
  }
}

bool TransactionEditor::createNewTransaction() const
{
  Q_D(const TransactionEditor);

  bool rc = true;
  if (!d->m_transactions.isEmpty()) {
    rc = d->m_transactions.at(0).transaction().id().isEmpty();
  }
  return rc;
}

bool TransactionEditor::enterTransactions(QString& newId, bool askForSchedule, bool suppressBalanceWarnings)
{
  Q_D(TransactionEditor);
  newId.clear();
  auto file = MyMoneyFile::instance();

  // make sure to run through all stuff that is tied to 'focusout events'.
  d->m_regForm->parentWidget()->setFocus();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);
  // we don't need to update our widgets anymore, so we just disconnect the signal
  disconnect(file, &MyMoneyFile::dataChanged, this, &TransactionEditor::slotReloadEditWidgets);

  KMyMoneyRegister::SelectedTransactions::iterator it_t;
  MyMoneyTransaction t;
  bool newTransactionCreated = false;

  // make sure, that only a single new transaction can be created.
  // we need to update m_transactions to contain the new transaction
  // which is then stored in the variable t when we leave the loop.
  // m_transactions will be sent out in finishEdit() and forces
  // the new transaction to be selected in the ledger view

  // collect the transactions to be stored in the engine in a local
  // list first, so that the user has a chance to interrupt the storage
  // process
  QList<MyMoneyTransaction> list;
  auto storeTransactions = true;

  // collect transactions
  for (it_t = d->m_transactions.begin(); storeTransactions && !newTransactionCreated && it_t != d->m_transactions.end(); ++it_t) {
    storeTransactions = createTransaction(t, (*it_t).transaction(), (*it_t).split());
    // if the transaction was created successfully, append it to the list
    if (storeTransactions)
      list.append(t);

    // if we created a new transaction keep that in mind
    if (t.id().isEmpty())
      newTransactionCreated = true;
  }

  // if not interrupted by user, continue to store them in the engine
  if (storeTransactions) {
    auto i = 0;
    emit statusMsg(i18n("Storing transactions"));
    emit statusProgress(0, list.count());

    MyMoneyFileTransaction ft;

    try {
      QMap<QString, bool> minBalanceEarly;
      QMap<QString, bool> minBalanceAbsolute;
      QMap<QString, bool> maxCreditEarly;
      QMap<QString, bool> maxCreditAbsolute;
      QMap<QString, bool> accountIds;

      for (MyMoneyTransaction& transaction : list) {
        // if we have a categorization, make sure we remove
        // the 'imported' flag automagically
        if (transaction.splitCount() > 1)
          transaction.setImported(false);

        // create information about min and max balances
        foreach (const auto split, transaction.splits()) {
          auto acc = file->account(split.accountId());
          accountIds[acc.id()] = true;
          MyMoneyMoney balance = file->balance(acc.id());
          if (!acc.value("minBalanceEarly").isEmpty()) {
            minBalanceEarly[acc.id()] = balance < MyMoneyMoney(acc.value("minBalanceEarly"));
          }
          if (!acc.value("minBalanceAbsolute").isEmpty()) {
            minBalanceAbsolute[acc.id()] = balance < MyMoneyMoney(acc.value("minBalanceAbsolute"));
            minBalanceEarly[acc.id()] = false;
          }
          if (!acc.value("maxCreditEarly").isEmpty()) {
            maxCreditEarly[acc.id()] = balance < MyMoneyMoney(acc.value("maxCreditEarly"));
          }
          if (!acc.value("maxCreditAbsolute").isEmpty()) {
            maxCreditAbsolute[acc.id()] = balance < MyMoneyMoney(acc.value("maxCreditAbsolute"));
            maxCreditEarly[acc.id()] = false;
          }
        }

        if (transaction.id().isEmpty()) {
          bool enter = true;
          if (askForSchedule && transaction.postDate() > QDate::currentDate()) {
            KGuiItem enterButton(i18n("&Enter"),
                                 Icons::get(Icon::DialogOK),
                                 i18n("Accepts the entered data and stores it"),
                                 i18n("Use this to enter the transaction into the ledger."));
            KGuiItem scheduleButton(i18n("&Schedule"),
                                    Icons::get(Icon::AppointmentNew),
                                    i18n("Accepts the entered data and stores it as schedule"),
                                    i18n("Use this to schedule the transaction for later entry into the ledger."));

            enter = KMessageBox::questionYesNo(d->m_regForm, QString("<qt>%1</qt>").arg(i18n("The transaction you are about to enter has a post date in the future.<br/><br/>Do you want to enter it in the ledger or add it to the schedules?")), i18nc("Dialog caption for 'Enter or schedule' dialog", "Enter or schedule?"), enterButton, scheduleButton, "EnterOrScheduleTransactionInFuture") == KMessageBox::Yes;
          }
          if (enter) {
            // add new transaction
            file->addTransaction(transaction);
            // pass the newly assigned id on to the caller
            newId = transaction.id();
            // refresh account object for transactional changes
            // refresh account and transaction object because they might have changed
            d->m_account = file->account(d->m_account.id());
            t = transaction;

            // if a new transaction has a valid number, keep it with the account
            d->keepNewNumber(transaction);
          } else {
            // turn object creation on, so that moving the focus does
            // not screw up the dialog that might be popping up
            emit objectCreation(true);
            emit scheduleTransaction(transaction, eMyMoney::Schedule::Occurrence::Once);
            emit objectCreation(false);

            newTransactionCreated = false;
          }

          // send out the post date of this transaction
          emit lastPostDateUsed(transaction.postDate());
        } else {
          // modify existing transaction
          // its number might have been edited
          // bearing in mind it could contain alpha characters
          d->keepNewNumber(transaction);
          file->modifyTransaction(transaction);
        }
      }
      emit statusProgress(i++, 0);

      // update m_transactions to contain the newly created transaction so that
      // it is selected as the current one
      // we need to do that before we commit the transaction to the engine
      // as we need it during the update of the views that is caused by committing already.
      if (newTransactionCreated) {
        d->m_transactions.clear();
        MyMoneySplit s;
        // a transaction w/o a single split should not exist and adding it
        // should throw an exception in MyMoneyFile::addTransaction, but we
        // remain on the save side of things to check for it
        if (t.splitCount() > 0)
          s = t.splits().front();
        KMyMoneyRegister::SelectedTransaction st(t, s, QString());
        d->m_transactions.append(st);
      }

      //    Save pricing information
      foreach (const auto split, t.splits()) {
        if ((split.action() != "Buy") &&
            (split.action() != "Reinvest")) {
          continue;
        }
        QString id = split.accountId();
        auto acc = file->account(id);
        MyMoneySecurity sec = file->security(acc.currencyId());
        MyMoneyPrice price(acc.currencyId(),
                           sec.tradingCurrency(),
                           t.postDate(),
                           split.price(), "Transaction");
        file->addPrice(price);
        break;
      }

      ft.commit();

      // now analyze the balances and spit out warnings to the user
      QMap<QString, bool>::const_iterator it_a;

      if (!suppressBalanceWarnings) {
        for (it_a = accountIds.constBegin(); it_a != accountIds.constEnd(); ++it_a) {
          QString msg;
          auto acc = file->account(it_a.key());
          MyMoneyMoney balance = file->balance(acc.id());
          const MyMoneySecurity& sec = file->security(acc.currencyId());
          QString key;
          key = "minBalanceEarly";
          if (!acc.value(key).isEmpty()) {
            if (minBalanceEarly[acc.id()] == false && balance < MyMoneyMoney(acc.value(key))) {
              msg = QString("<qt>%1</qt>").arg(i18n("The balance of account <b>%1</b> dropped below the warning balance of %2.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(acc.value(key)), acc, sec)));
            }
          }
          key = "minBalanceAbsolute";
          if (!acc.value(key).isEmpty()) {
            if (minBalanceAbsolute[acc.id()] == false && balance < MyMoneyMoney(acc.value(key))) {
              msg = QString("<qt>%1</qt>").arg(i18n("The balance of account <b>%1</b> dropped below the minimum balance of %2.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(acc.value(key)), acc, sec)));
            }
          }
          key = "maxCreditEarly";
          if (!acc.value(key).isEmpty()) {
            if (maxCreditEarly[acc.id()] == false && balance < MyMoneyMoney(acc.value(key))) {
              msg = QString("<qt>%1</qt>").arg(i18n("The balance of account <b>%1</b> dropped below the maximum credit warning limit of %2.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(acc.value(key)), acc, sec)));
            }
          }
          key = "maxCreditAbsolute";
          if (!acc.value(key).isEmpty()) {
            if (maxCreditAbsolute[acc.id()] == false && balance < MyMoneyMoney(acc.value(key))) {
              msg = QString("<qt>%1</qt>").arg(i18n("The balance of account <b>%1</b> dropped below the maximum credit limit of %2.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(acc.value(key)), acc, sec)));
            }
          }

          if (!msg.isEmpty()) {
            emit balanceWarning(d->m_regForm, acc, msg);
          }
        }
      }
    } catch (const MyMoneyException &e) {
      qDebug("Unable to store transaction within engine: %s", e.what());
    }

    emit statusProgress(-1, -1);
    emit statusMsg(QString());

  }
  return storeTransactions;
}

void TransactionEditor::resizeForm()
{
  Q_D(TransactionEditor);
  // force resizeing of the columns in the form

  if (auto form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(d->m_regForm))
    QMetaObject::invokeMethod(form, "resize", Qt::QueuedConnection, QGenericReturnArgument(), Q_ARG(int, (int)eWidgets::eTransactionForm::Column::Value1));
}

void TransactionEditor::slotNewPayee(const QString& newnameBase, QString& id)
{
  KMyMoneyUtils::newPayee(newnameBase, id);
}

void TransactionEditor::slotNewTag(const QString& newnameBase, QString& id)
{
  KMyMoneyUtils::newTag(newnameBase, id);
}

void TransactionEditor::slotNewCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewAccountDlg::newCategory(account, parent);
}

void TransactionEditor::slotNewInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewInvestmentWizard::newInvestment(account, parent);
}
