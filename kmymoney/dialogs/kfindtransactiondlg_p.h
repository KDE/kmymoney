/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   q program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDTRANSACTIONDLG_P_H
#define KFINDTRANSACTIONDLG_P_H

#include "kfindtransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QTabWidget>
#include <QList>
#include <QPushButton>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KComboBox>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyedit.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "kmymoneysettings.h"
#include "register.h"
#include "transaction.h"
#include "daterangedlg.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"

#include "ui_kfindtransactiondlg.h"

#include "widgetenums.h"
#include "mymoneyenums.h"

class KFindTransactionDlgPrivate
{
  Q_DISABLE_COPY(KFindTransactionDlgPrivate)
  Q_DECLARE_PUBLIC(KFindTransactionDlg)

public:
  enum opTypeE {
    addAccountToFilter = 0,
    addCategoryToFilter,
    addPayeeToFilter,
    addTagToFilter
  };

  explicit KFindTransactionDlgPrivate(KFindTransactionDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KFindTransactionDlg)
  {
  }

  ~KFindTransactionDlgPrivate()
  {
    delete ui;
  }

  void init(bool withEquityAccounts)
  {
    Q_Q(KFindTransactionDlg);
    m_needReload = false;
    ui->setupUi(q);
    m_dateRange = new DateRangeDlg;
    ui->dateRangeLayout->insertWidget(0, m_dateRange);

    ui->ButtonGroup1->setId(ui->m_amountButton, 0);
    ui->ButtonGroup1->setId(ui->m_amountRangeButton, 1);

    ui->m_register->installEventFilter(q);
    ui->m_tabWidget->setTabEnabled(ui->m_tabWidget->indexOf(ui->m_resultPage), false);

    // 'cause we don't have a separate setupTextPage
    q->connect(ui->m_textEdit, &QLineEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);
    // if return is pressed trigger a search (slotSearch checks if it's possible to perform the search)
    q->connect(ui->m_textEdit, &KLineEdit::returnPressed, q, &KFindTransactionDlg::slotSearch);
    // in case the date selection changes, we update the selection
    q->connect(m_dateRange, &DateRangeDlg::rangeChanged, q, &KFindTransactionDlg::slotUpdateSelections);

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

    // setup the register
    QList<eWidgets::eTransaction::Column> cols {
          eWidgets::eTransaction::Column::Date,
          eWidgets::eTransaction::Column::Account,
          eWidgets::eTransaction::Column::Detail,
          eWidgets::eTransaction::Column::ReconcileFlag,
          eWidgets::eTransaction::Column::Payment,
          eWidgets::eTransaction::Column::Deposit};
    ui->m_register->setupRegister(MyMoneyAccount(), cols);
    ui->m_register->setSelectionMode(QTableWidget::SingleSelection);

    q->connect(ui->m_register, &KMyMoneyRegister::Register::editTransaction, q, &KFindTransactionDlg::slotSelectTransaction);
    q->connect(ui->m_register->horizontalHeader(), &QWidget::customContextMenuRequested, q, &KFindTransactionDlg::slotSortOptions);

    q->slotUpdateSelections();

    // setup the connections
    q->connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotSearch);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotReset);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, ui->m_accountsView, &KMyMoneyAccountSelector::slotSelectAllAccounts);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, ui->m_categoriesView, &KMyMoneyAccountSelector::slotSelectAllAccounts);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked, q, &QObject::deleteLater);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotShowHelp);

    // only allow searches when a selection has been made
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    KGuiItem::assign(ui->buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::find());
    ui->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for find transaction apply button", "Search transactions"));
    q->connect(q, &KFindTransactionDlg::selectionNotEmpty, ui->buttonBox->button(QDialogButtonBox::Apply), &QWidget::setEnabled);

    // get signal about engine changes
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KFindTransactionDlg::slotRefreshView);

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
    Q_Q(KFindTransactionDlg);
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

    Q_Q(KFindTransactionDlg);
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
    * q method loads the register with the matching transactions
    */
  void loadView()
  {
    // setup sort order
    ui->m_register->setSortOrder(KMyMoneySettings::sortSearchView());

    // clear out old data
    ui->m_register->clear();

    // retrieve the list from the engine
    MyMoneyFile::instance()->transactionList(m_transactionList, m_filter);

    // create the elements for the register
    QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    QMap<QString, int>uniqueMap;
    MyMoneyMoney deposit, payment;

    int splitCount = 0;
    for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
      const MyMoneySplit& split = (*it).second;
      MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
      ++splitCount;
      uniqueMap[(*it).first.id()]++;

      KMyMoneyRegister::Register::transactionFactory(ui->m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
      { // debug stuff
        if (split.shares().isNegative()) {
          payment += split.shares().abs();
        } else {
          deposit += split.shares().abs();
        }
      }
    }

    // add the group markers
    ui->m_register->addGroupMarkers();

    // sort the transactions according to the sort setting
    ui->m_register->sortItems();

    // remove trailing and adjacent markers
    ui->m_register->removeUnwantedGroupMarkers();

    // turn on the ledger lens for the register
    ui->m_register->setLedgerLensForced();

    ui->m_register->updateRegister(true);

    ui->m_register->setFocusToTop();
    ui->m_register->selectItem(ui->m_register->focusItem());

  #ifdef KMM_DEBUG
    ui->m_foundText->setText(i18np("Found %1 matching transaction (D %2 / P %3 = %4)",
                                     "Found %1 matching transactions (D %2 / P %3 = %4)", splitCount, deposit.formatMoney("", 2), payment.formatMoney("", 2), (deposit - payment).formatMoney("", 2)));
  #else
    ui->m_foundText->setText(i18np("Found %1 matching transaction", "Found %1 matching transactions", splitCount));
  #endif

    ui->m_tabWidget->setTabEnabled(ui->m_tabWidget->indexOf(ui->m_resultPage), true);
    ui->m_tabWidget->setCurrentIndex(ui->m_tabWidget->indexOf(ui->m_resultPage));

    Q_Q(KFindTransactionDlg);
    QTimer::singleShot(10, q, SLOT(slotRightSize()));
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

  void setupFilter()
  {
    m_filter.clear();

    // Text tab
    if (!ui->m_textEdit->text().isEmpty()) {
      QRegExp exp(ui->m_textEdit->text(), ui->m_caseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, !ui->m_regExp->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp);
      m_filter.setTextFilter(exp, ui->m_textNegate->currentIndex() != 0);
    }

    // Account tab
    if (!ui->m_accountsView->allItemsSelected()) {
      // retrieve a list of selected accounts
      QStringList list;
      ui->m_accountsView->selectedItems(list);

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

      m_filter.addAccount(list);
    }

    // Date tab
    if ((int)m_dateRange->dateRange() != 0) {
      m_filter.setDateFilter(m_dateRange->fromDate(), m_dateRange->toDate());
    }

    // Amount tab
    if ((ui->m_amountButton->isChecked() && ui->m_amountEdit->isValid())) {
      m_filter.setAmountFilter(ui->m_amountEdit->value(), ui->m_amountEdit->value());

    } else if ((ui->m_amountRangeButton->isChecked()
                && (ui->m_amountFromEdit->isValid() || ui->m_amountToEdit->isValid()))) {

      MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
      if (ui->m_amountFromEdit->isValid())
        from = ui->m_amountFromEdit->value();
      if (ui->m_amountToEdit->isValid())
        to = ui->m_amountToEdit->value();

      m_filter.setAmountFilter(from, to);
    }

    // Categories tab
    if (!ui->m_categoriesView->allItemsSelected()) {
      m_filter.addCategory(ui->m_categoriesView->selectedItems());
    }

    // Tags tab
    if (ui->m_emptyTagsButton->isChecked()) {
      m_filter.addTag(QString());

    } else if (!allItemsSelected(ui->m_tagsView)) {
      scanCheckListItems(ui->m_tagsView, addTagToFilter);
    }

    // Payees tab
    if (ui->m_emptyPayeesButton->isChecked()) {
      m_filter.addPayee(QString());

    } else if (!allItemsSelected(ui->m_payeesView)) {
      scanCheckListItems(ui->m_payeesView, addPayeeToFilter);
    }

    // Details tab
    if (ui->m_typeBox->currentIndex() != 0)
      m_filter.addType(ui->m_typeBox->currentIndex());

    if (ui->m_stateBox->currentIndex() != 0)
      m_filter.addState(ui->m_stateBox->currentIndex());

    if (ui->m_validityBox->currentIndex() != 0)
      m_filter.addValidity(ui->m_validityBox->currentIndex());

    if (ui->m_nrButton->isChecked() && !ui->m_nrEdit->text().isEmpty())
      m_filter.setNumberFilter(ui->m_nrEdit->text(), ui->m_nrEdit->text());

    if (ui->m_nrRangeButton->isChecked()
        && (!ui->m_nrFromEdit->text().isEmpty() || !ui->m_nrToEdit->text().isEmpty())) {
      m_filter.setNumberFilter(ui->m_nrFromEdit->text(), ui->m_nrToEdit->text());
    }
  }

  void setupDetailsPage()
  {
    Q_Q(KFindTransactionDlg);
    q->connect(ui->m_typeBox,     static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_stateBox,    static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_validityBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KFindTransactionDlg::slotUpdateSelections);

    q->connect(ui->m_nrButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotNrSelected);
    q->connect(ui->m_nrRangeButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotNrRangeSelected);
    q->connect(ui->m_nrEdit,      &QLineEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_nrFromEdit,  &QLineEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_nrToEdit,    &QLineEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);

    ui->m_nrButton->setChecked(true);
    q->slotNrSelected();
  }

  void setupTagsPage()
  {
    Q_Q(KFindTransactionDlg);
    ui->m_tagsView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->m_tagsView->header()->hide();
    ui->m_tagsView->setAlternatingRowColors(true);

    loadTags();

    ui->m_tagsView->sortItems(0, Qt::AscendingOrder);
    ui->m_emptyTagsButton->setCheckState(Qt::Unchecked);

    q->connect(ui->m_allTagsButton,   &QAbstractButton::clicked, q, &KFindTransactionDlg::slotSelectAllTags);
    q->connect(ui->m_clearTagsButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotDeselectAllTags);
    q->connect(ui->m_emptyTagsButton, &QCheckBox::stateChanged,  q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_tagsView,        &QTreeWidget::itemChanged, q, &KFindTransactionDlg::slotUpdateSelections);
  }

  void setupPayeesPage()
  {
    Q_Q(KFindTransactionDlg);
    ui->m_payeesView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->m_payeesView->header()->hide();
    ui->m_payeesView->setAlternatingRowColors(true);

    loadPayees();

    ui->m_payeesView->sortItems(0, Qt::AscendingOrder);
    ui->m_emptyPayeesButton->setCheckState(Qt::Unchecked);

    q->connect(ui->m_allPayeesButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotSelectAllPayees);
    q->connect(ui->m_clearPayeesButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotDeselectAllPayees);
    q->connect(ui->m_emptyPayeesButton, &QCheckBox::stateChanged,  q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_payeesView,        &QTreeWidget::itemChanged, q, &KFindTransactionDlg::slotUpdateSelections);
  }

  void setupAmountPage()
  {
    Q_Q(KFindTransactionDlg);
    q->connect(ui->m_amountButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotAmountSelected);
    q->connect(ui->m_amountRangeButton, &QAbstractButton::clicked, q, &KFindTransactionDlg::slotAmountRangeSelected);

    q->connect(ui->m_amountEdit,      &KMyMoneyEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_amountFromEdit,  &KMyMoneyEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);
    q->connect(ui->m_amountToEdit,    &KMyMoneyEdit::textChanged, q, &KFindTransactionDlg::slotUpdateSelections);

    ui->m_amountButton->setChecked(true);
    q->slotAmountSelected();
  }

  void setupCategoriesPage()
  {
    Q_Q(KFindTransactionDlg);
    ui->m_categoriesView->setSelectionMode(QTreeWidget::MultiSelection);
    AccountSet categorySet;
    categorySet.addAccountGroup(eMyMoney::Account::Type::Income);
    categorySet.addAccountGroup(eMyMoney::Account::Type::Expense);
    categorySet.load(ui->m_categoriesView);
    q->connect(ui->m_categoriesView, &KMyMoneyAccountSelector::stateChanged, q, &KFindTransactionDlg::slotUpdateSelections);
  }

  void setupAccountsPage(bool withEquityAccounts)
  {
    Q_Q(KFindTransactionDlg);
    ui->m_accountsView->setSelectionMode(QTreeWidget::MultiSelection);
    AccountSet accountSet;
    accountSet.addAccountGroup(eMyMoney::Account::Type::Asset);
    accountSet.addAccountGroup(eMyMoney::Account::Type::Liability);

    if (withEquityAccounts)
      accountSet.addAccountGroup(eMyMoney::Account::Type::Equity);

    //set the accountset to show closed account if the settings say so
    accountSet.setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts());
    accountSet.load(ui->m_accountsView);
    q->connect(ui->m_accountsView, &KMyMoneyAccountSelector::stateChanged, q, &KFindTransactionDlg::slotUpdateSelections);
  }

  KFindTransactionDlg      *q_ptr;
  Ui::KFindTransactionDlg  *ui;
  QDate                m_startDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
  QDate                m_endDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];

  /**
    * q member holds a list of all transactions matching the filter criteria
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

  MyMoneyTransactionFilter        m_filter;

  QMap<QWidget*, QString>         m_helpAnchor;

  bool                            m_needReload;
  DateRangeDlg                    *m_dateRange;

};

#endif
