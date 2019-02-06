/***************************************************************************
                             transactioneditor.cpp
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

#include "transactioneditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QApplication>
#include <QEventLoop>
#include <QRadioButton>
#include <QKeyEvent>
#include <QList>
#include <QEvent>
#include <QToolTip>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <ktextedit.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneycategory.h>
#include "kmymoneymvccombo.h"
#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountcompletion.h>
#include <kmymoneyaccountselector.h>
#include <mymoneyfile.h>
#include <kmymoneyutils.h>
#include <transactionform.h>
#include <kmymoneyglobalsettings.h>

#include "ksplittransactiondlg.h"
#include "kcurrencycalculator.h"
#include "kselecttransactionsdlg.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

TransactionEditor::TransactionEditor() :
    m_paymentMethod(MyMoneySchedule::STYPE_ANY),
    m_regForm(0),
    m_item(0),
    m_initialAction(ActionNone),
    m_openEditSplits(false),
    m_memoChanged(false)
{
}

TransactionEditor::TransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) :
    m_paymentMethod(MyMoneySchedule::STYPE_ANY),
    m_transactions(list),
    m_regForm(regForm),
    m_item(item),
    m_transaction(item->transaction()),
    m_split(item->split()),
    m_lastPostDate(lastPostDate),
    m_initialAction(ActionNone),
    m_openEditSplits(false),
    m_memoChanged(false)
{
  m_item->startEditMode();
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotUpdateAccount()));
}

TransactionEditor::~TransactionEditor()
{
  // Make sure the widgets do not send out signals to the editor anymore
  // After all, the editor is about to die

  //disconnect first tagCombo:
  KTagContainer *w = dynamic_cast<KTagContainer*>(haveWidget("tag"));
  if (w && w->tagCombo()) {
    w->tagCombo()->disconnect(this);
  }

  QMap<QString, QWidget*>::iterator it_w;
  for (it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    (*it_w)->disconnect(this);
  }

  m_regForm->removeEditWidgets(m_editWidgets);
  m_item->leaveEditMode();
  emit finishEdit(m_transactions);
}

void TransactionEditor::slotUpdateAccount(const QString& id)
{
  m_account = MyMoneyFile::instance()->account(id);
  setupPrecision();
}

void TransactionEditor::slotUpdateAccount()
{
  // reload m_account as it might have been changed
  m_account = MyMoneyFile::instance()->account(m_account.id());
  setupPrecision();
}

void TransactionEditor::setupPrecision()
{
  const int prec = (m_account.id().isEmpty()) ? 2 : MyMoneyMoney::denomToPrec(m_account.fraction());
  QStringList widgets = QString("amount,deposit,payment").split(',');
  QStringList::const_iterator it_w;
  for (it_w = widgets.constBegin(); it_w != widgets.constEnd(); ++it_w) {
    QWidget * w;
    if ((w = haveWidget(*it_w)) != 0) {
      dynamic_cast<kMyMoneyEdit*>(w)->setPrecision(prec);
    }
  }
}

void TransactionEditor::setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account, KMyMoneyRegister::Action action)
{
  m_account = account;
  m_initialAction = action;
  createEditWidgets();
  m_regForm->arrangeEditWidgets(m_editWidgets, m_item);
  m_regForm->tabOrder(tabOrderWidgets, m_item);
  QWidget* w = haveWidget("tabbar");
  if (w) {
    tabOrderWidgets.append(w);
    TabBar* tabbar = dynamic_cast<TabBar*>(w);
    if ((tabbar) && (action == KMyMoneyRegister::ActionNone)) {
      action = static_cast<KMyMoneyRegister::Action>(tabbar->currentIndex());
    }
  }
  loadEditWidgets(action);

  // remove all unused widgets and don't forget to remove them
  // from the tab order list as well
  m_editWidgets.removeOrphans();
  QWidgetList::iterator it_w;
  const QWidgetList editWidgets(m_editWidgets.values());
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

void TransactionEditor::clearFinalWidgets()
{
  m_finalEditWidgets.clear();
}

void TransactionEditor::addFinalWidget(const QWidget* w)
{
  if (w) {
    m_finalEditWidgets << w;
  }
}

void TransactionEditor::slotReloadEditWidgets()
{
}

bool TransactionEditor::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;
  if (o == haveWidget("number")) {
    if (e->type() == QEvent::MouseButtonDblClick) {
      emit assignNumber();
      rc = true;
    }
  }

  // if the object is a widget, the event is a key press event and
  // the object is one of our edit widgets, then ....
  if (o->isWidgetType()
      && (e->type() == QEvent::KeyPress)
      && m_editWidgets.values().contains(dynamic_cast<QWidget*>(o))) {
    QKeyEvent* k = dynamic_cast<QKeyEvent*>(e);
    if ((k->modifiers() & Qt::KeyboardModifierMask) == 0
        || (k->modifiers() & Qt::KeypadModifier) != 0) {
      bool isFinal = false;
      QList<const QWidget*>::const_iterator it_w;
      switch (k->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          // we check, if the object is one of the m_finalEditWidgets and if it's
          // a kMyMoneyEdit object that the value is not 0. If any of that is the
          // case, it's the final object. In other cases, we convert the enter
          // key into a TAB key to move between the fields. Of course, we only need
          // to do this as long as the appropriate option is set. In all other cases,
          // we treat the return/enter key as such.
          if (KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
            for (it_w = m_finalEditWidgets.constBegin(); !isFinal && it_w != m_finalEditWidgets.constEnd(); ++it_w) {
              if (*it_w == o) {
                if (dynamic_cast<const kMyMoneyEdit*>(*it_w)) {
                  isFinal = !(dynamic_cast<const kMyMoneyEdit*>(*it_w)->value().isZero());
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
            if (dynamic_cast<KMyMoneyCategory*>(o) && dynamic_cast<KMyMoneyCategory*>(o)->splitButton())
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
  QString next = txt;
  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  QString schedInfo;
  if (!m_scheduleInfo.isEmpty()) {
    schedInfo = i18n("<center>Processing schedule for %1.</center>", m_scheduleInfo);
  }

  while (MyMoneyFile::instance()->checkNoUsed(m_account.id(), next)) {
    QString dontShowAgain = QLatin1String("DuplicatedNumber");
    KMessageBox::ButtonCode result;
    if ((!KMessageBox::shouldBeShownYesNo(dontShowAgain, result) && result == KMessageBox::Yes) ||
      KMessageBox::questionYesNo(m_regForm, QLatin1String("<qt>") + schedInfo + i18n("<center>Check number <b>%1</b> has already been used in account <b>%2</b>.</center>"
                                 "<center>Do you want to replace it with the next available number?</center>", next, m_account.name()) + QLatin1String("</qt>"), i18n("Duplicate number"),
                                 KStandardGuiItem::yes(), KStandardGuiItem::no(), dontShowAgain) == KMessageBox::Yes) {
      assignNextNumber();
      next = KMyMoneyUtils::nextCheckNumber(m_account);
    } else {
      number->loadText(txt);
      break;
    }
  }
}

void TransactionEditor::slotUpdateMemoState()
{
  KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
  if (memo) {
    m_memoChanged = (memo->toPlainText() != m_memoText);
  }
}

void TransactionEditor::slotUpdateButtonState()
{
  QString reason;
  emit transactionDataSufficient(isComplete(reason));
}

QWidget* TransactionEditor::haveWidget(const QString& name) const
{
  return m_editWidgets.haveWidget(name);
}

int TransactionEditor::slotEditSplits()
{
  return KDialog::Rejected;
}

void TransactionEditor::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s)
{
  m_transaction = t;
  m_split = s;
  loadEditWidgets();
}

bool TransactionEditor::fixTransactionCommodity(const MyMoneyAccount& account)
{
  bool rc = true;
  bool firstTimeMultiCurrency = true;
  m_account = account;

  MyMoneyFile* file = MyMoneyFile::instance();

  // determine the max fraction for this account
  MyMoneySecurity sec = file->security(m_account.currencyId());
  int fract = m_account.fraction();

  // scan the list of selected transactions
  KMyMoneyRegister::SelectedTransactions::iterator it_t;
  for (it_t = m_transactions.begin(); (rc == true) && (it_t != m_transactions.end()); ++it_t) {
    // there was a time when the schedule editor did not setup the transaction commodity
    // let's give a helping hand here for those old schedules
    if ((*it_t).transaction().commodity().isEmpty())
      (*it_t).transaction().setCommodity(m_account.currencyId());
    // we need to check things only if a different commodity is used
    if (m_account.currencyId() != (*it_t).transaction().commodity()) {
      MyMoneySecurity osec = file->security((*it_t).transaction().commodity());
      switch ((*it_t).transaction().splitCount()) {
        case 0:
          // new transaction, guess nothing's here yet ;)
          break;

        case 1:
          try {
            // make sure, that the value is equal to the shares, don't forget our own copy
            MyMoneySplit& splitB = (*it_t).split();  // reference usage wanted here
            if (m_split == splitB)
              m_split.setValue(splitB.shares());
            splitB.setValue(splitB.shares());
            (*it_t).transaction().modifySplit(splitB);

          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", qPrintable((*it_t).transaction().id()), qPrintable(e.what()));
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
            MyMoneySplit splitA = (*it_t).transaction().splitByAccount(m_account.id(), false);

            // - set V in both splits to SB. Don't forget our own copy
            if (m_split == splitB) {
              m_split.setValue(splitB.shares());
            }
            splitB.setValue(splitB.shares());
            splitA.setValue(-splitB.shares());
            (*it_t).transaction().modifySplit(splitA);
            (*it_t).transaction().modifySplit(splitB);

          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", qPrintable((*it_t).transaction().id()), qPrintable(e.what()));
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

              if (KMessageBox::warningContinueCancel(nullptr, QString("<qt>%1</qt>").arg(msg)) == KMessageBox::Cancel) {
                rc = false;
              }
            }

            if (rc == true) {
              MyMoneyMoney price;
              if (!(*it_t).split().shares().isZero() && !(*it_t).split().value().isZero())
                price = (*it_t).split().shares() / (*it_t).split().value();
              QList<MyMoneySplit>::iterator it_s;
              MyMoneySplit& mySplit = (*it_t).split();
              for (it_s = (*it_t).transaction().splits().begin(); it_s != (*it_t).transaction().splits().end(); ++it_s) {
                MyMoneySplit s = (*it_s);
                if (s == mySplit) {
                  s.setValue(s.shares());
                  if (mySplit == m_split) {
                    m_split = s;
                  }
                  mySplit = s;
                } else {
                  s.setValue((s.value() * price).convert(fract));
                }
                (*it_t).transaction().modifySplit(s);
              }
            }
          } catch (const MyMoneyException &e) {
            qDebug("Unable to update commodity of split currency in %s: '%s'", qPrintable((*it_t).transaction().id()), qPrintable(e.what()));
          }
          break;
      }

      // set the transaction's ommodity to this account's currency
      (*it_t).transaction().setCommodity(m_account.currencyId());

      // update our copy of the transaction that has the focus
      if ((*it_t).transaction().id() == m_transaction.id()) {
        m_transaction = (*it_t).transaction();
      }
    }
  }
  return rc;
}

void TransactionEditor::assignNextNumber()
{
  if (canAssignNumber()) {
    kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
    QString num = KMyMoneyUtils::nextCheckNumber(m_account);
    bool showMessage = true;
    int rc = KMessageBox::No;
    QString schedInfo;
    if (!m_scheduleInfo.isEmpty()) {
      schedInfo = i18n("<center>Processing schedule for %1.</center>", m_scheduleInfo);
    }
    while (MyMoneyFile::instance()->checkNoUsed(m_account.id(), num)) {
      if (showMessage) {
        rc = KMessageBox::questionYesNo(m_regForm, QString("<qt>") + schedInfo + i18n("Check number <b>%1</b> has already been used in account <b>%2</b>."
                                        "<center>Do you want to replace it with the next available number?</center>", num, m_account.name()) + QString("</qt>"), i18n("Duplicate number"));
        showMessage = false;
      }
      if (rc == KMessageBox::Yes) {
        num = KMyMoneyUtils::nextCheckNumber(m_account);
        KMyMoneyUtils::updateLastNumberUsed(m_account, num);
        m_account.setValue("lastNumberUsed", num);
        number->loadText(num);
      } else {
        number->loadText(num);
        break;
      }
    }
    number->setText(num);
  }
}

bool TransactionEditor::canAssignNumber() const
{
  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  return (number != 0);
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

bool TransactionEditor::enterTransactions(QString& newId, bool askForSchedule, bool suppressBalanceWarnings)
{
  newId.clear();
  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure to run through all stuff that is tied to 'focusout events'.
  m_regForm->parentWidget()->setFocus();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);
  // we don't need to update our widgets anymore, so we just disconnect the signal
  disconnect(file, SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

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
  bool storeTransactions = true;

  // collect transactions
  for (it_t = m_transactions.begin(); storeTransactions && !newTransactionCreated && it_t != m_transactions.end(); ++it_t) {
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
    int i = 0;
    emit statusMsg(i18n("Storing transactions"));
    emit statusProgress(0, list.count());

    MyMoneyFileTransaction ft;

    try {
      QList<MyMoneyTransaction>::iterator it_ts;
      QMap<QString, bool> minBalanceEarly;
      QMap<QString, bool> minBalanceAbsolute;
      QMap<QString, bool> maxCreditEarly;
      QMap<QString, bool> maxCreditAbsolute;
      QMap<QString, bool> accountIds;

      for (it_ts = list.begin(); it_ts != list.end(); ++it_ts) {
        // if we have a categorization, make sure we remove
        // the 'imported' flag automagically
        if ((*it_ts).splitCount() > 1)
          (*it_ts).setImported(false);

        // create information about min and max balances
        QList<MyMoneySplit>::const_iterator it_s;
        for (it_s = (*it_ts).splits().constBegin(); it_s != (*it_ts).splits().constEnd(); ++it_s) {
          MyMoneyAccount acc = file->account((*it_s).accountId());
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

        if ((*it_ts).id().isEmpty()) {
          bool enter = true;
          if (askForSchedule && (*it_ts).postDate() > QDate::currentDate()) {
            KGuiItem enterItem;
            KGuiItem enterButton(i18n("&Enter"),
                                 KIcon("dialog-ok"),
                                 i18n("Accepts the entered data and stores it"),
                                 i18n("Use this to enter the transaction into the ledger."));
            KGuiItem scheduleButton(i18n("&Schedule"),
                                    KIcon("appointment-new"),
                                    i18n("Accepts the entered data and stores it as schedule"),
                                    i18n("Use this to schedule the transaction for later entry into the ledger."));

            enter = KMessageBox::questionYesNo(m_regForm, QString("<qt>%1</qt>").arg(i18n("The transaction you are about to enter has a post date in the future.<br/><br/>Do you want to enter it in the ledger or add it to the schedules?")), i18nc("Dialog caption for 'Enter or schedule' dialog", "Enter or schedule?"), enterButton, scheduleButton, "EnterOrScheduleTransactionInFuture") == KMessageBox::Yes;
          }
          if (enter) {
            // add new transaction
            file->addTransaction(*it_ts);
            // pass the newly assigned id on to the caller
            newId = (*it_ts).id();
            // refresh account object for transactional changes
            // refresh account and transaction object because they might have changed
            m_account = file->account(m_account.id());
            t = (*it_ts);

            // if a new transaction has a valid number, keep it with the account
            keepNewNumber((*it_ts));
          } else {
            // turn object creation on, so that moving the focus does
            // not screw up the dialog that might be popping up
            emit objectCreation(true);
            emit scheduleTransaction(*it_ts, MyMoneySchedule::OCCUR_ONCE);
            emit objectCreation(false);

            newTransactionCreated = false;
          }

          // send out the post date of this transaction
          emit lastPostDateUsed((*it_ts).postDate());
        } else {
          // modify existing transaction
          // its number might have been edited
          // bearing in mind it could contain alpha characters
          keepNewNumber((*it_ts));
          file->modifyTransaction(*it_ts);
        }
      }
      emit statusProgress(i++, 0);

      // update m_transactions to contain the newly created transaction so that
      // it is selected as the current one
      // we need to do that before we commit the transaction to the engine
      // as we need it during the update of the views that is caused by committing already.
      if (newTransactionCreated) {
        m_transactions.clear();
        MyMoneySplit s;
        // a transaction w/o a single split should not exist and adding it
        // should throw an exception in MyMoneyFile::addTransaction, but we
        // remain on the save side of things to check for it
        if (t.splitCount() > 0)
          s = t.splits().front();
        KMyMoneyRegister::SelectedTransaction st(t, s);
        m_transactions.append(st);
      }

      //    Save pricing information
      QList<MyMoneySplit>::const_iterator it_t;
      for (it_t = t.splits().constBegin(); it_t != t.splits().constEnd(); ++it_t) {
        if (((*it_t).action() != MyMoneySplit::ActionBuyShares) &&
            ((*it_t).action() != MyMoneySplit::ActionReinvestDividend)) {
          continue;
        }
        QString id = (*it_t).accountId();
        MyMoneyAccount acc = file->account(id);
        MyMoneySecurity sec = file->security(acc.currencyId());
        MyMoneyPrice price(acc.currencyId(),
                           sec.tradingCurrency(),
                           t.postDate(),
                           (*it_t).price(), "Transaction");
        file->addPrice(price);
        break;
      }

      ft.commit();

      // now analyze the balances and spit out warnings to the user
      QMap<QString, bool>::const_iterator it_a;

      if (!suppressBalanceWarnings) {
        for (it_a = accountIds.constBegin(); it_a != accountIds.constEnd(); ++it_a) {
          QString msg;
          MyMoneyAccount acc = file->account(it_a.key());
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
            emit balanceWarning(m_regForm, acc, msg);
          }
        }
      }
    } catch (const MyMoneyException &e) {
      qDebug("Unable to store transaction within engine: %s", qPrintable(e.what()));
      newTransactionCreated = false;
    }

    emit statusProgress(-1, -1);
    emit statusMsg(QString());

  }
  return storeTransactions;
}

void TransactionEditor::keepNewNumber(const MyMoneyTransaction& tr)
{
  // verify that new number, possibly containing alpha, is valid
  MyMoneyTransaction txn = tr;
  MyMoneyFile* file = MyMoneyFile::instance();
  if (!txn.splits().isEmpty()) {
    QString number = txn.splits().first().number();
    if (KMyMoneyUtils::numericPart(number) > 0) {
      // numeric is valid
      kMyMoneyLineEdit* numberEdit = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
      if (numberEdit) {
        numberEdit->loadText(number);
        MyMoneySplit split = txn.splits().first();
        split.setNumber(number);
        txn.modifySplit(split);
        m_account.setValue("lastNumberUsed", number);
        file->modifyAccount(m_account);
      }
    }
  }
}

void TransactionEditor::resizeForm()
{
  // force resizeing of the columns in the form
  KMyMoneyTransactionForm::TransactionForm* form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(m_regForm);
  if (form) {
    QMetaObject::invokeMethod(form, "resize", Qt::QueuedConnection, QGenericReturnArgument(), Q_ARG(int, ValueColumn1));
  }
}

StdTransactionEditor::StdTransactionEditor() :
    m_inUpdateVat(false)
{
}

StdTransactionEditor::StdTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) :
    TransactionEditor(regForm, item, list, lastPostDate),
    m_inUpdateVat(false)
{
}

StdTransactionEditor::~StdTransactionEditor()
{
}


void StdTransactionEditor::createEditWidgets()
{
  // we only create the account widget in case it is needed
  // to avoid confusion in the tab order later on.
  if (m_item->showRowInForm(0)) {
    KMyMoneyCategory* account = new KMyMoneyCategory;
    account->setClickMessage(i18n("Account"));
    account->setObjectName(QLatin1String("Account"));
    m_editWidgets["account"] = account;
    connect(account, SIGNAL(editTextChanged(QString)), this, SLOT(slotUpdateButtonState()));
    connect(account, SIGNAL(itemSelected(QString)), this, SLOT(slotUpdateAccount(QString)));
  }

  KMyMoneyPayeeCombo* payee = new KMyMoneyPayeeCombo;
  payee->setClickMessage(i18n("Payer/Receiver"));
  payee->setObjectName(QLatin1String("Payee"));
  m_editWidgets["payee"] = payee;

  connect(payee, SIGNAL(createItem(QString,QString&)), this, SIGNAL(createPayee(QString,QString&)));
  connect(payee, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(payee, SIGNAL(itemSelected(QString)), this, SLOT(slotUpdatePayee(QString)));
  connect(payee, SIGNAL(editTextChanged(QString)), this, SLOT(slotUpdateButtonState()));

  KMyMoneyCategory* category = new KMyMoneyCategory(0, true);
  category->setClickMessage(i18n("Category/Account"));
  category->setObjectName(QLatin1String("Category/Account"));
  m_editWidgets["category"] = category;
  connect(category, SIGNAL(itemSelected(QString)), this, SLOT(slotUpdateCategory(QString)));
  connect(category, SIGNAL(editTextChanged(QString)), this, SLOT(slotUpdateButtonState()));
  connect(category, SIGNAL(createItem(QString,QString&)), this, SLOT(slotCreateCategory(QString,QString&)));
  connect(category, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(category->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditSplits()));
  // initially disable the split button since we don't have an account set
  if (category->splitButton())
    category->splitButton()->setDisabled(m_account.id().isEmpty());

  KTagContainer* tag = new KTagContainer;
  tag->tagCombo()->setClickMessage(i18n("Tag"));
  tag->tagCombo()->setObjectName(QLatin1String("Tag"));
  m_editWidgets["tag"] = tag;
  connect(tag->tagCombo(), SIGNAL(createItem(QString,QString&)), this, SIGNAL(createTag(QString,QString&)));
  connect(tag->tagCombo(), SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  KTextEdit* memo = new KTextEdit;
  memo->setObjectName(QLatin1String("Memo"));
  memo->setTabChangesFocus(true);
  connect(memo, SIGNAL(textChanged()), this, SLOT(slotUpdateMemoState()));
  connect(memo, SIGNAL(textChanged()), this, SLOT(slotUpdateButtonState()));
  m_editWidgets["memo"] = memo;
  m_memoText.clear();
  m_memoChanged = false;

  bool showNumberField = true;
  switch (m_account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      showNumberField = KMyMoneyGlobalSettings::alwaysShowNrField();
      break;

    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      showNumberField = false;
      break;

    default:
      break;
  }

  if (showNumberField) {
    kMyMoneyLineEdit* number = new kMyMoneyLineEdit;
    number->setClickMessage(i18n("Number"));
    number->setObjectName(QLatin1String("Number"));
    m_editWidgets["number"] = number;
    connect(number, SIGNAL(lineChanged(QString)), this, SLOT(slotNumberChanged(QString)));
    // number->installEventFilter(this);
  }

  kMyMoneyDateInput* postDate = new kMyMoneyDateInput;
  m_editWidgets["postdate"] = postDate;
  postDate->setObjectName(QLatin1String("PostDate"));
  connect(postDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotUpdateButtonState()));
  postDate->setDate(QDate());

  kMyMoneyEdit* value = new kMyMoneyEdit;
  m_editWidgets["amount"] = value;
  value->setObjectName(QLatin1String("Amount"));
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(QString)), this, SLOT(slotUpdateAmount(QString)));
  connect(value, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonState()));

  value = new kMyMoneyEdit;
  m_editWidgets["payment"] = value;
  value->setObjectName(QLatin1String("Payment"));
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(QString)), this, SLOT(slotUpdatePayment(QString)));
  connect(value, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonState()));

  value = new kMyMoneyEdit;
  m_editWidgets["deposit"] = value;
  value->setObjectName(QLatin1String("Deposit"));
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(QString)), this, SLOT(slotUpdateDeposit(QString)));
  connect(value, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonState()));

  KMyMoneyCashFlowCombo* cashflow = new KMyMoneyCashFlowCombo(0, m_account.accountGroup());
  m_editWidgets["cashflow"] = cashflow;
  cashflow->setObjectName(QLatin1String("Cashflow"));
  connect(cashflow, SIGNAL(directionSelected(KMyMoneyRegister::CashFlowDirection)), this, SLOT(slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection)));

  KMyMoneyReconcileCombo* reconcile = new KMyMoneyReconcileCombo;
  m_editWidgets["status"] = reconcile;
  reconcile->setObjectName(QLatin1String("Reconcile"));

  KMyMoneyRegister::QWidgetContainer::iterator it_w;
  for (it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    (*it_w)->installEventFilter(this);
  }
  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if (!isMultiSelection()) {
    reconcile->removeDontCare();
    cashflow->removeDontCare();
  }

  QLabel* label;
  m_editWidgets["account-label"] = label = new QLabel(i18n("Account"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["category-label"] = label = new QLabel(i18n("Category"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["tag-label"] = label = new QLabel(i18n("Tags"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["memo-label"] = label = new QLabel(i18n("Memo"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["number-label"] = label = new QLabel(i18n("Number"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["date-label"] = label = new QLabel(i18n("Date"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["amount-label"] = label = new QLabel(i18n("Amount"));
  label->setAlignment(Qt::AlignVCenter);

  m_editWidgets["status-label"] = label = new QLabel(i18n("Status"));
  label->setAlignment(Qt::AlignVCenter);

  // create a copy of tabbar above the form (if we are created for a form)
  KMyMoneyTransactionForm::TransactionForm* form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(m_regForm);
  if (form) {
    KMyMoneyTransactionForm::TabBar* tabbar = new KMyMoneyTransactionForm::TabBar;
    m_editWidgets["tabbar"] = tabbar;
    tabbar->setObjectName(QLatin1String("TabBar"));
    tabbar->copyTabs(form->tabBar());
    connect(tabbar, SIGNAL(tabCurrentChanged(int)), this, SLOT(slotUpdateAction(int)));
    connect(tabbar, SIGNAL(tabCurrentChanged(int)), this, SIGNAL(operationTypeChanged(int)));
  }

  setupPrecision();
}

void StdTransactionEditor::setupCategoryWidget(QString& categoryId)
{
  TransactionEditor::setupCategoryWidget(dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]), m_splits, categoryId, SLOT(slotEditSplits()));

  if (m_splits.count() == 1)
    m_shares = m_splits[0].shares();
}

bool StdTransactionEditor::isTransfer(const QString& accId1, const QString& accId2) const
{
  if (accId1.isEmpty() || accId2.isEmpty())
    return false;

  return MyMoneyFile::instance()->account(accId1).isIncomeExpense() == MyMoneyFile::instance()->account(accId2).isIncomeExpense();
}

void StdTransactionEditor::loadEditWidgets(KMyMoneyRegister::Action action)
{
  // don't kick off VAT processing from here
  m_inUpdateVat = true;

  QMap<QString, QWidget*>::const_iterator it_w;
  QWidget* w;
  AccountSet aSet;

  // load the account widget
  KMyMoneyCategory* account = dynamic_cast<KMyMoneyCategory*>(haveWidget("account"));
  if (account) {
    aSet.addAccountGroup(MyMoneyAccount::Asset);
    aSet.addAccountGroup(MyMoneyAccount::Liability);
    aSet.removeAccountType(MyMoneyAccount::AssetLoan);
    aSet.removeAccountType(MyMoneyAccount::CertificateDep);
    aSet.removeAccountType(MyMoneyAccount::Investment);
    aSet.removeAccountType(MyMoneyAccount::Stock);
    aSet.removeAccountType(MyMoneyAccount::MoneyMarket);
    aSet.removeAccountType(MyMoneyAccount::Loan);
    aSet.load(account->selector());
    account->completion()->setSelected(m_account.id());
    account->slotItemSelected(m_account.id());
  }

  // load the payee widget
  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  // load the category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  disconnect(category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));

  // load the tag widget
  //KMyMoneyTagCombo* tag = dynamic_cast<KMyMoneyTagCombo*>(m_editWidgets["tag"]);
  KTagContainer* tag = dynamic_cast<KTagContainer*>(m_editWidgets["tag"]);
  tag->loadTags(MyMoneyFile::instance()->tagList());

  // check if the current transaction has a reference to an equity account
  bool haveEquityAccount = false;
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = m_transaction.splits().constBegin(); !haveEquityAccount && it_s != m_transaction.splits().constEnd(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if (acc.accountType() == MyMoneyAccount::Equity)
      haveEquityAccount = true;
  }

  aSet.clear();
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if (KMyMoneyGlobalSettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(MyMoneyAccount::Equity);

  aSet.removeAccountType(MyMoneyAccount::CertificateDep);
  aSet.removeAccountType(MyMoneyAccount::Investment);
  aSet.removeAccountType(MyMoneyAccount::Stock);
  aSet.removeAccountType(MyMoneyAccount::MoneyMarket);
  aSet.load(category->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if (!m_account.id().isEmpty())
    category->selector()->removeItem(m_account.id());

  //  also show memo text if isMultiSelection()
  dynamic_cast<KTextEdit*>(m_editWidgets["memo"])->setText(m_split.memo());
  // need to know if it changed
  m_memoText = m_split.memo();
  m_memoChanged = false;

  if (!isMultiSelection()) {
    if (m_transaction.postDate().isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_transaction.postDate());
    else if (m_lastPostDate.isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_lastPostDate);
    else
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(QDate::currentDate());

    if ((w = haveWidget("number")) != 0) {
      dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(m_split.number());
      if (m_transaction.id().isEmpty()                              // new transaction
          && dynamic_cast<kMyMoneyLineEdit*>(w)->text().isEmpty()   // no number filled in
          && m_account.accountType() == MyMoneyAccount::Checkings   // checkings account
          && KMyMoneyGlobalSettings::autoIncCheckNumber()           // and auto inc number turned on?
          && action != KMyMoneyRegister::ActionDeposit              // only transfers or withdrawals
          && (m_scheduleInfo.isEmpty()                              // normal transactions
              || m_paymentMethod == MyMoneySchedule::STYPE_WRITECHEQUE)) {// only for STYPE_WRITECHEQUE schedule transaction
        assignNextNumber();
      }
    }
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(m_split.reconcileFlag());

    QString payeeId = m_split.payeeId();
    if (!payeeId.isEmpty()) {
      payee->setSelectedItem(payeeId);
    }
    QList<QString> t = m_split.tagIdList();
    if (!t.isEmpty()) {
      for (int i = 0; i < t.size(); i++)
        tag->addTagWidget(t[i]);
    }

    m_splits.clear();
    if (m_transaction.splitCount() < 2) {
      category->completion()->setSelected(QString());
    } else {
      QList<MyMoneySplit>::const_iterator it_s;
      for (it_s = m_transaction.splits().constBegin(); it_s != m_transaction.splits().constEnd(); ++it_s) {
        if ((*it_s) == m_split)
          continue;
        m_splits.append(*it_s);
      }
    }
    QString categoryId;
    setupCategoryWidget(categoryId);

    if ((w = haveWidget("cashflow")) != 0) {
      KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
      cashflow->setDirection(!m_split.value().isPositive() ? KMyMoneyRegister::Payment : KMyMoneyRegister::Deposit);  //  include isZero case
    }

    if ((w = haveWidget("category-label")) != 0) {
      QLabel *categoryLabel = dynamic_cast<QLabel*>(w);
      if (isTransfer(m_split.accountId(), categoryId)) {
        if (m_split.value().isPositive())
          categoryLabel->setText(i18n("Transfer from"));
        else
          categoryLabel->setText(i18n("Transfer to"));
      }
    }

    MyMoneyMoney value = m_split.shares();

    if (haveWidget("deposit")) {
      if (m_split.shares().isNegative()) {
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->loadText("");
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setValue(value.abs());
      } else {
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setValue(value.abs());
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->loadText("");
      }
    }
    if ((w = haveWidget("amount")) != 0) {
      dynamic_cast<kMyMoneyEdit*>(w)->setValue(value.abs());
    }

    slotUpdateCategory(categoryId);

    // try to preset for specific action if a new transaction is being started
    if (m_transaction.id().isEmpty()) {
      if ((w = haveWidget("category-label")) != 0) {
        TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
        if (action == KMyMoneyRegister::ActionNone) {
          if (tabbar) {
            action = static_cast<KMyMoneyRegister::Action>(tabbar->currentIndex());
          }
        }
        if (action != KMyMoneyRegister::ActionNone) {
          QLabel *categoryLabel = dynamic_cast<QLabel*>(w);
          if (action == KMyMoneyRegister::ActionTransfer) {
            if (m_split.value().isPositive())
              categoryLabel->setText(i18n("Transfer from"));
            else
              categoryLabel->setText(i18n("Transfer to"));
          }
          if ((w = haveWidget("cashflow")) != 0) {
            KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
            if (action == KMyMoneyRegister::ActionDeposit || (action == KMyMoneyRegister::ActionTransfer && m_split.value().isPositive()))
              cashflow->setDirection(KMyMoneyRegister::Deposit);
            else
              cashflow->setDirection(KMyMoneyRegister::Payment);
          }
          if (tabbar) {
            tabbar->setCurrentIndex(action);
          }
        }
      }
    } else {
      TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
      if (tabbar) {
        if (!isTransfer(m_split.accountId(), categoryId)) {
          tabbar->setCurrentIndex(m_split.value().isNegative() ? KMyMoneyRegister::ActionWithdrawal : KMyMoneyRegister::ActionDeposit);
        } else {
          tabbar->setCurrentIndex(KMyMoneyRegister::ActionTransfer);
        }
      }
    }

  } else {  //  isMultiSelection()
    dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->loadDate(QDate());
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(MyMoneySplit::Unknown);
    if (haveWidget("deposit")) {
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->loadText("");
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setAllowEmpty();
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->loadText("");
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setAllowEmpty();
    }
    if ((w = haveWidget("amount")) != 0) {
      dynamic_cast<kMyMoneyEdit*>(w)->loadText("");
      dynamic_cast<kMyMoneyEdit*>(w)->setAllowEmpty();
    }

    slotUpdateAction(action);

    if ((w = haveWidget("tabbar")) != 0) {
      w->setEnabled(false);
    }

    category->completion()->setSelected(QString());
  }

  // allow kick off VAT processing again
  m_inUpdateVat = false;
}

QWidget* StdTransactionEditor::firstWidget() const
{
  QWidget* w = 0;
  if (m_initialAction != KMyMoneyRegister::ActionNone) {
    w = haveWidget("payee");
  }
  return w;
}

void StdTransactionEditor::slotReloadEditWidgets()
{
  // reload category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  QString categoryId = category->selectedItem();

  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if (KMyMoneyGlobalSettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(category->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if (!m_account.id().isEmpty())
    category->selector()->removeItem(m_account.id());

  if (!categoryId.isEmpty())
    category->setSelectedItem(categoryId);


  // reload payee widget
  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  QString payeeId = payee->selectedItem();

  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if (!payeeId.isEmpty()) {
    payee->setSelectedItem(payeeId);
  }

  // reload tag widget
  KTagContainer* tag = dynamic_cast<KTagContainer*>(m_editWidgets["tag"]);
  QString tagId = tag->tagCombo()->selectedItem();

  tag->loadTags(MyMoneyFile::instance()->tagList());

  if (!tagId.isEmpty()) {
    tag->RemoveAllTagWidgets();
    tag->addTagWidget(tagId);
  }
}

void StdTransactionEditor::slotUpdatePayee(const QString& payeeId)
{
  // we have a new payee assigned to this transaction.
  // in case there is no category assigned, no value entered and no
  // memo available, we search for the last transaction of this payee
  // in the account.
  if (m_transaction.id().isEmpty()
      && m_splits.count() == 0
      && KMyMoneyGlobalSettings::autoFillTransaction() != 0) {
    // check if category is empty
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    QStringList list;
    category->selectedItems(list);
    if (!list.isEmpty())
      return;

    // check if memo is empty
    KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
    if (memo && !memo->toPlainText().isEmpty())
      return;

    // check if all value fields are empty
    kMyMoneyEdit* amount;
    QStringList fields;
    fields << "amount" << "payment" << "deposit";
    QStringList::const_iterator it_f;
    for (it_f = fields.constBegin(); it_f != fields.constEnd(); ++it_f) {
      amount = dynamic_cast<kMyMoneyEdit*>(haveWidget(*it_f));
      if (amount && !amount->value().isZero())
        return;
    }

#if 0
    // Tony mentioned, that autofill does not work when he changed the date. Well,
    // that certainly makes sense when you enter transactions in register mode as
    // opposed to form mode, because the date field is located prior to the date
    // field in the tab order of the widgets and the user might have already
    // changed it.
    //
    // So I commented out the code that checks the date but left it in for reference.
    // (ipwizard, 2008-04-07)

    // check if date has been altered by user
    kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
    if ((m_lastPostDate.isValid() && (postDate->date() != m_lastPostDate))
        || (!m_lastPostDate.isValid() && (postDate->date() != QDate::currentDate())))
      return;
#endif

    // if we got here, we have to autofill
    autoFill(payeeId);
  }

  // If payee has associated default account (category), set that now.
  const MyMoneyPayee& payeeObj = MyMoneyFile::instance()->payee(payeeId);
  if (payeeObj.defaultAccountEnabled()) {
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    category->slotItemSelected(payeeObj.defaultAccountId());
  }
}

MyMoneyMoney StdTransactionEditor::shares(const MyMoneyTransaction& t) const
{
  MyMoneyMoney result;
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if ((*it_s).accountId() == m_account.id()) {
      result += (*it_s).shares();
    }
  }
  return result;
}

struct uniqTransaction {
  const MyMoneyTransaction* t;
  int   cnt;
};

void StdTransactionEditor::autoFill(const QString& payeeId)
{
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >  list;
  MyMoneyTransactionFilter filter(m_account.id());
  filter.addPayee(payeeId);
  MyMoneyFile::instance()->transactionList(list, filter);
  if (!list.empty()) {
    // ok, we found at least one previous transaction. now we clear out
    // what we have collected so far and add those splits from
    // the previous transaction.
    QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator  it_t;
    QMap<QString, struct uniqTransaction> uniqList;

    // collect the transactions and see if we have any duplicates
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      QString key = (*it_t).first.accountSignature();
      int cnt = 0;
      QMap<QString, struct uniqTransaction>::iterator it_u;
      do {
        QString ukey = QString("%1-%2").arg(key).arg(cnt);
        it_u = uniqList.find(ukey);
        if (it_u == uniqList.end()) {
          uniqList[ukey].t = &((*it_t).first);
          uniqList[ukey].cnt = 1;
        } else if (KMyMoneyGlobalSettings::autoFillTransaction() == 1) {
          // we already have a transaction with this signature. we must
          // now check, if we should really treat it as a duplicate according
          // to the value comparison delta.
          MyMoneyMoney s1 = shares(*((*it_u).t));
          MyMoneyMoney s2 = shares((*it_t).first);
          if (s2.abs() > s1.abs()) {
            MyMoneyMoney t(s1);
            s1 = s2;
            s2 = t;
          }
          MyMoneyMoney diff;
          if (s2.isZero())
            diff = s1.abs();
          else
            diff = ((s1 - s2) / s2).convert(10000);
          if (diff.isPositive() && diff <= MyMoneyMoney(KMyMoneyGlobalSettings::autoFillDifference(), 100)) {
            uniqList[ukey].t = &((*it_t).first);
            break;    // end while loop
          }
        } else if (KMyMoneyGlobalSettings::autoFillTransaction() == 2) {
          (*it_u).cnt++;
          break;      // end while loop
        }
        ++cnt;
      } while (it_u != uniqList.end());

    }

    MyMoneyTransaction t;
    if (KMyMoneyGlobalSettings::autoFillTransaction() != 2) {
#if 0
      // I removed this code to allow cancellation of an autofill if
      // it does not match even if there is only a single matching
      // transaction for the payee in question. In case, we want to revert
      // to the old behavior, don't forget to uncomment the closing
      // brace further down in the code as well. (ipwizard 2009-01-16)
      if (uniqList.count() == 1) {
        t = list.last().first;
      } else {
#endif
        QPointer<KSelectTransactionsDlg> dlg = new KSelectTransactionsDlg(m_account, m_regForm);
        dlg->setWindowTitle(i18n("Select autofill transaction"));

        QMap<QString, struct uniqTransaction>::const_iterator it_u;
        for (it_u = uniqList.constBegin(); it_u != uniqList.constEnd(); ++it_u) {
          dlg->addTransaction(*(*it_u).t);
        }

        // setup sort order
        dlg->m_register->setSortOrder("1,-9,-4");
        // sort the transactions according to the sort setting
        dlg->m_register->sortItems();

        // and select the last item
        if (dlg->m_register->lastItem())
          dlg->m_register->selectItem(dlg->m_register->lastItem());

        if (dlg->exec() == KDialog::Accepted) {
          t = dlg->transaction();
        }
#if 0
      }
#endif
    } else {
      int maxCnt = 0;
      QMap<QString, struct uniqTransaction>::const_iterator it_u;
      for (it_u = uniqList.constBegin(); it_u != uniqList.constEnd(); ++it_u) {
        if ((*it_u).cnt > maxCnt) {
          t = *(*it_u).t;
          maxCnt = (*it_u).cnt;
        }
      }
    }

    if (t != MyMoneyTransaction()) {
      m_transaction.removeSplits();
      m_split = MyMoneySplit();
      MyMoneySplit otherSplit;
      QList<MyMoneySplit>::ConstIterator it;
      for (it = t.splits().constBegin(); it != t.splits().constEnd(); ++it) {
        MyMoneySplit s(*it);
        s.setReconcileFlag(MyMoneySplit::NotReconciled);
        s.setReconcileDate(QDate());
        s.clearId();
        s.setBankID(QString());
        // older versions of KMyMoney used to set the action
        // we don't need this anymore
        if (s.action() != MyMoneySplit::ActionAmortization
            && s.action() != MyMoneySplit::ActionInterest)  {
          s.setAction(QString());
        }

        // FIXME update check number. The old comment contained
        //
        // <quote>
        // If a check number is already specified by the user it is
        // used. If the input field is empty and the previous transaction
        // contains a checknumber, the next usable check number will be assigned
        // to the transaction.
        // </quote>

        kMyMoneyLineEdit* editNr = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
        if (editNr && !editNr->text().isEmpty()) {
          s.setNumber(editNr->text());
        } else if (!s.number().isEmpty()) {
          s.setNumber(KMyMoneyUtils::nextCheckNumber(m_account));
        }

        // if the memos should not be used with autofill or
        // if the transaction has exactly two splits, remove
        // the memo text of the split that does not reference
        // the current account. This allows the user to change
        // the autofilled memo text which will then also be used
        // in this split. See createTransaction() for this logic.
        if ((s.accountId() != m_account.id() && t.splitCount() == 2) || !KMyMoneyGlobalSettings::autoFillUseMemos())
          s.setMemo(QString());

        m_transaction.addSplit(s);
        if (s.accountId() == m_account.id() && m_split == MyMoneySplit()) {
          m_split = s;
        } else {
          otherSplit = s;
        }
      }

      // make sure to extract the right action
      KMyMoneyRegister::Action action;
      action = m_split.shares().isNegative() ? KMyMoneyRegister::ActionWithdrawal : KMyMoneyRegister::ActionDeposit;

      if (m_transaction.splitCount() == 2) {
        MyMoneyAccount acc = MyMoneyFile::instance()->account(otherSplit.accountId());
        if (acc.isAssetLiability())
          action = KMyMoneyRegister::ActionTransfer;
      }

      // now setup the widgets with the new data but keep the date
      QDate date = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->date();
      loadEditWidgets(action);
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(date);
    }
  }

  // focus jumps into the category field
  QWidget* w;
  if ((w = haveWidget("payee")) != 0) {
    w->setFocus();
  }
}

void StdTransactionEditor::slotUpdateAction(int action)
{
  TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
  if (tabbar) {
    QLabel* categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(m_editWidgets["cashflow"]);
    switch (action) {
      case KMyMoneyRegister::ActionDeposit:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(KMyMoneyRegister::Deposit);
        break;
      case KMyMoneyRegister::ActionTransfer:
        if (m_split.shares().isNegative()) {
          cashflow->setDirection(KMyMoneyRegister::Payment);
          categoryLabel->setText(i18n("Transfer to"));
        } else {
          cashflow->setDirection(KMyMoneyRegister::Deposit);
          categoryLabel->setText(i18n("Transfer from"));
        }
        tabbar->setCurrentIndex(KMyMoneyRegister::ActionTransfer);
        slotUpdateCashFlow(cashflow->direction());
        break;
      case KMyMoneyRegister::ActionWithdrawal:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(KMyMoneyRegister::Payment);
        break;
    }
    resizeForm();
  }
}

void StdTransactionEditor::slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection dir)
{
  QLabel* categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  cashflow->setDirection(dir);
  // qDebug("Update cashflow to %d", dir);
  if (categoryLabel) {
    TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
    if (!tabbar) return;  //  no transaction form
    if (categoryLabel->text() != i18n("Category")) {
      tabbar->setCurrentIndex(KMyMoneyRegister::ActionTransfer);
      if (dir == KMyMoneyRegister::Deposit) {
        categoryLabel->setText(i18n("Transfer from"));
      } else {
        categoryLabel->setText(i18n("Transfer to"));
      }
      resizeForm();
    } else {
      if (dir == KMyMoneyRegister::Deposit)
        tabbar->setCurrentIndex(KMyMoneyRegister::ActionDeposit);
      else
        tabbar->setCurrentIndex(KMyMoneyRegister::ActionWithdrawal);
    }
  }
}

void StdTransactionEditor::slotUpdateCategory(const QString& id)
{
  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  // qDebug("Update category to %s", qPrintable(id));

  if (categoryLabel) {
    TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"]);
    MyMoneyMoney val = amount->value();

    if (categoryLabel->text() == i18n("Transfer from")) {
      val = -val;
    } else {
      val = val.abs();
    }

    if (tabbar) {
      tabbar->setTabEnabled(KMyMoneyRegister::ActionTransfer, true);
      tabbar->setTabEnabled(KMyMoneyRegister::ActionDeposit, true);
      tabbar->setTabEnabled(KMyMoneyRegister::ActionWithdrawal, true);
    }

    bool disableTransferTab = false;
    if (!id.isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      if (acc.isAssetLiability()
          || acc.accountGroup() == MyMoneyAccount::Equity) {
        if (tabbar) {
          tabbar->setCurrentIndex(KMyMoneyRegister::ActionTransfer);
          tabbar->setTabEnabled(KMyMoneyRegister::ActionDeposit, false);
          tabbar->setTabEnabled(KMyMoneyRegister::ActionWithdrawal, false);
        }
        KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(m_editWidgets["cashflow"]);
        if (val.isZero()) {
          if (cashflow && (cashflow->direction() == KMyMoneyRegister::Deposit)) {
            categoryLabel->setText(i18n("Transfer from"));
          } else {
            categoryLabel->setText(i18n("Transfer to"));
          }
        } else if (val.isNegative()) {
          categoryLabel->setText(i18n("Transfer from"));
          cashflow->setDirection(KMyMoneyRegister::Deposit);
        } else
          categoryLabel->setText(i18n("Transfer to"));
      } else {
        disableTransferTab = true;
        categoryLabel->setText(i18n("Category"));
      }
      updateAmount(val);
    } else {  //id.isEmpty()
      KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
      disableTransferTab = !category->currentText().isEmpty();
      categoryLabel->setText(i18n("Category"));
    }
    if (tabbar) {
      if (disableTransferTab) {
        // set the proper tab before disabling the currently active tab
        if (tabbar->currentIndex() == KMyMoneyRegister::ActionTransfer) {
          tabbar->setCurrentIndex(val.isPositive() ? KMyMoneyRegister::ActionWithdrawal : KMyMoneyRegister::ActionDeposit);
        }
        tabbar->setTabEnabled(KMyMoneyRegister::ActionTransfer, false);
      }
      tabbar->update();
    }

    resizeForm();
  }
  updateVAT(false);
}

void StdTransactionEditor::slotUpdatePayment(const QString& txt)
{
  MyMoneyMoney val(txt);

  if (val.isNegative()) {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setValue(val.abs());
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->clearText();
  } else {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->clearText();
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateDeposit(const QString& txt)
{
  MyMoneyMoney val(txt);
  if (val.isNegative()) {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setValue(val.abs());
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->clearText();
  } else {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->clearText();
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateAmount(const QString& txt)
{
  // qDebug("Update amount to %s", qPrintable(txt));
  MyMoneyMoney val(txt);
  updateAmount(val);
  updateVAT(true);
}

void StdTransactionEditor::updateAmount(const MyMoneyMoney& val)
{
  // we don't do anything if we have multiple transactions selected
  if (isMultiSelection())
    return;

  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  if (categoryLabel) {
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(m_editWidgets["cashflow"]);

    if (!val.isPositive())  {  //   fixes BUG321317
      if (categoryLabel->text() != i18n("Category")) {
        if (cashflow->direction() == KMyMoneyRegister::Payment) {
          categoryLabel->setText(i18n("Transfer to"));
        }
      } else {
        slotUpdateCashFlow(cashflow->direction());
      }
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"])->setValue(val.abs());
    } else {
      if (categoryLabel->text() != i18n("Category")) {
        if (cashflow->direction() == KMyMoneyRegister::Payment) {
          categoryLabel->setText(i18n("Transfer to"));
        } else {
          categoryLabel->setText(i18n("Transfer from"));
          cashflow->setDirection(KMyMoneyRegister::Deposit);  //  editing with +ve shows 'from' not 'pay to'
        }
      }
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"])->setValue(val.abs());
    }
  }
}

void StdTransactionEditor::updateVAT(bool amountChanged)
{
  // make sure that we don't do this recursively
  if (m_inUpdateVat)
    return;

  // we don't do anything if we have multiple transactions selected
  if (isMultiSelection())
    return;

  // if auto vat assignment for this account is turned off
  // we don't care about taxes
  if (m_account.value("NoVat") == "Yes")
    return;

  // more splits than category and tax are not supported
  if (m_splits.count() > 2)
    return;

  // in order to do anything, we need an amount
  MyMoneyMoney amount, newAmount;
  bool amountOk;
  amount = amountFromWidget(&amountOk);
  if (!amountOk)
    return;

  // If the transaction has a tax and a category split, remove the tax split
  if (m_splits.count() == 2) {
    newAmount = removeVatSplit();
    if (m_splits.count() == 2) // not removed?
      return;

  } else {
    // otherwise, we need a category
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if (category->selectedItem().isEmpty())
      return;

    // if no VAT account is associated with this category/account, then we bail out
    MyMoneyAccount cat = MyMoneyFile::instance()->account(category->selectedItem());
    if (cat.value("VatAccount").isEmpty())
      return;

    newAmount = amount;
  }

  // seems we have everything we need
  if (amountChanged)
    newAmount = amount;

  MyMoneyTransaction transaction;
  if (createTransaction(transaction, m_transaction, m_split)) {
    if (addVatSplit(transaction, newAmount)) {
      m_transaction = transaction;
      if (!m_transaction.splits().isEmpty())
        m_split = m_transaction.splits().front();

      loadEditWidgets();

      // if we made this a split transaction, then move the
      // focus to the memo field
      if (qApp->focusWidget() == haveWidget("category")) {
        QWidget* w = haveWidget("memo");
        if (w)
          w->setFocus();
      }
    }
  }
}

bool StdTransactionEditor::addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount)
{
  if (tr.splitCount() != 2)
    return false;

  MyMoneyFile* file = MyMoneyFile::instance();
  // extract the category split from the transaction
  MyMoneyAccount category = file->account(tr.splitByAccount(m_account.id(), false).accountId());
  return file->addVATSplit(tr, m_account, category, amount);
}

MyMoneyMoney StdTransactionEditor::removeVatSplit()
{
  // we only deal with splits that have three splits
  if (m_splits.count() != 2)
    return amountFromWidget();

  MyMoneySplit c; // category split
  MyMoneySplit t; // tax split

  bool netValue = false;
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = m_splits.constBegin(); it_s != m_splits.constEnd(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if (!acc.value("VatAccount").isEmpty()) {
      netValue = (acc.value("VatAmount").toLower() == "net");
      c = (*it_s);
    } else if (!acc.value("VatRate").isEmpty()) {
      t = (*it_s);
    }
  }

  // bail out if not all splits are setup
  if (c.id().isEmpty() || t.id().isEmpty())
    return amountFromWidget();

  MyMoneyMoney amount;
  // reduce the splits
  if (netValue) {
    amount = -c.shares();
  } else {
    amount = -(c.shares() + t.shares());
  }

  // remove tax split from the list, ...
  m_splits.clear();
  m_splits.append(c);

  // ... make sure that the widget is updated ...
  // block the signals to avoid popping up the split editor dialog
  // for nothing
  m_editWidgets["category"]->blockSignals(true);
  QString id;
  setupCategoryWidget(id);
  m_editWidgets["category"]->blockSignals(false);

  // ... and return the updated amount
  return amount;
}

bool StdTransactionEditor::isComplete(QString& reason) const
{
  reason.clear();
  QMap<QString, QWidget*>::const_iterator it_w;

  kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
  if (postDate) {
    QDate accountOpeningDate = m_account.openingDate();
    for (QList<MyMoneySplit>::const_iterator it_s = m_splits.constBegin(); it_s != m_splits.constEnd(); ++it_s) {
      const MyMoneyAccount& acc = MyMoneyFile::instance()->account((*it_s).accountId());
      // compute the newest opening date of all accounts involved in the transaction
      if (acc.openingDate() > accountOpeningDate)
        accountOpeningDate = acc.openingDate();
    }
    // check the selected category in case m_splits hasn't been updated yet
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if (category && !category->selectedItem().isEmpty()) {
      MyMoneyAccount cat = MyMoneyFile::instance()->account(category->selectedItem());
      if (cat.openingDate() > accountOpeningDate)
        accountOpeningDate = cat.openingDate();
    }

    if (postDate->date().isValid() && (postDate->date() < accountOpeningDate)) {
      postDate->markAsBadDate(true, KMyMoneyGlobalSettings::listNegativeValueColor());
      reason = i18n("Cannot enter transaction with postdate prior to account's opening date.");
      postDate->setToolTip(reason);
      return false;
    }
    postDate->markAsBadDate();
    postDate->setToolTip("");
  }

  for (it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it_w);
    KTagContainer* tagContainer = dynamic_cast<KTagContainer*>(*it_w);
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(*it_w);
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(*it_w);
    KMyMoneyReconcileCombo* reconcile = dynamic_cast<KMyMoneyReconcileCombo*>(*it_w);
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(*it_w);
    KTextEdit* memo = dynamic_cast<KTextEdit*>(*it_w);

    if (payee && !(payee->currentText().isEmpty()))
      break;

    if (category && !category->lineEdit()->text().isEmpty())
      break;

    if (amount && !(amount->value().isZero()))
      break;

    // the following widgets are only checked if we are editing multiple transactions
    if (isMultiSelection()) {
      TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
      if (tabbar) {
        tabbar->setEnabled(true);
      }
      if (reconcile && reconcile->state() != MyMoneySplit::Unknown)
        break;

      if (cashflow && cashflow->direction() != KMyMoneyRegister::Unknown)
        break;

      if (postDate->date().isValid() && (postDate->date() >= m_account.openingDate()))
        break;

      if (memo && m_memoChanged)
        break;

      if (tagContainer && !(tagContainer->selectedTags().isEmpty()))  //  Tag is optional field
        break;
    }
  }
  return it_w != m_editWidgets.end();
}

void StdTransactionEditor::slotCreateCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if (cashflow) {
    // form based input
    if (cashflow->direction() == KMyMoneyRegister::Deposit)
      parent = MyMoneyFile::instance()->income();
    else
      parent = MyMoneyFile::instance()->expense();

  } else if (haveWidget("deposit")) {
    // register based input
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"]);
    if (deposit->value().isPositive())
      parent = MyMoneyFile::instance()->income();
    else
      parent = MyMoneyFile::instance()->expense();

  } else
    parent = MyMoneyFile::instance()->expense();

  // TODO extract possible first part of a hierarchy and check if it is one
  // of our top categories. If so, remove it and select the parent
  // according to this information.

  emit createCategory(acc, parent);

  // return id
  id = acc.id();
}

int StdTransactionEditor::slotEditSplits()
{
  int rc = KDialog::Rejected;

  if (!m_openEditSplits) {
    // only get in here in a single instance
    m_openEditSplits = true;

    // force focus change to update all data
    QWidget* w = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"])->splitButton();
    if (w)
      w->setFocus();

    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("amount"));
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(haveWidget("deposit"));
    kMyMoneyEdit* payment = dynamic_cast<kMyMoneyEdit*>(haveWidget("payment"));
    KMyMoneyCashFlowCombo* cashflow = 0;
    KMyMoneyRegister::CashFlowDirection dir = KMyMoneyRegister::Unknown;
    bool isValidAmount = false;

    if (amount) {
      isValidAmount = amount->lineedit()->text().length() != 0;
      cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
      if (cashflow)
        dir = cashflow->direction();

    } else {
      if (deposit) {
        if (deposit->lineedit()->text().length() != 0) {
          isValidAmount = true;
          dir = KMyMoneyRegister::Deposit;
        }
      }
      if (payment) {
        if (payment->lineedit()->text().length() != 0) {
          isValidAmount = true;
          dir = KMyMoneyRegister::Payment;
        }
      }
      if (!deposit || !payment) {
        qDebug("Internal error: deposit(%p) & payment(%p) widgets not found but required", deposit, payment);
        return rc;
      }
    }

    if (dir == KMyMoneyRegister::Unknown)
      dir = KMyMoneyRegister::Payment;

    MyMoneyTransaction transaction;
    if (createTransaction(transaction, m_transaction, m_split)) {
      MyMoneyMoney value;

      QPointer<KSplitTransactionDlg> dlg =
        new KSplitTransactionDlg(transaction,
                                 transaction.splits().isEmpty() ? MyMoneySplit() : transaction.splits().front(),
                                 m_account,
                                 isValidAmount,
                                 dir == KMyMoneyRegister::Deposit,
                                 MyMoneyMoney(),
                                 m_priceInfo,
                                 m_regForm);
      connect(dlg, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
      connect(dlg, SIGNAL(createCategory(MyMoneyAccount&,MyMoneyAccount)), this, SIGNAL(createCategory(MyMoneyAccount&,MyMoneyAccount)));
      connect(dlg, SIGNAL(createTag(QString, QString&)), this, SIGNAL(createTag(QString, QString&)));

      if ((rc = dlg->exec()) == KDialog::Accepted) {
        m_transaction = dlg->transaction();
        if (!m_transaction.splits().isEmpty())
          m_split = m_transaction.splits().front();
        loadEditWidgets();
      }

      delete dlg;
    }

    // focus jumps into the tag field
    if ((w = haveWidget("tag")) != 0) {
      w->setFocus();
    }

    m_openEditSplits = false;
  }

  return rc;
}

void StdTransactionEditor::checkPayeeInSplit(MyMoneySplit& s, const QString& payeeId)
{
  if (s.accountId().isEmpty())
    return;

  MyMoneyAccount acc = MyMoneyFile::instance()->account(s.accountId());
  if (acc.isIncomeExpense()) {
    s.setPayeeId(payeeId);
  } else {
    if (s.payeeId().isEmpty())
      s.setPayeeId(payeeId);
  }
}

MyMoneyMoney StdTransactionEditor::amountFromWidget(bool* update) const
{
  bool updateValue = false;
  MyMoneyMoney value;

  KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if (cashflow) {
    // form based input
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"]);
    // if both fields do not contain changes -> no need to update
    if (cashflow->direction() != KMyMoneyRegister::Unknown
        && !amount->lineedit()->text().isEmpty())
      updateValue = true;
    value = amount->value();
    if (cashflow->direction() == KMyMoneyRegister::Payment)
      value = -value;

  } else if (haveWidget("deposit")) {
    // register based input
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"]);
    kMyMoneyEdit* payment = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"]);
    // if both fields do not contain text -> no need to update
    if (!(deposit->lineedit()->text().isEmpty() && payment->lineedit()->text().isEmpty()))
      updateValue = true;

    if (deposit->value().isPositive())
      value = deposit->value();
    else
      value = -(payment->value());
  }

  if (update)
    *update = updateValue;

  // determine the max fraction for this account and
  // adjust the value accordingly
  return value.convert(m_account.fraction());
}

bool StdTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog)
{
  // extract price info from original transaction
  m_priceInfo.clear();
  QList<MyMoneySplit>::const_iterator it_s;
  if (!torig.id().isEmpty()) {
    for (it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
      if ((*it_s).id() != sorig.id()) {
        MyMoneyAccount cat = MyMoneyFile::instance()->account((*it_s).accountId());
        if (cat.currencyId() != m_account.currencyId()) {
          if (!(*it_s).shares().isZero() && !(*it_s).value().isZero()) {
            m_priceInfo[cat.currencyId()] = ((*it_s).shares() / (*it_s).value()).reduce();
          }
        }
      }
    }
  }

  t = torig;

  t.removeSplits();
  t.setCommodity(m_account.currencyId());

  kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
  if (postDate->date().isValid()) {
    t.setPostDate(postDate->date());
  }

  // we start with the previous values, make sure we can add them later on
  MyMoneySplit s0 = sorig;
  s0.clearId();

  // make sure we reference this account here
  s0.setAccountId(m_account.id());

  // memo and number field are special: if we have multiple transactions selected
  // and the edit field is empty, we treat it as "not modified".
  // FIXME a better approach would be to have a 'dirty' flag with the widgets
  //       which identifies if the originally loaded value has been modified
  //       by the user
  KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
  if (memo) {
    if (!isMultiSelection() || (isMultiSelection() && m_memoChanged))
      s0.setMemo(memo->toPlainText());
  }

  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  if (number) {
    if (!isMultiSelection() || (isMultiSelection() && !number->text().isEmpty()))
      s0.setNumber(number->text());
  }

  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  QString payeeId;
  if (!isMultiSelection() || (isMultiSelection() && !payee->currentText().isEmpty())) {
    payeeId = payee->selectedItem();
    s0.setPayeeId(payeeId);
  }

  //KMyMoneyTagCombo* tag = dynamic_cast<KMyMoneyTagCombo*>(m_editWidgets["tag"]);
  KTagContainer* tag = dynamic_cast<KTagContainer*>(m_editWidgets["tag"]);
  if (!isMultiSelection() || (isMultiSelection() && !tag->selectedTags().isEmpty())) {
    s0.setTagIdList(tag->selectedTags());
  }

  bool updateValue;
  MyMoneyMoney value = amountFromWidget(&updateValue);

  if (updateValue) {
    // for this account, the shares and value is the same
    s0.setValue(value);
    s0.setShares(value);
  } else {
    value = s0.value();
  }

  // if we mark the split reconciled here, we'll use today's date if no reconciliation date is given
  KMyMoneyReconcileCombo* status = dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"]);
  if (status->state() != MyMoneySplit::Unknown)
    s0.setReconcileFlag(status->state());

  if (s0.reconcileFlag() == MyMoneySplit::Reconciled && !s0.reconcileDate().isValid())
    s0.setReconcileDate(QDate::currentDate());

  checkPayeeInSplit(s0, payeeId);

  // add the split to the transaction
  t.addSplit(s0);

  // if we have no other split we create it
  // if we have none or only one other split, we reconstruct it here
  // if we have more than one other split, we take them as they are
  // make sure to perform all those changes on a local copy
  QList<MyMoneySplit> splits = m_splits;

  MyMoneySplit s1;
  if (splits.isEmpty()) {
    s1.setMemo(s0.memo());
    splits.append(s1);

    // make sure we will fill the value and share fields later on
    updateValue = true;
  }

  // FIXME in multiSelection we currently only support transactions with one
  // or two splits. So we check the original transaction and extract the other
  // split or create it
  if (isMultiSelection()) {
    if (torig.splitCount() == 2) {
      QList<MyMoneySplit>::const_iterator it_s;
      for (it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
        if ((*it_s).id() == sorig.id())
          continue;
        s1 = *it_s;
        s1.clearId();
        break;
      }
    }
  } else {
    if (splits.count() == 1) {
      s1 = splits[0];
      s1.clearId();
    }
  }

  if (isMultiSelection() || splits.count() == 1) {
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if (!isMultiSelection() || (isMultiSelection() && !category->currentText().isEmpty())) {
      s1.setAccountId(category->selectedItem());
    }

    // if the first split has a memo but the second split is empty,
    // we just copy the memo text over
    if (memo) {
      if (!isMultiSelection() || (isMultiSelection() && !memo->toPlainText().isEmpty())) {
        // if the memo is filled, we check if the
        // account referenced by s1 is a regular account or a category.
        // in case of a regular account, we just leave the memo as is
        // in case of a category we simply copy the new value over the old.
        // in case we don't even have an account id, we just skip because
        // the split will be removed later on anyway.
        if (!s1.memo().isEmpty() && s1.memo() != s0.memo()) {
          if (!s1.accountId().isEmpty()) {
            MyMoneyAccount acc = MyMoneyFile::instance()->account(s1.accountId());
            if (acc.isIncomeExpense())
              s1.setMemo(s0.memo());
            else if (KMessageBox::questionYesNo(m_regForm,
                                                i18n("Do you want to replace memo<p><i>%1</i></p>with memo<p><i>%2</i></p>in the other split?", s1.memo(), s0.memo()), i18n("Copy memo"),
                                                KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                                QLatin1String("CopyMemoOver")) == KMessageBox::Yes)
              s1.setMemo(s0.memo());
          }
        } else {
          s1.setMemo(s0.memo());
        }
      }
    }

    if (updateValue && !s1.accountId().isEmpty()) {
      s1.setValue(-value);
      MyMoneyMoney shares;
      if (!skipPriceDialog) {
        if (!KCurrencyCalculator::setupSplitPrice(shares, t, s1, m_priceInfo, m_regForm))
          return false;
      } else {
        MyMoneyAccount cat = MyMoneyFile::instance()->account(s1.accountId());
        if (m_priceInfo.find(cat.currencyId()) != m_priceInfo.end()) {
          shares = (s1.value() * m_priceInfo[cat.currencyId()]).reduce().convert(cat.fraction());
        } else
          shares = s1.value();
      }
      s1.setShares(shares);
    }

    checkPayeeInSplit(s1, payeeId);

    if (!s1.accountId().isEmpty())
      t.addSplit(s1);

  } else {
    QList<MyMoneySplit>::iterator it_s;
    for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      s1 = *it_s;
      s1.clearId();
      checkPayeeInSplit(s1, payeeId);
      t.addSplit(s1);
    }
  }
  return true;
}

void StdTransactionEditor::setupFinalWidgets()
{
  addFinalWidget(haveWidget("deposit"));
  addFinalWidget(haveWidget("payment"));
  addFinalWidget(haveWidget("amount"));
  addFinalWidget(haveWidget("status"));
}

void StdTransactionEditor::slotUpdateAccount(const QString& id)
{
  TransactionEditor::slotUpdateAccount(id);
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  if (category && category->splitButton()) {
    category->splitButton()->setDisabled(id.isEmpty());
  }
}
