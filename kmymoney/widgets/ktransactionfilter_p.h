/*
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KTRANSACTIONFILTER_P_H
#define KTRANSACTIONFILTER_P_H

#include "ktransactionfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktransactionfilter.h"

#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "kmymoneysettings.h"
#include "transaction.h"
#include "daterangedlg.h"
#include "mymoneytransactionfilter.h"

#include "widgetenums.h"
#include "mymoneyenums.h"

class KTransactionFilterPrivate
{
  Q_DISABLE_COPY(KTransactionFilterPrivate)
  Q_DECLARE_PUBLIC(KTransactionFilter)

public:
  enum opTypeE {
    addAccountToFilter = 0,
    addCategoryToFilter,
    addPayeeToFilter,
    addTagToFilter
  };

  // the set of accounts to choose from
  AccountSet accountSet;

  explicit KTransactionFilterPrivate(KTransactionFilter *qq) :
    q_ptr(qq),
    ui(new Ui::KTransactionFilter),
    m_dateRange(nullptr)
  {
  }

  ~KTransactionFilterPrivate()
  {
    delete ui;
  }

  void init(bool withEquityAccounts, bool withDataTab)
  {
    Q_Q(KTransactionFilter);
    ui->setupUi(q);

    if (withDataTab) {
      m_dateRange = new DateRangeDlg;
      ui->dateRangeLayout->insertWidget(0, m_dateRange);
      // in case the date selection changes, we update the selection
      q->connect(m_dateRange, &DateRangeDlg::rangeChanged, q, &KTransactionFilter::slotUpdateSelections);
    } else {
      ui->m_criteriaTab->removeTab(ui->m_criteriaTab->indexOf(ui->m_dateTab));
      ui->m_dateTab->deleteLater();
    }

    // 'cause we don't have a separate setupTextPage
    q->connect(ui->m_textEdit, &QLineEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);

    setupAccountsPage(withEquityAccounts);
    setupCategoriesPage();
    setupAmountPage();
    setupPayeesPage();
    setupTagsPage();
    setupDetailsPage();

    // We don't need to add the default into the list (see ::slotShowHelp() why)
    // m_helpAnchor[m_ui->m_textTab] = QLatin1String("details.search");
    m_helpAnchor[ui->m_accountTab] = QLatin1String("details.search.account");
    m_helpAnchor[ui->m_dateTab] = QLatin1String("details.search.date");
    m_helpAnchor[ui->m_amountTab] = QLatin1String("details.search.amount");
    m_helpAnchor[ui->m_categoryTab] = QLatin1String("details.search.category");
    m_helpAnchor[ui->m_payeeTab] = QLatin1String("details.search.payee");
    m_helpAnchor[ui->m_tagTab] = QLatin1String("details.search.tag"); //FIXME-ALEX update Help
    m_helpAnchor[ui->m_detailsTab] = QLatin1String("details.search.details");

    q->slotUpdateSelections();

    ui->m_textEdit->setFocus();
  }

  /**
    * q method returns information about the selection state
    * of the items in the m_accountsView.
    *
    * @param view pointer to the listview to scan
    *
    * @retval true if all items in the view are marked
    * @retval false if at least one item is not marked
    *
    * @note If the view contains no items the method returns @p true.
    */
  bool allItemsSelected(const QTreeWidgetItem *item) const
  {
    QTreeWidgetItem* it_v;

    for (auto i = 0; i < item->childCount(); ++i) {
      it_v = item->child(i);
      if (!(it_v->checkState(0) == Qt::Checked && allItemsSelected(it_v))) {
        return false;
      }
    }
    return true;
  }

  bool allItemsSelected(const QTreeWidget* view) const
  {
    QTreeWidgetItem* it_v;

    for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
      it_v = view->invisibleRootItem()->child(i);
      if (it_v->flags() & Qt::ItemIsUserCheckable) {
        if (!(it_v->checkState(0) == Qt::Checked && allItemsSelected(it_v))) {
          return false;
        } else {
          if (!allItemsSelected(it_v))
            return false;
        }
      }
    }
    return true;
  }

  void addItemToFilter(const opTypeE op, const QString& id)
  {
    switch (op) {
      case addAccountToFilter:
        m_filter.addAccount(id);
        break;
      case addCategoryToFilter:
        m_filter.addCategory(id);
        break;
      case addPayeeToFilter:
        m_filter.addPayee(id);
        break;
      case addTagToFilter:
        m_filter.addTag(id);
        break;
    }
  }

  void scanCheckListItems(const QTreeWidgetItem* item, const opTypeE op)
  {
    QTreeWidgetItem* it_v;

    for (auto i = 0; i < item->childCount(); ++i) {
      it_v = item->child(i);
      QVariant idData = it_v->data(0, Qt::UserRole);
      if (it_v->flags() & Qt::ItemIsUserCheckable) {
        if (it_v->checkState(0) == Qt::Checked)
          addItemToFilter(op, idData.toString());
      }
      scanCheckListItems(it_v, op);
    }
  }

  void scanCheckListItems(const QTreeWidget* view, const opTypeE op)
  {
    QTreeWidgetItem* it_v;

    for (auto i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
      it_v = view->invisibleRootItem()->child(i);
      QVariant idData = it_v->data(0, Qt::UserRole);
      if (it_v->flags() & Qt::ItemIsUserCheckable) {
        if (it_v->checkState(0) == Qt::Checked) {
          addItemToFilter(op, idData.toString());
        }
      }
      scanCheckListItems(it_v, op);
    }
  }

  void selectAllItems(QTreeWidget* view, const bool state)
  {
    QTreeWidgetItem* it_v;

    for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
      it_v = view->invisibleRootItem()->child(i);
      if (it_v->flags() & Qt::ItemIsUserCheckable) {
        it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
      }
      selectAllSubItems(it_v, state);
    }
    Q_Q(KTransactionFilter);
    q->slotUpdateSelections();
  }

  void selectItems(QTreeWidget* view, const QStringList& list, const bool state)
  {
    QTreeWidgetItem* it_v;

    for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
      it_v = view->invisibleRootItem()->child(i);
      QVariant idData = it_v->data(0, Qt::UserRole);
      if (it_v->flags() & Qt::ItemIsUserCheckable && list.contains(idData.toString())) {
        it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
      }
      selectSubItems(it_v, list, state);
    }

    Q_Q(KTransactionFilter);
    q->slotUpdateSelections();
  }

  void selectAllSubItems(QTreeWidgetItem* item, const bool state)
  {
    QTreeWidgetItem* it_v;

    for (int i = 0; i < item->childCount(); ++i) {
      it_v = item->child(i);
      it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
      selectAllSubItems(it_v, state);
    }
  }

  void selectSubItems(QTreeWidgetItem* item, const QStringList& list, const bool state)
  {
    QTreeWidgetItem* it_v;

    for (int i = 0; i < item->childCount(); ++i) {
      it_v = item->child(i);
      QVariant idData = it_v->data(0, Qt::UserRole);
      if (list.contains(idData.toString()))
        it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
      selectSubItems(it_v, list, state);
    }
  }

  /**
    * q method loads the m_tagsView with the tags name
    * found in the engine.
    */
  void loadTags()
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    QList<MyMoneyTag> list;
    QList<MyMoneyTag>::Iterator it_l;

    list = file->tagList();
    // load view
    for (it_l = list.begin(); it_l != list.end(); ++it_l) {
      auto item = new QTreeWidgetItem(ui->m_tagsView);
      item->setText(0, (*it_l).name());
      item->setData(0, Qt::UserRole, (*it_l).id());
      item->setCheckState(0, Qt::Checked);
    }
  }

  /**
    * q method loads the m_payeesView with the payees name
    * found in the engine.
    */
  void loadPayees()
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    QList<MyMoneyPayee> list;
    QList<MyMoneyPayee>::Iterator it_l;

    list = file->payeeList();
    // load view
    for (it_l = list.begin(); it_l != list.end(); ++it_l) {
      auto item = new QTreeWidgetItem(ui->m_payeesView);
      item->setText(0, (*it_l).name());
      item->setData(0, Qt::UserRole, (*it_l).id());
      item->setCheckState(0, Qt::Checked);
    }
  }

  void setupDetailsPage()
  {
    Q_Q(KTransactionFilter);
    q->connect(ui->m_typeBox,     static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_stateBox,    static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_validityBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KTransactionFilter::slotUpdateSelections);

    q->connect(ui->m_nrButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotNrSelected);
    q->connect(ui->m_nrRangeButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotNrRangeSelected);
    q->connect(ui->m_nrEdit,      &QLineEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_nrFromEdit,  &QLineEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_nrToEdit,    &QLineEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);

    ui->m_nrButton->setChecked(true);
    q->slotNrSelected();
  }

  void setupTagsPage()
  {
    Q_Q(KTransactionFilter);
    ui->m_tagsView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->m_tagsView->header()->hide();
    ui->m_tagsView->setAlternatingRowColors(true);

    loadTags();

    ui->m_tagsView->sortItems(0, Qt::AscendingOrder);
    ui->m_emptyTagsButton->setCheckState(Qt::Unchecked);

    q->connect(ui->m_allTagsButton,   &QAbstractButton::clicked, q, &KTransactionFilter::slotSelectAllTags);
    q->connect(ui->m_clearTagsButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotDeselectAllTags);
    q->connect(ui->m_emptyTagsButton, &QCheckBox::stateChanged,  q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_tagsView,        &QTreeWidget::itemChanged, q, &KTransactionFilter::slotUpdateSelections);
  }

  void setupPayeesPage()
  {
    Q_Q(KTransactionFilter);
    ui->m_payeesView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->m_payeesView->header()->hide();
    ui->m_payeesView->setAlternatingRowColors(true);

    loadPayees();

    ui->m_payeesView->sortItems(0, Qt::AscendingOrder);
    ui->m_emptyPayeesButton->setCheckState(Qt::Unchecked);

    q->connect(ui->m_allPayeesButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotSelectAllPayees);
    q->connect(ui->m_clearPayeesButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotDeselectAllPayees);
    q->connect(ui->m_emptyPayeesButton, &QCheckBox::stateChanged,  q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_payeesView,        &QTreeWidget::itemChanged, q, &KTransactionFilter::slotUpdateSelections);
  }

  void setupAmountPage()
  {
    Q_Q(KTransactionFilter);
    q->connect(ui->m_amountButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotAmountSelected);
    q->connect(ui->m_amountRangeButton, &QAbstractButton::clicked, q, &KTransactionFilter::slotAmountRangeSelected);

    q->connect(ui->m_amountEdit,      &AmountEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_amountFromEdit,  &AmountEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);
    q->connect(ui->m_amountToEdit,    &AmountEdit::textChanged, q, &KTransactionFilter::slotUpdateSelections);

    ui->m_amountButton->setChecked(true);
    q->slotAmountSelected();
  }

  void setupCategoriesPage()
  {
    Q_Q(KTransactionFilter);
    ui->m_categoriesView->setSelectionMode(QTreeWidget::MultiSelection);
    AccountSet categorySet;
    categorySet.addAccountGroup(eMyMoney::Account::Type::Income);
    categorySet.addAccountGroup(eMyMoney::Account::Type::Expense);
    categorySet.load(ui->m_categoriesView);
    q->connect(ui->m_categoriesView, &KMyMoneyAccountSelector::stateChanged, q, &KTransactionFilter::slotUpdateSelections);
  }

  void setupAccountsPage(bool withEquityAccounts)
  {
    Q_Q(KTransactionFilter);
    ui->m_accountsView->setSelectionMode(QTreeWidget::MultiSelection);
    accountSet.addAccountGroup(eMyMoney::Account::Type::Asset);
    accountSet.addAccountGroup(eMyMoney::Account::Type::Liability);

    if (withEquityAccounts)
      accountSet.addAccountGroup(eMyMoney::Account::Type::Equity);

    // set the accountset to show closed account if the settings say so
    accountSet.setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts() && !KMyMoneySettings::showAllAccounts());
    accountSet.load(ui->m_accountsView);
    q->connect(ui->m_accountsView, &KMyMoneyAccountSelector::stateChanged, q, &KTransactionFilter::slotUpdateSelections);
  }

  KTransactionFilter      *q_ptr;
  Ui::KTransactionFilter  *ui;
  QDate                m_startDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
  QDate                m_endDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];

  MyMoneyTransactionFilter        m_filter;

  QMap<QWidget*, QString>         m_helpAnchor;

  DateRangeDlg                    *m_dateRange;

};

#endif
