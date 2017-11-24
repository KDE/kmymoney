/***************************************************************************
                          stdtransaction.cpp  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "stdtransaction.h"
#include "stdtransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPainter>
#include <QWidget>
#include <QList>
#include <QBoxLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypayeecombo.h"
#include "kmymoneycombo.h"
#include "kmymoneytagcombo.h"
#include "tabbar.h"
#include "ktagcontainer.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "register.h"
#include "transactionform.h"
#include "kmymoneylineedit.h"
#include "kmymoneyutils.h"
#ifndef KMM_DESIGNER
#include "stdtransactioneditor.h"
#endif

#include "kmymoneyglobalsettings.h"
#include "widgetenums.h"
#include "mymoneyenums.h"

using namespace eWidgets;
using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransaction::StdTransaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  Transaction(*new StdTransactionPrivate, parent, transaction, split, uniqueId)
{
  Q_D(StdTransaction);
  d->m_showAccountRow = false;
  try {
    d->m_categoryHeader = i18n("Category");
    switch (transaction.splitCount()) {
      default:
        d->m_category = i18nc("Split transaction (category replacement)", "Split transaction");
        break;

      case 0: // the empty transaction
      case 1:
        break;

      case 2:
        setupFormHeader(d->m_transaction.splitByAccount(d->m_split.accountId(), false).accountId());
        break;
    }
  } catch (const MyMoneyException &e) {
    qDebug() << "Problem determining the category for transaction '" << d->m_transaction.id() << "'. Reason: " << e.what()  << "\n";
  }
  d->m_rowsForm = 6;

  if (KMyMoneyUtils::transactionType(d->m_transaction) == KMyMoneyUtils::InvestmentTransaction) {
    MyMoneySplit split = KMyMoneyUtils::stockSplit(d->m_transaction);
    d->m_payee = MyMoneyFile::instance()->account(split.accountId()).name();
    QString addon;
    if (split.action() == MyMoneySplit::ActionBuyShares) {
      if (split.value().isNegative()) {
        addon = i18n("Sell");
      } else {
        addon = i18n("Buy");
      }
    } else if (split.action() == MyMoneySplit::ActionDividend) {
      addon = i18n("Dividend");
    } else if (split.action() == MyMoneySplit::ActionYield) {
      addon = i18n("Yield");
    } else if (split.action() == MyMoneySplit::ActionInterestIncome) {
      addon = i18n("Interest Income");
    }
    if (!addon.isEmpty()) {
      d->m_payee += QString(" (%1)").arg(addon);
    }
    d->m_payeeHeader = i18n("Activity");
    d->m_category = i18n("Investment transaction");
  }

  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));

  emit parent->itemAdded(this);
}

StdTransaction::~StdTransaction()
{
}

const char* StdTransaction::className()
{
  return "StdTransaction";
}

void StdTransaction::setupFormHeader(const QString& id)
{
  Q_D(StdTransaction);
  d->m_category = MyMoneyFile::instance()->accountToCategory(id);
  switch (MyMoneyFile::instance()->account(id).accountGroup()) {
    case eMyMoney::Account::Asset:
    case eMyMoney::Account::Liability:
      d->m_categoryHeader = d->m_split.shares().isNegative() ? i18n("Transfer to") : i18n("Transfer from");
      break;

    default:
      d->m_categoryHeader = i18n("Category");
      break;
  }
}

eRegister::Action StdTransaction::actionType() const
{
  Q_D(const StdTransaction);
  eRegister::Action action = eRegister::Action::None;

  // if at least one split is referencing an income or
  // expense account, we will not call it a transfer
  QList<MyMoneySplit>::const_iterator it_s;

  for (it_s = d->m_transaction.splits().begin(); it_s != d->m_transaction.splits().end(); ++it_s) {
    if ((*it_s).accountId() == d->m_split.accountId())
      continue;
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if (acc.accountGroup() == eMyMoney::Account::Income
        || acc.accountGroup() == eMyMoney::Account::Expense) {
      // otherwise, we have to determine between deposit and withdrawal
      action = d->m_split.shares().isNegative() ? eRegister::Action::Withdrawal : eRegister::Action::Deposit;
      break;
    }
  }
  // otherwise, it's a transfer
  if (it_s == d->m_transaction.splits().end())
    action = eRegister::Action::Transfer;

  return action;
}

void StdTransaction::loadTab(TransactionForm* form)
{
  Q_D(StdTransaction);
  KMyMoneyTransactionForm::TabBar* bar = form->getTabBar();
  bar->setSignalEmission(eTabBar::SignalEmission::Never);
  for (auto i = 0; i < bar->count(); ++i) {
    bar->setTabEnabled(i, true);
  }

  if (d->m_transaction.splitCount() > 0) {
    bar->setCurrentIndex((int)actionType());
  }
  bar->setSignalEmission(eTabBar::SignalEmission::Always);
}

int StdTransaction::numColsForm() const
{
  return 4;
}

void StdTransaction::setupForm(TransactionForm* form)
{
  Transaction::setupForm(form);
  form->setSpan(4, (int)eTransactionForm::Column::Value1, 3, 1);
}

bool StdTransaction::showRowInForm(int row) const
{
  Q_D(const StdTransaction);
  return row == 0 ? d->m_showAccountRow : true;
}

void StdTransaction::setShowRowInForm(int row, bool show)
{
  Q_D(StdTransaction);
  if (row == 0)
    d->m_showAccountRow = show;
}

bool StdTransaction::formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* /* painter */)
{
  Q_D(const StdTransaction);
  // if(m_transaction != MyMoneyTransaction()) {
  switch (row) {
    case 0:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Account");
          break;
      }
      break;

    case 1:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = d->m_payeeHeader;
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          txt = d->m_payee;
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (haveNumberField())
            txt = i18n("Number");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          if (haveNumberField())
            txt = d->m_split.number();
          break;
      }
      break;

    case 2:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = d->m_categoryHeader;
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          txt = d->m_category;
          if (d->m_transaction != MyMoneyTransaction()) {
            if (txt.isEmpty() && !d->m_split.value().isZero())
              txt = i18n("*** UNASSIGNED ***");
          }
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          txt = i18n("Date");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          if (d->m_transaction != MyMoneyTransaction())
            txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;
      }
      break;

    case 3:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Tags");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          if (!d->m_tagList.isEmpty()) {
            for (auto i = 0; i < d->m_tagList.size() - 1; i++)
              txt += d->m_tagList[i] + ", ";
            txt += d->m_tagList.last();
          }
          //if (m_transaction != MyMoneyTransaction())
          //  txt = m_split.tagId();
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          txt = i18n("Amount");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          if (d->m_transaction != MyMoneyTransaction()) {
            txt = (d->m_split.value(d->m_transaction.commodity(), d->m_splitCurrencyId).abs()).formatMoney(d->m_account.fraction());
          }
          break;
      }
      break;

    case 4:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Memo");
          break;

        case (int)eTransactionForm::Column::Value1:
          align &= ~Qt::AlignVCenter;
          align |= Qt::AlignTop;
          align |= Qt::AlignLeft;
          if (d->m_transaction != MyMoneyTransaction())
            txt = d->m_split.memo().section('\n', 0, 2);
          break;
      }
      break;

    case 5:
      switch (col) {
        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          txt = i18n("Status");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          txt = reconcileState();
          break;
      }
  }

  // }
  if (col == (int)eTransactionForm::Column::Value2 && row == 1) {
    return haveNumberField();
  }
  return (col == (int)eTransactionForm::Column::Value1 && row < 5) || (col == (int)eTransactionForm::Column::Value2 && row > 0 && row != 4);
}

