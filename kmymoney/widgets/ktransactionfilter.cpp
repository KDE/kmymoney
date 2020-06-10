/*
 * Copyright 2002-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include "ktransactionfilter.h"
#include "ktransactionfilter_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktransactionfilter.h"
#include "mymoneyreport.h"
#include "mymoneysplit.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneysettings.h"
#include "daterangedlg.h"

KTransactionFilter::KTransactionFilter(QWidget *parent, bool withEquityAccounts, bool withInvestments, bool withDataTab) :
  QWidget(parent),
  d_ptr(new KTransactionFilterPrivate(this))
{
  Q_D(KTransactionFilter);
  d->init(withEquityAccounts, withInvestments, withDataTab);
}

KTransactionFilter::~KTransactionFilter()
{
  Q_D(KTransactionFilter);
  delete d;
}

void KTransactionFilter::slotReset()
{
  Q_D(KTransactionFilter);
  d->ui->m_textEdit->setText(QString());
  d->ui->m_regExp->setChecked(false);
  d->ui->m_caseSensitive->setChecked(false);
  d->ui->m_textNegate->setCurrentItem(0);

  d->ui->m_amountEdit->setEnabled(true);
  d->ui->m_amountFromEdit->setEnabled(false);
  d->ui->m_amountToEdit->setEnabled(false);
  d->ui->m_amountEdit->setText(QString());
  d->ui->m_amountFromEdit->setText(QString());
  d->ui->m_amountToEdit->setText(QString());
  d->ui->m_amountButton->setChecked(true);
  d->ui->m_amountRangeButton->setChecked(false);

  d->ui->m_emptyPayeesButton->setChecked(false);
  d->selectAllItems(d->ui->m_payeesView, true);

  d->ui->m_emptyTagsButton->setChecked(false);
  d->selectAllItems(d->ui->m_tagsView, true);

  d->ui->m_typeBox->setCurrentIndex((int)eMyMoney::TransactionFilter::Type::All);
  d->ui->m_stateBox->setCurrentIndex((int)eMyMoney::TransactionFilter::State::All);
  d->ui->m_validityBox->setCurrentIndex((int)eMyMoney::TransactionFilter::Validity::Any);

  d->ui->m_nrEdit->setEnabled(true);
  d->ui->m_nrFromEdit->setEnabled(false);
  d->ui->m_nrToEdit->setEnabled(false);
  d->ui->m_nrEdit->setText(QString());
  d->ui->m_nrFromEdit->setText(QString());
  d->ui->m_nrToEdit->setText(QString());
  d->ui->m_nrButton->setChecked(true);
  d->ui->m_nrRangeButton->setChecked(false);

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  if (d->m_dateRange)
    d->m_dateRange->slotReset();
  slotUpdateSelections();

  d->ui->m_accountsView->slotSelectAllAccounts();
  d->ui->m_categoriesView->slotSelectAllAccounts();
}

void KTransactionFilter::slotUpdateSelections()
{
  Q_D(KTransactionFilter);
  QString txt;
  const QString separator(", ");
  // Text tab
  if (!d->ui->m_textEdit->text().isEmpty()) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Text");
    d->ui->m_regExp->setEnabled(QRegExp(d->ui->m_textEdit->text()).isValid());
  } else
    d->ui->m_regExp->setEnabled(false);

  d->ui->m_caseSensitive->setEnabled(!d->ui->m_textEdit->text().isEmpty());
  d->ui->m_textNegate->setEnabled(!d->ui->m_textEdit->text().isEmpty());

  // Account tab
  if (!d->ui->m_accountsView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Account");
  }

  if (d->m_dateRange && d->m_dateRange->dateRange() != eMyMoney::TransactionFilter::Date::All) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Date");
  }

  // Amount tab
  if ((d->ui->m_amountButton->isChecked() && d->ui->m_amountEdit->isValid())
      || (d->ui->m_amountRangeButton->isChecked()
          && (d->ui->m_amountFromEdit->isValid() || d->ui->m_amountToEdit->isValid()))) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Amount");
  }

  // Categories tab
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Category");
  }

  // Tags tab
  if (!d->allItemsSelected(d->ui->m_tagsView)
      || d->ui->m_emptyTagsButton->isChecked()) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Tags");
  }
  d->ui->m_tagsView->setEnabled(!d->ui->m_emptyTagsButton->isChecked());

  // Payees tab
  if (!d->allItemsSelected(d->ui->m_payeesView)
      || d->ui->m_emptyPayeesButton->isChecked()) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Payees");
  }
  d->ui->m_payeesView->setEnabled(!d->ui->m_emptyPayeesButton->isChecked());

  // Details tab
  if (d->ui->m_typeBox->currentIndex() != 0
      || d->ui->m_stateBox->currentIndex() != 0
      || d->ui->m_validityBox->currentIndex() != 0
      || (d->ui->m_nrButton->isChecked() && d->ui->m_nrEdit->text().length() != 0)
      || (d->ui->m_nrRangeButton->isChecked()
          && (d->ui->m_nrFromEdit->text().length() != 0 || d->ui->m_nrToEdit->text().length() != 0))) {
    if (!txt.isEmpty())
      txt += separator;
    txt += i18n("Details");
  }

  //Show a warning about transfers if Categories are filtered - bug #1523508
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    d->ui->m_transferWarning->setText(i18n("Warning: Filtering by Category will exclude all transfers from the results."));
  } else {
    d->ui->m_transferWarning->setText(QString());
  }

  // disable the search button if no selection is made
  emit selectionNotEmpty(!txt.isEmpty());

  if (txt.isEmpty()) {
    txt = i18nc("No selection", "(None)");
  }
  d->ui->m_selectedCriteria->setText(i18n("Current selections: %1", txt));
}

void KTransactionFilter::slotAmountSelected()
{
  Q_D(KTransactionFilter);
  d->ui->m_amountEdit->setEnabled(true);
  d->ui->m_amountFromEdit->setEnabled(false);
  d->ui->m_amountToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KTransactionFilter::slotAmountRangeSelected()
{
  Q_D(KTransactionFilter);
  d->ui->m_amountEdit->setEnabled(false);
  d->ui->m_amountFromEdit->setEnabled(true);
  d->ui->m_amountToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KTransactionFilter::slotSelectAllPayees()
{
  Q_D(KTransactionFilter);
  d->selectAllItems(d->ui->m_payeesView, true);
}

void KTransactionFilter::slotDeselectAllPayees()
{
  Q_D(KTransactionFilter);
  d->selectAllItems(d->ui->m_payeesView, false);
}

void KTransactionFilter::slotSelectAllTags()
{
  Q_D(KTransactionFilter);
  d->selectAllItems(d->ui->m_tagsView, true);
}

void KTransactionFilter::slotDeselectAllTags()
{
  Q_D(KTransactionFilter);
  d->selectAllItems(d->ui->m_tagsView, false);
}

void KTransactionFilter::slotNrSelected()
{
  Q_D(KTransactionFilter);
  d->ui->m_nrEdit->setEnabled(true);
  d->ui->m_nrFromEdit->setEnabled(false);
  d->ui->m_nrToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KTransactionFilter::slotNrRangeSelected()
{
  Q_D(KTransactionFilter);
  d->ui->m_nrEdit->setEnabled(false);
  d->ui->m_nrFromEdit->setEnabled(true);
  d->ui->m_nrToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KTransactionFilter::slotShowHelp()
{
  Q_D(KTransactionFilter);
  auto anchor = d->m_helpAnchor[d->ui->m_criteriaTab->currentWidget()];
  if (anchor.isEmpty())
    anchor = QString("details.search");

  KHelpClient::invokeHelp(anchor);
}

MyMoneyTransactionFilter KTransactionFilter::setupFilter()
{
  Q_D(KTransactionFilter);
  d->m_filter.clear();

  // Text tab
  if (!d->ui->m_textEdit->text().isEmpty()) {
    QRegExp exp(d->ui->m_textEdit->text(), d->ui->m_caseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, !d->ui->m_regExp->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp);
    d->m_filter.setTextFilter(exp, d->ui->m_textNegate->currentIndex() != 0);
  }

  // Account tab
  if (!d->ui->m_accountsView->allItemsSelected()) {
    // retrieve a list of selected accounts
    QStringList list;
    d->ui->m_accountsView->selectedItems(list);

    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected
    if (!KMyMoneySettings::expertMode()) {
      QStringList missing;
      foreach (const auto selection, list) {
        auto acc = MyMoneyFile::instance()->account(selection);
        if (acc.accountType() == eMyMoney::Account::Type::Investment) {
          foreach (const auto sAccount, acc.accountList()) {
            if (!list.contains(sAccount)) {
              missing.append(sAccount);
            }
          }
        }
      }
      list += missing;
    }

    d->m_filter.addAccount(list);
  }

  // Date tab
  if (d->m_dateRange && (int)d->m_dateRange->dateRange() != 0) {
    d->m_filter.setDateFilter(d->m_dateRange->fromDate(), d->m_dateRange->toDate());
  }

  // Amount tab
  if ((d->ui->m_amountButton->isChecked() && d->ui->m_amountEdit->isValid())) {
    d->m_filter.setAmountFilter(d->ui->m_amountEdit->value(), d->ui->m_amountEdit->value());

  } else if ((d->ui->m_amountRangeButton->isChecked()
              && (d->ui->m_amountFromEdit->isValid() || d->ui->m_amountToEdit->isValid()))) {

    MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
    if (d->ui->m_amountFromEdit->isValid())
      from = d->ui->m_amountFromEdit->value();
    if (d->ui->m_amountToEdit->isValid())
      to = d->ui->m_amountToEdit->value();

    d->m_filter.setAmountFilter(from, to);
  }

  // Categories tab
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    d->m_filter.addCategory(d->ui->m_categoriesView->selectedItems());
  }

  // Tags tab
  if (d->ui->m_emptyTagsButton->isChecked()) {
    d->m_filter.addTag(QString());

  } else if (!d->allItemsSelected(d->ui->m_tagsView)) {
    d->scanCheckListItems(d->ui->m_tagsView, KTransactionFilterPrivate::addTagToFilter);
  }

  // Payees tab
  if (d->ui->m_emptyPayeesButton->isChecked()) {
    d->m_filter.addPayee(QString());

  } else if (!d->allItemsSelected(d->ui->m_payeesView)) {
    d->scanCheckListItems(d->ui->m_payeesView, KTransactionFilterPrivate::addPayeeToFilter);
  }

  // Details tab
  if (d->ui->m_typeBox->currentIndex() != 0)
    d->m_filter.addType(d->ui->m_typeBox->currentIndex());

  if (d->ui->m_stateBox->currentIndex() != 0)
    d->m_filter.addState(d->ui->m_stateBox->currentIndex());

  if (d->ui->m_validityBox->currentIndex() != 0)
    d->m_filter.addValidity(d->ui->m_validityBox->currentIndex());

  if (d->ui->m_nrButton->isChecked() && !d->ui->m_nrEdit->text().isEmpty())
    d->m_filter.setNumberFilter(d->ui->m_nrEdit->text(), d->ui->m_nrEdit->text());

  if (d->ui->m_nrRangeButton->isChecked()
      && (!d->ui->m_nrFromEdit->text().isEmpty() || !d->ui->m_nrToEdit->text().isEmpty())) {
    d->m_filter.setNumberFilter(d->ui->m_nrFromEdit->text(), d->ui->m_nrToEdit->text());
  }
  return d->m_filter;
}

void KTransactionFilter::resetFilter(MyMoneyReport& rep)
{
  Q_D(KTransactionFilter);
  //
  // Text Filter
  //

  QRegExp textfilter;
  if (rep.textFilter(textfilter)) {
    d->ui->m_textEdit->setText(textfilter.pattern());
    d->ui->m_caseSensitive->setChecked(Qt::CaseSensitive == textfilter.caseSensitivity());
    d->ui->m_regExp->setChecked(QRegExp::RegExp == textfilter.patternSyntax());
    d->ui->m_textNegate->setCurrentIndex(rep.isInvertingText());
  }

  //
  // Type & State Filters
  //

  int type;
  if (rep.firstType(type))
    d->ui->m_typeBox->setCurrentIndex(type);

  int state;
  if (rep.firstState(state))
    d->ui->m_stateBox->setCurrentIndex(state);

  int validity;
  if (rep.firstValidity(validity))
    d->ui->m_validityBox->setCurrentIndex(validity);

  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (rep.numberFilter(nrFrom, nrTo)) {
    if (nrFrom == nrTo) {
      d->ui->m_nrEdit->setEnabled(true);
      d->ui->m_nrFromEdit->setEnabled(false);
      d->ui->m_nrToEdit->setEnabled(false);
      d->ui->m_nrEdit->setText(nrFrom);
      d->ui->m_nrFromEdit->setText(QString());
      d->ui->m_nrToEdit->setText(QString());
      d->ui->m_nrButton->setChecked(true);
      d->ui->m_nrRangeButton->setChecked(false);
    } else {
      d->ui->m_nrEdit->setEnabled(false);
      d->ui->m_nrFromEdit->setEnabled(true);
      d->ui->m_nrToEdit->setEnabled(false);
      d->ui->m_nrEdit->setText(QString());
      d->ui->m_nrFromEdit->setText(nrFrom);
      d->ui->m_nrToEdit->setText(nrTo);
      d->ui->m_nrButton->setChecked(false);
      d->ui->m_nrRangeButton->setChecked(true);
    }
  } else {
    d->ui->m_nrEdit->setEnabled(true);
    d->ui->m_nrFromEdit->setEnabled(false);
    d->ui->m_nrToEdit->setEnabled(false);
    d->ui->m_nrEdit->setText(QString());
    d->ui->m_nrFromEdit->setText(QString());
    d->ui->m_nrToEdit->setText(QString());
    d->ui->m_nrButton->setChecked(true);
    d->ui->m_nrRangeButton->setChecked(false);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (rep.amountFilter(from, to)) { // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    if (from == to) {
      d->ui->m_amountEdit->setEnabled(true);
      d->ui->m_amountFromEdit->setEnabled(false);
      d->ui->m_amountToEdit->setEnabled(false);
      d->ui->m_amountEdit->setText(QString::number(from.toDouble()));
      d->ui->m_amountFromEdit->setText(QString());
      d->ui->m_amountToEdit->setText(QString());
      d->ui->m_amountButton->setChecked(true);
      d->ui->m_amountRangeButton->setChecked(false);
    } else {
      d->ui->m_amountEdit->setEnabled(false);
      d->ui->m_amountFromEdit->setEnabled(true);
      d->ui->m_amountToEdit->setEnabled(true);
      d->ui->m_amountEdit->setText(QString());
      d->ui->m_amountFromEdit->setText(QString::number(from.toDouble()));
      d->ui->m_amountToEdit->setText(QString::number(to.toDouble()));
      d->ui->m_amountButton->setChecked(false);
      d->ui->m_amountRangeButton->setChecked(true);
    }
  } else {
    d->ui->m_amountEdit->setEnabled(true);
    d->ui->m_amountFromEdit->setEnabled(false);
    d->ui->m_amountToEdit->setEnabled(false);
    d->ui->m_amountEdit->setText(QString());
    d->ui->m_amountFromEdit->setText(QString());
    d->ui->m_amountToEdit->setText(QString());
    d->ui->m_amountButton->setChecked(true);
    d->ui->m_amountRangeButton->setChecked(false);
  }

  //
  // Payees Filter
  //

  QStringList payees;
  if (rep.payees(payees)) {
    if (payees.empty()) {
      d->ui->m_emptyPayeesButton->setChecked(true);
    } else {
      d->selectAllItems(d->ui->m_payeesView, false);
      d->selectItems(d->ui->m_payeesView, payees, true);
    }
  } else {
    d->selectAllItems(d->ui->m_payeesView, true);
  }

  //
  // Tags Filter
  //

  QStringList tags;
  if (rep.tags(tags)) {
    if (tags.empty()) {
      d->ui->m_emptyTagsButton->setChecked(true);
    } else {
      d->selectAllItems(d->ui->m_tagsView, false);
      d->selectItems(d->ui->m_tagsView, tags, true);
    }
  } else {
    d->selectAllItems(d->ui->m_tagsView, true);
  }

  //
  // Accounts Filter
  //

  QStringList accounts;
  if (rep.accounts(accounts)) {
    // in case the presentation of closed accounts is turned off ...
    if (d->accountSet.isHidingClosedAccounts()) {
      // ... we need to turn them on again in case our own
      // configuration references a closed account
      const MyMoneyFile* file = MyMoneyFile::instance();
      foreach(const auto accId, accounts) {
        try {
          if (file->account(accId).isClosed()) {
            d->accountSet.setHideClosedAccounts(false);
            d->accountSet.load(d->ui->m_accountsView);
            break;
          }
        } catch (const MyMoneyException&) {
        }
      }
    }
    d->ui->m_accountsView->selectAllItems(false);
    d->ui->m_accountsView->selectItems(accounts, true);
  } else
    d->ui->m_accountsView->selectAllItems(true);

  //
  // Categories Filter
  //

  if (rep.categories(accounts)) {
    d->ui->m_categoriesView->selectAllItems(false);
    d->ui->m_categoriesView->selectItems(accounts, true);
  } else
    d->ui->m_categoriesView->selectAllItems(true);

  //
  // Date Filter
  //

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last

  if (d->m_dateRange) {
    rep.updateDateFilter();
    QDate dateFrom, dateTo;
    if (rep.dateFilter(dateFrom, dateTo)) {
      if (rep.isDateUserDefined()) {
        d->m_dateRange->setDateRange(dateFrom, dateTo);
      } else {
        d->m_dateRange->setDateRange(rep.dateRange());
      }
    } else {
      d->m_dateRange->setDateRange(eMyMoney::TransactionFilter::Date::All);
    }
  }

}

KMyMoneyAccountSelector* KTransactionFilter::categoriesView()
{
  Q_D(KTransactionFilter);
  return d->ui->m_categoriesView;
}

DateRangeDlg* KTransactionFilter::dateRange()
{
  Q_D(KTransactionFilter);
  return d->m_dateRange;
}
