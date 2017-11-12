/***************************************************************************
                          investtransaction.cpp  -  description
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

#include "investtransaction.h"
#include "investtransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPainter>
#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypayeecombo.h"
#include "transaction.h"

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "register.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "transactionform.h"
#include "kmymoneylineedit.h"
#include "kmymoneyedit.h"
#include "transactioneditor.h"
#include "investtransactioneditor.h"
#include "kmymoneyutils.h"
#include "kmymoneymvccombo.h"
#ifndef KMM_DESIGNER
#include "stdtransactioneditor.h"
#endif

#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

InvestTransaction::InvestTransaction() :
  Transaction(*new InvestTransactionPrivate, nullptr, MyMoneyTransaction(), MyMoneySplit(), 0)
{
}

InvestTransaction::InvestTransaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  Transaction(*new InvestTransactionPrivate, parent, transaction, split, uniqueId)
{
  Q_D(InvestTransaction);
#ifndef KMM_DESIGNER
  // dissect the transaction into its type, splits, currency, security etc.
  KMyMoneyUtils::dissectTransaction(d->m_transaction, d->m_split,
                                    d->m_assetAccountSplit,
                                    d->m_feeSplits,
                                    d->m_interestSplits,
                                    d->m_security,
                                    d->m_currency,
                                    d->m_transactionType);
#endif

  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = d->m_feeSplits.constBegin(); it_s != d->m_feeSplits.constEnd(); ++it_s) {
    d->m_feeAmount += (*it_s).value();
  }
  for (it_s = d->m_interestSplits.constBegin(); it_s != d->m_interestSplits.constEnd(); ++it_s) {
    d->m_interestAmount += (*it_s).value();
  }

  // check the count of the fee splits and setup the text
  switch (d->m_feeSplits.count()) {
    case 0:
      break;

    case 1:
      d->m_feeCategory = MyMoneyFile::instance()->accountToCategory(d->m_feeSplits[0].accountId());
      break;

    default:
      d->m_feeCategory = i18nc("Split transaction (category replacement)", "Split transaction");
      break;
  }

  // check the count of the interest splits and setup the text
  switch (d->m_interestSplits.count()) {
    case 0:
      break;

    case 1:
      d->m_interestCategory = MyMoneyFile::instance()->accountToCategory(d->m_interestSplits[0].accountId());
      break;

    default:
      d->m_interestCategory = i18nc("Split transaction (category replacement)", "Split transaction");
      break;
  }

  d->m_rowsForm = 7;

  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));

  emit parent->itemAdded(this);
}

InvestTransaction::InvestTransaction(const InvestTransaction& other) :
  Transaction(*new InvestTransactionPrivate(*other.d_func()))
{
}

InvestTransaction::~InvestTransaction()
{
}

const QString InvestTransaction::sortSecurity() const
{
  Q_D(const InvestTransaction);
  return d->m_security.name();
}

const char* InvestTransaction::className()
{
  return "InvestTransaction";
}

void InvestTransaction::setupForm(TransactionForm* form)
{
  Transaction::setupForm(form);
  form->setSpan(5, 1, 2, 1);
}

void InvestTransaction::activity(QString& txt, eMyMoney::Split::InvestmentTransactionType type) const
{
  switch (type) {
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
      txt = i18n("Add shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
      txt = i18n("Remove shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
      txt = i18n("Buy shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
      txt = i18n("Sell shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
      txt = i18n("Dividend");
      break;
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
      txt = i18n("Reinvest Dividend");
      break;
    case eMyMoney::Split::InvestmentTransactionType::Yield:
      txt = i18n("Yield");
      break;
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
      txt = i18n("Split shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      txt = i18n("Interest Income");
      break;
    default:
      txt = i18nc("Unknown investment activity", "Unknown");
      break;
  }
}

bool InvestTransaction::formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* /* painter */)
{
  Q_D(InvestTransaction);
  bool fieldEditable = false;

  switch (row) {
    case 0:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Activity");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          activity(txt, d->m_transactionType);
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          txt = i18n("Date");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          fieldEditable = true;
          if (d->m_transaction != MyMoneyTransaction())
            txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;
      }
      break;

    case 1:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Security");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if (d->m_account.isInvest())
            txt = d->m_security.name();
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if (haveShares()) {
            txt = i18n("Shares");
          } else if (haveSplitRatio()) {
            txt = i18n("Ratio");
          }
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if ((fieldEditable = haveShares()) == true) {
            txt = d->m_split.shares().abs().formatMoney(QString(), MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
          } else if (haveSplitRatio()) {
            txt = QString("1 / %1").arg(d->m_split.shares().abs().formatMoney(QString(), -1));
          }
          break;
      }
      break;

    case 2:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if (haveAssetAccount())
            txt = i18n("Account");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveAssetAccount()) == true) {
            txt = MyMoneyFile::instance()->accountToCategory(d->m_assetAccountSplit.accountId());
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if (havePrice())
            txt = i18n("Price/share");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if ((fieldEditable = havePrice()) == true && !d->m_split.shares().isZero()) {
            txt = d->m_split.price().formatMoney(QString(), d->m_security.pricePrecision());
          }
          break;
      }
      break;

    case 3:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if (haveFees())
            txt = i18n("Fees");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveFees()) == true) {
            txt = d->m_feeCategory;
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if (haveFees() && !d->m_feeCategory.isEmpty())
            txt = i18n("Fee Amount");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if (haveFees()) {
            if ((fieldEditable = !d->m_feeCategory.isEmpty()) == true) {
              txt = MyMoneyUtils::formatMoney(d->m_feeAmount, d->m_currency);
            }
          }
          break;
      }
      break;

    case 4:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if (haveInterest())
            txt = i18n("Interest");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveInterest()) == true) {
            txt = d->m_interestCategory;
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if (haveInterest() && !d->m_interestCategory.isEmpty())
            txt = i18n("Interest");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if (haveInterest()) {
            if ((fieldEditable = !d->m_interestCategory.isEmpty()) == true) {
              txt = MyMoneyUtils::formatMoney(-d->m_interestAmount, d->m_currency);
            }
          }
          break;
      }
      break;

    case 5:
      switch (col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Memo");
          break;

        case ValueColumn1:
          align &= ~Qt::AlignVCenter;
          align |= Qt::AlignTop;
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if (d->m_transaction != MyMoneyTransaction())
            txt = d->m_split.memo().section('\n', 0, 2);
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if (haveAmount())
            txt = i18nc("Total balance", "Total");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if ((fieldEditable = haveAmount()) == true) {
            txt = d->m_assetAccountSplit.value().abs()
                .formatMoney(d->m_currency.tradingSymbol(), MyMoneyMoney::denomToPrec(d->m_currency.smallestAccountFraction()));
          }
      }
      break;

    case 6:
      switch (col) {
        case LabelColumn2:
          align |= Qt::AlignLeft;
          txt = i18n("Status");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          fieldEditable = true;
          txt = reconcileState();
          break;
      }
  }

  return fieldEditable;
}