void StdTransaction::registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter)
{
  Q_D(const StdTransaction);
  switch (row) {
    case 0:
      switch (col) {
        case (int)eTransaction::Column::Number:
          align |= Qt::AlignLeft;
          if (haveNumberField())
            txt = d->m_split.number();
          break;

        case (int)eTransaction::Column::Date:
          align |= Qt::AlignLeft;
          txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;

        case (int)eTransaction::Column::Detail:
          switch (d->m_parent->getDetailsColumnType()) {
            case eRegister::DetailColumn::PayeeFirst:
              txt = d->m_payee;
              break;
            case eRegister::DetailColumn::AccountFirst:
              txt = d->m_category;
              if (!d->m_tagList.isEmpty()) {
                txt += " ( ";
                for (auto i = 0; i < d->m_tagList.size() - 1; i++) {
                  txt += "<span style='color: " + d->m_tagColorList[i].name() + "'>&#x25CF;</span> " + d->m_tagList[i] + ", ";
                }
                txt += "<span style='color: " + d->m_tagColorList.last().name() + "'>&#x25CF;</span> " + d->m_tagList.last() + " )";
              }
              break;
          }
          align |= Qt::AlignLeft;
          if (txt.isEmpty() && d->m_rowsRegister < 3) {
            singleLineMemo(txt, d->m_split);
          }
          if (txt.isEmpty() && d->m_rowsRegister < 2) {
            if (d->m_account.accountType() != eMyMoney::Account::Income
                && d->m_account.accountType() != eMyMoney::Account::Expense) {
              txt = d->m_category;
              if (txt.isEmpty() && !d->m_split.value().isZero()) {
                txt = i18n("*** UNASSIGNED ***");
                if (painter)
                  painter->setPen(KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionErroneous));
              }
            }
          }
          break;

        case (int)eTransaction::Column::ReconcileFlag:
          align |= Qt::AlignHCenter;
          txt = reconcileState(false);
          break;

        case (int)eTransaction::Column::Payment:
          align |= Qt::AlignRight;
          if (d->m_split.value().isNegative()) {
            txt = (-d->m_split.value(d->m_transaction.commodity(), d->m_splitCurrencyId)).formatMoney(d->m_account.fraction());
          }
          break;

        case (int)eTransaction::Column::Deposit:
          align |= Qt::AlignRight;
          if (!d->m_split.value().isNegative()) {
            txt = d->m_split.value(d->m_transaction.commodity(), d->m_splitCurrencyId).formatMoney(d->m_account.fraction());
          }
          break;

        case (int)eTransaction::Column::Balance:
          align |= Qt::AlignRight;
          if (d->m_showBalance)
            txt = d->m_balance.formatMoney(d->m_account.fraction());
          else
            txt = "----";
          break;

        case (int)eTransaction::Column::Account:
          // txt = m_objects->account(m_transaction.splits()[0].accountId()).name();
          txt = MyMoneyFile::instance()->account(d->m_split.accountId()).name();
          break;

        default:
          break;
      }
      break;

    case 1:
      switch (col) {
        case (int)eTransaction::Column::Detail:
          switch (d->m_parent->getDetailsColumnType()) {
            case eRegister::DetailColumn::PayeeFirst:
              txt = d->m_category;
              if (!d->m_tagList.isEmpty()) {
                txt += " ( ";
                for (auto i = 0; i < d->m_tagList.size() - 1; i++) {
                  txt += "<span style='color: " + d->m_tagColorList[i].name() + "'>&#x25CF;</span> " + d->m_tagList[i] + ", ";
                }
                txt += "<span style='color: " + d->m_tagColorList.last().name() + "'>&#x25CF;</span> " + d->m_tagList.last() + " )";
              }
              break;
            case eRegister::DetailColumn::AccountFirst:
              txt = d->m_payee;
              break;
          }
          align |= Qt::AlignLeft;
          if (txt.isEmpty() && !d->m_split.value().isZero()) {
            txt = i18n("*** UNASSIGNED ***");
            if (painter)
              painter->setPen(KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionErroneous));
          }
          break;

        default:
          break;
      }
      break;

    case 2:
      switch (col) {
        case (int)eTransaction::Column::Detail:
          align |= Qt::AlignLeft;
          singleLineMemo(txt, d->m_split);
          break;

        default:
          break;
      }
      break;
  }
}

