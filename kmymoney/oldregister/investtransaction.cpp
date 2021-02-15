/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "investtransaction.h"
#include "investtransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QWidget>
#include <QList>
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
#include "register.h"
#include "transactionform.h"
#include "kmymoneylineedit.h"
#include "kmymoneycombo.h"
#include "investtransactioneditor.h"
#include "amountedit.h"

#include "kmymoneysettings.h"
#include "widgetenums.h"

using namespace eWidgets;
using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

InvestTransaction::InvestTransaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  Transaction(*new InvestTransactionPrivate, parent, transaction, split, uniqueId)
{
  Q_D(InvestTransaction);
#ifndef KMM_DESIGNER
  // dissect the transaction into its type, splits, currency, security etc.
  MyMoneyUtils::dissectTransaction(d->m_transaction, d->m_split,
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
  setNumRowsRegister(numRowsRegister(KMyMoneySettings::showRegisterDetailed()));

  emit parent->itemAdded(this);
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
      txt = i18nc("Add securities/shares/bonds", "Add shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
      txt = i18nc("Remove securities/shares/bonds", "Remove shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
      txt = i18nc("Buy securities/shares/bonds", "Buy shares");
      break;
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
      txt = i18nc("Sell securities/shares/bonds", "Sell shares");
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
      txt = i18nc("Split securities/shares/bonds", "Split shares");
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
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Activity");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          activity(txt, d->m_transactionType);
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          txt = i18n("Date");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          fieldEditable = true;
          if (!d->m_transaction.id().isEmpty())
            txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;
      }
      break;

    case 1:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Security");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if (d->m_account.isInvest())
            txt = d->m_security.name();
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (haveShares()) {
            txt = i18n("Shares");
          } else if (haveSplitRatio()) {
            txt = i18n("Ratio");
          }
          break;

        case (int)eTransactionForm::Column::Value2:
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
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          if (haveAssetAccount())
            txt = i18n("Account");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveAssetAccount()) == true) {
            txt = MyMoneyFile::instance()->accountToCategory(d->m_assetAccountSplit.accountId());
          }
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (havePrice())
            txt = i18n("Price/share");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          if ((fieldEditable = havePrice()) == true && !d->m_split.shares().isZero()) {
            txt = d->m_split.price().formatMoney(QString(), d->m_security.pricePrecision());
          }
          break;
      }
      break;

    case 3:
      switch (col) {
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          if (haveFees())
            txt = i18n("Fees");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveFees()) == true) {
            txt = d->m_feeCategory;
          }
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (haveFees() && !d->m_feeCategory.isEmpty())
            txt = i18n("Fee Amount");
          break;

        case (int)eTransactionForm::Column::Value2:
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
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          if (haveInterest())
            txt = i18n("Interest");
          break;

        case (int)eTransactionForm::Column::Value1:
          align |= Qt::AlignLeft;
          if ((fieldEditable = haveInterest()) == true) {
            txt = d->m_interestCategory;
          }
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (haveInterest() && !d->m_interestCategory.isEmpty())
            txt = i18n("Interest");
          break;

        case (int)eTransactionForm::Column::Value2:
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
        case (int)eTransactionForm::Column::Label1:
          align |= Qt::AlignLeft;
          txt = i18n("Memo");
          break;

        case (int)eTransactionForm::Column::Value1:
          align &= ~Qt::AlignVCenter;
          align |= Qt::AlignTop;
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if (!d->m_transaction.id().isEmpty())
            txt = d->m_split.memo().section('\n', 0, 2);
          break;

        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          if (haveAmount())
            txt = i18nc("Total balance", "Total");
          break;

        case (int)eTransactionForm::Column::Value2:
          align |= Qt::AlignRight;
          if ((fieldEditable = haveAmount()) == true) {
            txt = d->m_assetAccountSplit.value().abs()
                .formatMoney(d->m_currency.tradingSymbol(), MyMoneyMoney::denomToPrec(d->m_currency.smallestAccountFraction()));
          }
      }
      break;

    case 6:
      switch (col) {
        case (int)eTransactionForm::Column::Label2:
          align |= Qt::AlignLeft;
          txt = i18n("Status");
          break;

        case (int)eTransactionForm::Column::Value2:
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
        case (int)eTransaction::Column::Date:
          align |= Qt::AlignLeft;
          txt = QLocale().toString(d->m_transaction.postDate(), QLocale::ShortFormat);
          break;

        case (int)eTransaction::Column::Detail:
          align |= Qt::AlignLeft;
          activity(txt, d->m_transactionType);
          break;

        case (int)eTransaction::Column::Security:
          align |= Qt::AlignLeft;
          if (d->m_account.isInvest())
            txt = d->m_security.name();
          break;

        case (int)eTransaction::Column::ReconcileFlag:
          align |= Qt::AlignHCenter;
          txt = reconcileState(false);
          break;

        case (int)eTransaction::Column::Quantity:
          align |= Qt::AlignRight;
          if (haveShares())
            txt = d->m_split.shares().abs().formatMoney(QString(), MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
          else if (haveSplitRatio()) {
            txt = QString("1 / %1").arg(d->m_split.shares().abs().formatMoney(QString(), -1));
          }
          break;

        case (int)eTransaction::Column::Price:
          align |= Qt::AlignRight;
          if (havePrice() && !d->m_split.shares().isZero()) {
            txt = d->m_split.price().formatMoney(d->m_currency.tradingSymbol(), d->m_security.pricePrecision());
          }
          break;

        case (int)eTransaction::Column::Value:
          align |= Qt::AlignRight;
          if (haveAmount()) {
            txt = MyMoneyUtils::formatMoney(d->m_assetAccountSplit.value().abs(), d->m_currency);

          } else if (haveInterest()) {
            txt = MyMoneyUtils::formatMoney(-d->m_interestAmount, d->m_currency);
          }
          break;

        case (int)eTransaction::Column::Balance:
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
        case (int)eTransaction::Column::Detail:
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

        case (int)eTransaction::Column::Quantity:
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
        case (int)eTransaction::Column::Detail:
          align |= Qt::AlignLeft;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()) {
            txt = d->m_interestCategory;
          } else if (haveFees() && d->m_feeSplits.count()) {
            txt = d->m_feeCategory;
          } else
            singleLineMemo(txt, d->m_split);
          break;

        case (int)eTransaction::Column::Quantity:
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
        case (int)eTransaction::Column::Detail:
          align |= Qt::AlignLeft;
          if (haveAssetAccount() && !d->m_assetAccountSplit.accountId().isEmpty()
              && haveInterest() && d->m_interestSplits.count()
              && haveFees() && d->m_feeSplits.count()) {
            txt = d->m_feeCategory;
          } else
            singleLineMemo(txt, d->m_split);
          break;

        case (int)eTransaction::Column::Quantity:
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
        txt = (m_split.value() / m_split.shares()).formatMoney(QString(), KMyMoneySettings::pricePrecision());
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
  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Value1, editWidgets["activity"]);
  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Value2, editWidgets["postdate"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Value1, editWidgets["security"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Value2, editWidgets["shares"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Value1, editWidgets["asset-account"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Value2, editWidgets["price"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Value1, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Value2, editWidgets["fee-amount"]);
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Value1, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Value2, editWidgets["interest-amount"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Value1, editWidgets["memo"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Value2, editWidgets["total"]);
  arrangeWidget(d->m_form, 6, (int)eTransactionForm::Column::Value2, editWidgets["status"]);

  // arrange dynamic labels
  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Label1, editWidgets["activity-label"]);
  arrangeWidget(d->m_form, 0, (int)eTransactionForm::Column::Label2, editWidgets["postdate-label"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Label1, editWidgets["security-label"]);
  arrangeWidget(d->m_form, 1, (int)eTransactionForm::Column::Label2, editWidgets["shares-label"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Label1, editWidgets["asset-label"]);
  arrangeWidget(d->m_form, 2, (int)eTransactionForm::Column::Label2, editWidgets["price-label"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Label1, editWidgets["fee-label"]);
  arrangeWidget(d->m_form, 3, (int)eTransactionForm::Column::Label2, editWidgets["fee-amount-label"]);
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Label1, editWidgets["interest-label"]);
  arrangeWidget(d->m_form, 4, (int)eTransactionForm::Column::Label2, editWidgets["interest-amount-label"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Label1, editWidgets["memo-label"]);
  arrangeWidget(d->m_form, 5, (int)eTransactionForm::Column::Label2, editWidgets["total-label"]);
  arrangeWidget(d->m_form, 6, (int)eTransactionForm::Column::Label2, editWidgets["status-label"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for (it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    KMyMoneyLineEdit* lineedit = dynamic_cast<KMyMoneyLineEdit*>(*it);
    AmountEdit* edit = dynamic_cast<AmountEdit*>(*it);
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
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(0, (int)eTransactionForm::Column::Value1)));

  // date
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(0, (int)eTransactionForm::Column::Value2)));

  // security
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, (int)eTransactionForm::Column::Value1)));

  // shares
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(1, (int)eTransactionForm::Column::Value2)));

  // account
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(2, (int)eTransactionForm::Column::Value1)));

  // price
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(2, (int)eTransactionForm::Column::Value2)));

  // make sure to have the fee category field and the split button as separate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  QWidget* w = d->m_form->cellWidget(3, (int)eTransactionForm::Column::Value1);
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(3, (int)eTransactionForm::Column::Value2)));

  // the same applies for the interest categories
  w = d->m_form->cellWidget(4, (int)eTransactionForm::Column::Value1);
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(4, (int)eTransactionForm::Column::Value2)));

  // memo
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(5, (int)eTransactionForm::Column::Value1)));

  // state
  tabOrderWidgets.append(focusWidget(d->m_form->cellWidget(6, (int)eTransactionForm::Column::Value2)));
}

void InvestTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(InvestTransaction);
  if (!d->m_parent)
    return;

  setupRegisterPalette(editWidgets);

  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Date, editWidgets["postdate"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Security, editWidgets["security"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Detail, editWidgets["activity"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, (int)eTransaction::Column::Detail, editWidgets["asset-account"]);
  arrangeWidget(d->m_parent, d->m_startRow + 2, (int)eTransaction::Column::Detail, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(d->m_parent, d->m_startRow + 3, (int)eTransaction::Column::Detail, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(d->m_parent, d->m_startRow + 4, (int)eTransaction::Column::Detail, editWidgets["memo"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Quantity, editWidgets["shares"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Price, editWidgets["price"]);
  arrangeWidget(d->m_parent, d->m_startRow + 2, (int)eTransaction::Column::Quantity, editWidgets["interest-amount"]);
  arrangeWidget(d->m_parent, d->m_startRow + 3, (int)eTransaction::Column::Quantity, editWidgets["fee-amount"]);
  arrangeWidget(d->m_parent, d->m_startRow + 0, (int)eTransaction::Column::Value, editWidgets["total"]);
  arrangeWidget(d->m_parent, d->m_startRow + 1, (int)eTransaction::Column::Date, editWidgets["status"]);

  // increase the height of the row containing the memo widget
  d->m_parent->setRowHeight(d->m_startRow + 4, d->m_parent->rowHeightHint() * 3);
}

void InvestTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  Q_D(const InvestTransaction);
  QWidget* w;

  // date
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Date)));
  // security
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Security)));
  // activity
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Detail)));
  // shares
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Quantity)));
  // price
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 0, (int)eTransaction::Column::Price)));
  // asset account
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 1, (int)eTransaction::Column::Detail)));

  // make sure to have the category fields and the split button as separate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  w = d->m_parent->cellWidget(d->m_startRow + 2, (int)eTransaction::Column::Detail);    // interest account
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 2, (int)eTransaction::Column::Quantity)));

  w = d->m_parent->cellWidget(d->m_startRow + 3, (int)eTransaction::Column::Detail);    // fee account
  tabOrderWidgets.append(focusWidget(w));
  w = w->findChild<QPushButton*>("splitButton");
  if (w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 3, (int)eTransaction::Column::Quantity)));

  // memo
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 4, (int)eTransaction::Column::Detail)));

  // status
  tabOrderWidgets.append(focusWidget(d->m_parent->cellWidget(d->m_startRow + 1, (int)eTransaction::Column::Date)));
}

eRegister::Action InvestTransaction::actionType() const
{
  return eRegister::Action::None;
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
  return new OldInvestTransactionEditor(regForm, this, list, lastPostDate);
#else
  return NULL;
#endif
}