void InvestTransaction::registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* /* painter */)
{
  Q_D(InvestTransaction);
  switch (row) {
    case 0:
      switch (col) {
        case DateColumn:
          align |= Qt::AlignLeft;
          txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;

        case DetailColumn:
          align |= Qt::AlignLeft;
          activity(txt, d->m_transactionType);
          break;

        case SecurityColumn:
          align |= Qt::AlignLeft;
          if (d->m_account.isInvest())
            txt = d->m_security.name();
          break;

        case ReconcileFlagColumn:
          align |= Qt::AlignHCenter;
          txt = reconcileState(false);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if (haveShares())
            txt = d->m_split.shares().abs().formatMoney(QString(), MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
          else if (haveSplitRatio()) {
            txt = QString("1 / %1").arg(d->m_split.shares().abs().formatMoney(QString(), -1));
          }
          break;

        case PriceColumn:
          align |= Qt::AlignRight;
          if (havePrice() && !d->m_split.shares().isZero()) {
            txt = d->m_split.price().formatMoney(d->m_currency.tradingSymbol(), d->m_security.pricePrecision());
          }
          break;

        case ValueColumn:
          align |= Qt::AlignRight;
          if (haveAmount()) {
            txt = MyMoneyUtils::formatMoney(d->m_assetAccountSplit.value().abs(), d->m_currency);

          } else if (haveInterest()) {
            txt = MyMoneyUtils::formatMoney(-d->m_interestAmount, d->m_currency);
          }
          break;

        case BalanceColumn:
          align |= Qt::AlignRight;
          if (d->m_showBalance)
            txt = d->m_balance.formatMoney(QString(), MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
          else
            txt = "----";
          break;

        default:
          break;
      }
      break;

    case 1:
      switch (col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()) {
            txt = MyMoneyFile::instance()->accountToCategory(d->m_assetAccountSplit.accountId());
          } else if (haveInterest() && d->m_interestSplits.count()) {
            txt = d->m_interestCategory;
          } else if (haveFees() && d->m_feeSplits.count()) {
            txt = d->m_feeCategory;
          } else
            singleLineMemo(txt, d->m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()) {
            // txt = m_interestAmount.abs().formatMoney(m_currency);
          } else if (haveInterest() && d->m_interestSplits.count()) {
            txt = MyMoneyUtils::formatMoney(-d->m_interestAmount, d->m_currency);
          } else if (haveFees() && d->m_feeSplits.count()) {
            txt = MyMoneyUtils::formatMoney(d->m_feeAmount, d->m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 2:
      switch (col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()) {
            txt = d->m_interestCategory;
          } else if (haveFees() && d->m_feeSplits.count()) {
            txt = d->m_feeCategory;
          } else
            singleLineMemo(txt, d->m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()) {
            txt = MyMoneyUtils::formatMoney(-d->m_interestAmount, d->m_currency);
          } else if (haveFees() && d->m_feeSplits.count()) {
            txt = MyMoneyUtils::formatMoney(d->m_feeAmount, d->m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 3:
      switch (col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()
              && haveFees() && d->m_feeSplits.count()) {
            txt = d->m_feeCategory;
          } else
            singleLineMemo(txt, d->m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()
              && haveFees() && d->m_feeSplits.count()) {
            txt = MyMoneyUtils::formatMoney(d->m_feeAmount, d->m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 4:
      switch (col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          singleLineMemo(txt, d->m_split);
          break;

        default:
          break;
      }
      break;
  }
}

int InvestTransaction::registerColWidth(int col, const QFontMetrics& cellFontMetrics)
{
  Q_D(InvestTransaction);
  QString txt;
  MyMoneyMoney amount;
  int nw = 0;

  // for now just check all rows in that column
  for (int row = 0; row < d->m_rowsRegister; ++row) {
    int w;
    Transaction::registerCellText(txt, row, col);
    w = cellFontMetrics.width(txt + "  ");
    nw = qMax(nw, w);
  }

  // TODO the optimized way would be to base the size on the contents of a single row
  //      as we do it in StdTransaction::registerColWidth()
#if 0
  switch (col) {
    default:
      break;

    case PriceColumn:
      if (havePrice()) {
        txt = (m_split.value() / m_split.shares()).formatMoney(QString(), KMyMoneyGlobalSettings::pricePrecision());
        nw = cellFontMetrics.width(txt + "  ");
      }
      break;
  }
#endif
  return nw;
}

void InvestTransaction::loadTab(KMyMoneyTransactionForm::TransactionForm* /* form */)
{
}

int InvestTransaction::numColsForm() const
{
  return 4;
}

void InvestTransaction::arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(InvestTransaction);
  if (!d->m_form || !d->m_parent)
    return;

  setupFormPalette(editWidgets);

  // arrange the edit widgets
  arrangeWidget(d->m_form, 0, ValueColumn1, editWidgets["activity"]);
  arrangeWidget(d->m_form, 0, ValueColumn2, editWidgets["postdate"]);
  arrangeWidget(d->m_form, 1, ValueColumn1, editWidgets["security"]);
  arrangeWidget(d->m_form, 1, ValueColumn2, editWidgets["shares"]);
  arrangeWidget(d->m_form, 2, ValueColumn1, editWidgets["asset-account"]);
  arrangeWidget(d->m_form, 2, ValueColumn2, editWidgets["price"]);
  arrangeWidget(d->m_form, 3, ValueColumn1, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(d->m_form, 3, ValueColumn2, editWidgets["fee-amount"]);
  arrangeWidget(d->m_form, 4, ValueColumn1, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(d->m_form, 4, ValueColumn2, editWidgets["interest-amount"]);
  arrangeWidget(d->m_form, 5, ValueColumn1, editWidgets["memo"]);
  arrangeWidget(d->m_form, 5, ValueColumn2, editWidgets["total"]);
  arrangeWidget(d->m_form, 6, ValueColumn2, editWidgets["status"]);

  // arrange dynamic labels
  arrangeWidget(d->m_form, 0, LabelColumn1, editWidgets["activity-label"]);
  arrangeWidget(d->m_form, 0, LabelColumn2, editWidgets["postdate-label"]);
  arrangeWidget(d->m_form, 1, LabelColumn1, editWidgets["security-label"]);
  arrangeWidget(d->m_form, 1, LabelColumn2, editWidgets["shares-label"]);
  arrangeWidget(d->m_form, 2, LabelColumn1, editWidgets["asset-label"]);
  arrangeWidget(d->m_form, 2, LabelColumn2, editWidgets["price-label"]);
  arrangeWidget(d->m_form, 3, LabelColumn1, editWidgets["fee-label"]);
  arrangeWidget(d->m_form, 3, LabelColumn2, editWidgets["fee-amount-label"]);
  arrangeWidget(d->m_form, 4, LabelColumn1, editWidgets["interest-label"]);
  arrangeWidget(d->m_form, 4, LabelColumn2, editWidgets["interest-amount-label"]);
  arrangeWidget(d->m_form, 5, LabelColumn1, editWidgets["memo-label"]);
  arrangeWidget(d->m_form, 5, LabelColumn2, editWidgets["total-label"]);
  arrangeWidget(d->m_form, 6, LabelColumn2, editWidgets["status-label"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for (it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    kMyMoneyLineEdit* lineedit = dynamic_cast<kMyMoneyLineEdit*>(*it);
    kMyMoneyEdit* edit = dynamic_cast<kMyMoneyEdit*>(*it);
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it);
    if (combo)
      combo->setPlaceholderText(QString());
    if (edit)
      edit->setPlaceholderText(QString());
    if (lineedit)
      lineedit->setPlaceholderText(QString());
    if (payee)
      payee->setPlaceholderText(QString());
  }
}

void InvestTransaction::tabOrderInForm(QWidgetList& tabOrderWidgets) const
{
  Q_D(const InvestTransaction);
  // activity
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(0, ValueColumn1)));

  // date
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(0, ValueColumn2)));

  // security
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, ValueColumn1)));

  // shares
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, ValueColumn2)));

  // account
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(2, ValueColumn1)));

  // price
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(2, ValueColumn2)));

  // make sure to have the fee category field and the split button as separate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  QWidget* w = d->m_form->cellWidget(3, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(3, ValueColumn2)));

  // the same applies for the interest categories
  w = d->m_form->cellWidget(4, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(4, ValueColumn2)));

  // memo
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(5, ValueColumn1)));

  // state
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(6, ValueColumn2)));
}

void InvestTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(InvestTransaction);
  if (!d->m_parent)
    return;

  setupRegisterPalette(editWidgets);

  arrangeWidget(d->m_parent, d->m_startRow + 0, DateColumn, editWidgets["postdate"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, SecurityColumn, editWidgets["security"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, DetailColumn, editWidgets["activity"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, DetailColumn, editWidgets["asset-account"]);
  arrangeWidget(d->m_parent, d->m_startRow + 2, DetailColumn, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(d->m_parent, d->m_startRow + 3, DetailColumn, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(d->m_parent, d->m_startRow + 4, DetailColumn, editWidgets["memo"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, QuantityColumn, editWidgets["shares"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, PriceColumn, editWidgets["price"]);
  arrangeWidget(d->m_parent, d->m_startRow + 2, QuantityColumn, editWidgets["interest-amount"]);
  arrangeWidget(d->m_parent, d->m_startRow + 3, QuantityColumn, editWidgets["fee-amount"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, ValueColumn, editWidgets["total"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, DateColumn, editWidgets["status"]);

  // increase the height of the row containing the memo widget
  d->m_parent->setRowHeight(d->m_startRow + 4, d->m_parent->rowHeightHint() * 3);
}

void InvestTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  Q_D(const InvestTransaction);
  QWidget* w;

  // date
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, DateColumn)));
  // security
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, SecurityColumn)));
  // activity
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, DetailColumn)));
  // shares
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, QuantityColumn)));
  // price
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, PriceColumn)));
  // asset account
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 1, DetailColumn)));

  // make sure to have the category fields and the split button as separate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  w = d->m_parent->cellWidget(d->m_startRow + 2, DetailColumn);    // interest account
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 2, QuantityColumn)));

  w = d->m_parent->cellWidget(d->m_startRow + 3, DetailColumn);    // fee account
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 3, QuantityColumn)));

  // memo
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 4, DetailColumn)));

  // status
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 1, DateColumn)));
}