int StdTransaction::registerColWidth(int col, const QFontMetrics& cellFontMetrics)
{
  QString txt;
  int firstRow = 0, lastRow = numRowsRegister();

  int nw = 0;
  for (int i = firstRow; i <= lastRow; ++i) {
    Qt::Alignment align;
    registerCellText(txt, align, i, col, 0);
    int w = cellFontMetrics.width(txt + "   ");
    if (w > nw)
      nw = w;
  }
  return nw;
}

void StdTransaction::arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(StdTransaction);
  if (!d->m_form || !d->m_parent)
    return;

  setupFormPalette(editWidgets);

  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Label1, editWidgets["account-label"]);
  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Value1, editWidgets["account"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Label1, editWidgets["cashflow"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Value1, editWidgets["payee"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Label1, editWidgets["category-label"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Value1, editWidgets["category"]->parentWidget());
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Label1, editWidgets["tag-label"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Value1, editWidgets["tag"]);
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Label1, editWidgets["memo-label"]);
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Value1, editWidgets["memo"]);
  if (haveNumberField()) {
    arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Label2, editWidgets["number-label"]);
    arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Value2, editWidgets["number"]);
  }
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Label2, editWidgets["date-label"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Value2, editWidgets["postdate"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Label2, editWidgets["amount-label"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Value2, editWidgets["amount"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Label2, editWidgets["status-label"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Value2, editWidgets["status"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for (it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    KMyMoneyLineEdit* edit = dynamic_cast<KMyMoneyLineEdit*>(*it);
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it);
    KTagContainer* tag = dynamic_cast<KTagContainer*>(*it);
    if (combo)
      combo->setPlaceholderText(QString());
    if (edit)
      edit->setPlaceholderText(QString());
    if (payee)
      payee->setPlaceholderText(QString());
    if (tag)
      tag->tagCombo()->setPlaceholderText(QString());
  }

  auto form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(d->m_form);
  auto w = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(editWidgets["tabbar"]);
  if (w) {
    // insert the tabbar in the boxlayout so it will take the place of the original tabbar which was hidden
    QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(form->getTabBar()->parentWidget()->layout());
    boxLayout->insertWidget(0, w);
  }
}

void StdTransaction::tabOrderInForm(QWidgetList& tabOrderWidgets) const
{
  Q_D(const StdTransaction);
  QStringList taborder = KMyMoneyGlobalSettings::stdTransactionFormTabOrder().split(',', QString::SkipEmptyParts);
  QStringList::const_iterator it_s = taborder.constBegin();
  QWidget* w;
  while (it_s != taborder.constEnd()) {
    if (*it_s == "account") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(0, (int)eTransactionForm::Column::Value1)));
    } else if (*it_s == "cashflow") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, (int)eTransactionForm::Column::Label1)));
    } else if (*it_s == "payee") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, (int)eTransactionForm::Column::Value1)));
    } else if (*it_s == "category") {
      // make sure to have the category field and the split button as separate tab order widgets
      // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
      // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
      // go haywire when someone changes the KMyMoneyCategory object ...
      QWidget* w = d->m_form->cellWidget(2, (int)eTransactionForm::Column::Value1);
      tabOrderWidgets.append(focusWidget(w));
      w = w->findChild<QPushButton*>("splitButton");
      if (w)
        tabOrderWidgets.append(w);
    } else if (*it_s == "tag") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(3, (int)eTransactionForm::Column::Value1)));
    } else if (*it_s == "memo") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(4, (int)eTransactionForm::Column::Value1)));
    } else if (*it_s == "number") {
      if (haveNumberField()) {
        if ((w = focusWidget(d->m_form->cellWidget(1, (int)eTransactionForm::Column::Value2))))
          tabOrderWidgets.append(w);
      }
    } else if (*it_s == "date") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(2, (int)eTransactionForm::Column::Value2)));
    } else if (*it_s == "amount") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(3, (int)eTransactionForm::Column::Value2)));
    } else if (*it_s == "state") {
      tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(5, (int)eTransactionForm::Column::Value2)));
    }
    ++it_s;
  }
}

void StdTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(StdTransaction);
  if (!d->m_parent)
    return;

  setupRegisterPalette(editWidgets);

  if (haveNumberField())
    arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Number, editWidgets["number"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Date, editWidgets["postdate"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, (int)eTransaction::Column::Date, editWidgets["status"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Detail, editWidgets["payee"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, (int)eTransaction::Column::Detail, editWidgets["category"]->parentWidget());
  arrangeWidget(d->m_parent, d->m_startRow + 2, (int)eTransaction::Column::Detail, editWidgets["tag"]);
  arrangeWidget(d->m_parent, d->m_startRow + 3, (int)eTransaction::Column::Detail, editWidgets["memo"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Payment, editWidgets["payment"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Deposit, editWidgets["deposit"]);

  // increase the height of the row containing the memo widget
  d->m_parent->setRowHeight(d->m_startRow + 3, d->m_parent->rowHeightHint() * 3);
}

void StdTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  Q_D(const StdTransaction);
  QStringList taborder = KMyMoneyGlobalSettings::stdTransactionRegisterTabOrder().split(',', QString::SkipEmptyParts);
  QStringList::const_iterator it_s = taborder.constBegin();
  QWidget* w;
  while (it_s != taborder.constEnd()) {
    if (*it_s == "number") {
      if (haveNumberField()) {
        if ((w = focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Number))))
          tabOrderWidgets.append(w);
      }
    } else if (*it_s == "date") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Date)));
    } else if (*it_s == "payee") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Detail)));
    } else if (*it_s == "category") {
      // make sure to have the category field and the split button as separate tab order widgets
      // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
      // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
      // go haywire when someone changes the KMyMoneyCategory object ...
      w = d->m_parent->cellWidget(d->m_startRow + 1, (int)eTransaction::Column::Detail);
      tabOrderWidgets.append(focusWidget(w));
      w = w->findChild<QPushButton*>("splitButton");
      if (w)
        tabOrderWidgets.append(w);
    } else if (*it_s == "tag") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 2, (int)eTransaction::Column::Detail)));
    } else if (*it_s == "memo") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 3, (int)eTransaction::Column::Detail)));
    } else if (*it_s == "payment") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Payment)));
    } else if (*it_s == "deposit") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Deposit)));
    } else if (*it_s == "state") {
      tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 1, (int)eTransaction::Column::Date)));
    }
    ++it_s;
  }
}

int StdTransaction::numRowsRegister(bool expanded) const
{
  Q_D(const StdTransaction);
  int numRows = 1;
  if (expanded) {
    numRows = 4;
    if (!d->m_inEdit) {
      //When not in edit Tags haven't a separate row;
      numRows--;
      if (d->m_payee.isEmpty()) {
        numRows--;
      }
      if (d->m_split.memo().isEmpty()) {
        numRows--;
      }
      // For income and expense accounts that only have
      // two splits we only show one line, because the
      // account name is already contained in the account column.
      if (d->m_account.accountType() == eMyMoney::Account::Income
          || d->m_account.accountType() == eMyMoney::Account::Expense) {
        if (numRows > 2 && d->m_transaction.splitCount() == 2)
          numRows = 1;
      }
    }
  }
  return numRows;
}

int StdTransaction::numRowsRegister() const
{
  return RegisterItem::numRowsRegister();
}

TransactionEditor* StdTransaction::createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate)
{
#ifndef KMM_DESIGNER
  Q_D(StdTransaction);
  d->m_inRegisterEdit = regForm == d->m_parent;
  return new StdTransactionEditor(regForm, this, list, lastPostDate);
#else
  return NULL;
#endif
}