KMyMoneyRegister::Action InvestTransaction::actionType() const
{
  return KMyMoneyRegister::ActionNone;
}

int InvestTransaction::numRowsRegister(bool expanded) const
{
  Q_D(const InvestTransaction);
  int numRows = 1;
  if (expanded) {
    if (!d->m_inEdit) {
      if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty())
        ++numRows;
      if (haveInterest() && d->m_interestSplits.count())
        ++numRows;
      if (haveFees() && d->m_feeSplits.count())
        ++numRows;
      if (!d->m_split.memo().isEmpty())
        ++numRows;
    } else
      numRows = 5;
  }
  return numRows;
}

bool InvestTransaction::haveShares() const
{
  Q_D(const InvestTransaction);
  auto rc = true;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
    case eMyMoney::Split::InvestmentTransactionType::Yield:
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveFees() const
{
  Q_D(const InvestTransaction);
  auto rc = true;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveInterest() const
{
  Q_D(const InvestTransaction);
  auto rc = false;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
    case eMyMoney::Split::InvestmentTransactionType::Yield:
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::havePrice() const
{
  Q_D(const InvestTransaction);
  auto rc = false;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveAmount() const
{
  Q_D(const InvestTransaction);
  auto rc = false;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
    case eMyMoney::Split::InvestmentTransactionType::Yield:
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveAssetAccount() const
{
  Q_D(const InvestTransaction);
  auto rc = true;
  switch (d->m_transactionType) {
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveSplitRatio() const
{
  Q_D(const InvestTransaction);
  return d->m_transactionType == eMyMoney::Split::InvestmentTransactionType::SplitShares;
}

void InvestTransaction::splits(MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& feeSplits) const
{
  Q_D(const InvestTransaction);
  assetAccountSplit = d->m_assetAccountSplit;
  interestSplits = d->m_interestSplits;
  feeSplits = d->m_feeSplits;
}

int InvestTransaction::numRowsRegister() const
{
  return RegisterItem::numRowsRegister();
}

TransactionEditor* InvestTransaction::createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate)
{
#ifndef KMM_DESIGNER
  Q_D(InvestTransaction);
  d->m_inRegisterEdit = regForm == d->m_parent;
  return new InvestTransactionEditor(regForm, this, list, lastPostDate);
#else
  return NULL;
#endif
}

