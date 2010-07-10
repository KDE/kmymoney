/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kfindtransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QTabWidget>
#include <QLayout>
#include <QKeyEvent>
#include <QList>
#include <QResizeEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kstdguiitem.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountselector.h>
#include <mymoneyfile.h>
#include <kmymoneychecklistitem.h>
#include <kmymoneyglobalsettings.h>
#include <register.h>
#include <transaction.h>
#include <ktoolinvocation.h>


KSortOptionDlg::KSortOptionDlg(QWidget *parent)
    : KDialog(parent)
{
  setupUi(this);
  init();
}

void KSortOptionDlg::init()
{
  setButtons(ButtonCodes(KDialog::None));
}

void KSortOptionDlg::setSortOption(const QString& option, const QString& def)
{
  if (option.isEmpty()) {
    m_sortOption->setSettings(def);
    m_useDefault->setChecked(true);
  } else {
    m_sortOption->setSettings(option);
    m_useDefault->setChecked(false);
  }
}

QString KSortOptionDlg::sortOption(void) const
{
  QString rc;
  if (!m_useDefault->isChecked()) {
    rc = m_sortOption->settings();
  }
  return rc;
}

void KSortOptionDlg::hideDefaultButton(void)
{
  m_useDefault->hide();
}


KFindTransactionDlg::KFindTransactionDlg(QWidget *parent) :
    KFindTransactionDlgDecl(parent),
    m_needReload(false)
{
  ButtonGroup1->setId(m_amountButton, 0);
  ButtonGroup1->setId(m_amountRangeButton, 1);

  m_register->installEventFilter(this);
  m_tabWidget->setTabEnabled(m_tabWidget->indexOf(m_resultPage), false);

  // 'cause we don't have a separate setupTextPage
  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  setupAccountsPage();
  setupCategoriesPage();
  setupDatePage();
  setupAmountPage();
  setupPayeesPage();
  setupDetailsPage();

  // We don't need to add the default into the list (see ::slotShowHelp() why)
  // m_helpAnchor[m_textTab] = QString("details.search");
  m_helpAnchor[m_accountTab] = QString("details.search.account");
  m_helpAnchor[m_dateTab] = QString("details.search.date");
  m_helpAnchor[m_amountTab] = QString("details.search.amount");
  m_helpAnchor[m_categoryTab] = QString("details.search.category");
  m_helpAnchor[m_payeeTab] = QString("details.search.payee");
  m_helpAnchor[m_detailsTab] = QString("details.search.details");

  // setup the register
  QList<KMyMoneyRegister::Column> cols;
  cols << KMyMoneyRegister::DateColumn;
  cols << KMyMoneyRegister::AccountColumn;
  cols << KMyMoneyRegister::DetailColumn;
  cols << KMyMoneyRegister::ReconcileFlagColumn;
  cols << KMyMoneyRegister::PaymentColumn;
  cols << KMyMoneyRegister::DepositColumn;
  m_register->setupRegister(MyMoneyAccount(), cols);
  m_register->setSelectionMode(QTableWidget::SingleSelection);

  connect(m_register, SIGNAL(editTransaction()), this, SLOT(slotSelectTransaction()));
  connect(m_register->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSortOptions()));

  slotUpdateSelections();

  // setup the connections
  connect(this, SIGNAL(applyClicked()), this, SLOT(slotSearch()));
  connect(this, SIGNAL(resetClicked()), this, SLOT(slotReset()));
  connect(this, SIGNAL(resetClicked()), m_accountsView, SLOT(slotSelectAllAccounts()));
  connect(this, SIGNAL(resetClicked()), m_categoriesView, SLOT(slotSelectAllAccounts()));
  connect(this, SIGNAL(closeClicked()), this, SLOT(deleteLater()));
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotShowHelp()));

  // only allow searches when a selection has been made
  enableButtonApply(false);
  setButtonText(KDialog::Apply, i18n("&Find"));
  connect(this, SIGNAL(selectionNotEmpty(bool)), this, SLOT(enableButtonApply(bool)));

  // get signal about engine changes
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));

  slotUpdateSelections();

  m_textEdit->setFocus();
}

void KFindTransactionDlg::slotReset(void)
{
  m_textEdit->setText(QString());
  m_regExp->setChecked(false);
  m_caseSensitive->setChecked(false);
  m_textNegate->setCurrentItem(0);

  m_amountEdit->setEnabled(true);
  m_amountFromEdit->setEnabled(false);
  m_amountToEdit->setEnabled(false);
  m_amountEdit->loadText(QString());
  m_amountFromEdit->loadText(QString());
  m_amountToEdit->loadText(QString());
  m_amountButton->setChecked(true);
  m_amountRangeButton->setChecked(false);

  m_emptyPayeesButton->setChecked(false);
  selectAllItems(m_payeesView, true);

  m_typeBox->setCurrentIndex(MyMoneyTransactionFilter::allTypes);
  m_stateBox->setCurrentIndex(MyMoneyTransactionFilter::allStates);
  m_validityBox->setCurrentIndex(MyMoneyTransactionFilter::anyValidity);

  m_nrEdit->setEnabled(true);
  m_nrFromEdit->setEnabled(false);
  m_nrToEdit->setEnabled(false);
  m_nrEdit->setText(QString());
  m_nrFromEdit->setText(QString());
  m_nrToEdit->setText(QString());
  m_nrButton->setChecked(true);
  m_nrRangeButton->setChecked(false);

  m_tabWidget->setTabEnabled(m_tabWidget->indexOf(m_resultPage), false);
  m_tabWidget->setCurrentIndex(m_tabWidget->indexOf(m_criteriaTab));

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  m_dateRange->setCurrentItem(MyMoneyTransactionFilter::allDates);
  slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
}

void KFindTransactionDlg::slotUpdateSelections(void)
{
  QString txt;

  // Text tab
  if (!m_textEdit->text().isEmpty()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Text");
    m_regExp->setEnabled(QRegExp(m_textEdit->text()).isValid());
  } else
    m_regExp->setEnabled(false);

  m_caseSensitive->setEnabled(!m_textEdit->text().isEmpty());
  m_textNegate->setEnabled(!m_textEdit->text().isEmpty());

  // Account tab
  if (!m_accountsView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Account");
  }

  // Date tab
  if (m_dateRange->currentItem() != 0) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Date");
  }

  // Amount tab
  if ((m_amountButton->isChecked() && m_amountEdit->isValid())
      || (m_amountRangeButton->isChecked()
          && (m_amountFromEdit->isValid() || m_amountToEdit->isValid()))) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Amount");
  }

  // Categories tab
  if (!m_categoriesView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Category");
  }

  // Payees tab
  if (!allItemsSelected(m_payeesView)
      || m_emptyPayeesButton->isChecked()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Payees");
  }
  m_payeesView->setEnabled(!m_emptyPayeesButton->isChecked());

  // Details tab
  if (m_typeBox->currentIndex() != 0
      || m_stateBox->currentIndex() != 0
      || m_validityBox->currentIndex() != 0
      || (m_nrButton->isChecked() && m_nrEdit->text().length() != 0)
      || (m_nrRangeButton->isChecked()
          && (m_nrFromEdit->text().length() != 0 || m_nrToEdit->text().length() != 0))) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Details");
  }

  //Show a warning about transfers if Categories are filtered - bug #1523508
  if (!m_categoriesView->allItemsSelected()) {
    m_transferWarning->setText(i18n("Warning: Filtering by Category will exclude all transfers from the results."));
  } else {
    m_transferWarning->setText("");
  }

  // disable the search button if no selection is made
  emit selectionNotEmpty(!txt.isEmpty());

  if (txt.isEmpty()) {
    txt = i18nc("No selection", "(None)");
  }
  m_selectedCriteria->setText(i18n("Current selections: ") + txt);
}

bool KFindTransactionDlg::allItemsSelected(const QTreeWidgetItem *item) const
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < item->childCount(); ++i) {
    it_v = item->child(i);
    if (!(it_v->checkState(0) == Qt::Checked && allItemsSelected(it_v))) {
      return false;
    }
  }
  return true;
}

bool KFindTransactionDlg::allItemsSelected(const QTreeWidget* view) const
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

void KFindTransactionDlg::setupAccountsPage(void)
{
  m_accountsView->setSelectionMode(QTreeWidget::MultiSelection);
  AccountSet accountSet;
  accountSet.addAccountGroup(MyMoneyAccount::Asset);
  accountSet.addAccountGroup(MyMoneyAccount::Liability);
  //set the accountset to show closed account if the settings say so
  accountSet.setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());
  accountSet.load(m_accountsView);
  connect(m_accountsView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::selectAllItems(QTreeWidget* view, const bool state)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
    it_v = view->invisibleRootItem()->child(i);
    if (it_v->flags() & Qt::ItemIsUserCheckable) {
      it_v->setCheckState(0, Qt::Checked);
    }
    selectAllSubItems(it_v, state);
  }
  slotUpdateSelections();
}

void KFindTransactionDlg::selectItems(QTreeWidget* view, const QStringList& list, const bool state)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
    it_v = view->invisibleRootItem()->child(i);
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if (it_c->flags() & Qt::ItemIsUserCheckable && list.contains(it_c->id())) {
      it_c->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
    }
    selectSubItems(it_v, list, state);
  }

  slotUpdateSelections();
}

void KFindTransactionDlg::setupCategoriesPage(void)
{
  m_categoriesView->setSelectionMode(QTreeWidget::MultiSelection);
  AccountSet categorySet;
  categorySet.addAccountGroup(MyMoneyAccount::Income);
  categorySet.addAccountGroup(MyMoneyAccount::Expense);
  categorySet.load(m_categoriesView);
  connect(m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::selectAllSubItems(QTreeWidgetItem* item, const bool state)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < item->childCount(); ++i) {
    it_v = item->child(i);
    it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
    selectAllSubItems(it_v, state);
  }
}

void KFindTransactionDlg::selectSubItems(QTreeWidgetItem* item, const QStringList& list, const bool state)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < item->childCount(); ++i) {
    it_v = item->child(i);
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if (list.contains(it_c->id()))
      it_c->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
    selectSubItems(it_v, list, state);
  }
}

void KFindTransactionDlg::setupDatePage(void)
{
  int i;
  for (i = MyMoneyTransactionFilter::allDates; i < MyMoneyTransactionFilter::dateOptionCount; ++i) {
    MyMoneyTransactionFilter::translateDateRange(static_cast<MyMoneyTransactionFilter::dateOptionE>(i), m_startDates[i], m_endDates[i]);
  }

  connect(m_dateRange, SIGNAL(itemSelected(int)), this, SLOT(slotDateRangeChanged(int)));
  connect(m_fromDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged()));
  connect(m_toDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged()));

  slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
}

void KFindTransactionDlg::slotDateRangeChanged(int idx)
{
  switch (idx) {
    case MyMoneyTransactionFilter::allDates:
    case MyMoneyTransactionFilter::userDefined:
      m_fromDate->loadDate(QDate());
      m_toDate->loadDate(QDate());
      break;
    default:
      m_fromDate->loadDate(m_startDates[idx]);
      m_toDate->loadDate(m_endDates[idx]);
      break;
  }
  slotUpdateSelections();
}

void KFindTransactionDlg::slotDateChanged(void)
{
  int idx;
  for (idx = MyMoneyTransactionFilter::asOfToday; idx < MyMoneyTransactionFilter::dateOptionCount; ++idx) {
    if (m_fromDate->date() == m_startDates[idx]
        && m_toDate->date() == m_endDates[idx]) {
      break;
    }
  }
  //if no filter matched, set to user defined
  if (idx == MyMoneyTransactionFilter::dateOptionCount)
    idx = MyMoneyTransactionFilter::userDefined;

  m_dateRange->blockSignals(true);
  m_dateRange->setCurrentItem(static_cast<MyMoneyTransactionFilter::dateOptionE>(idx));
  m_dateRange->blockSignals(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupAmountPage(void)
{
  connect(m_amountButton, SIGNAL(clicked()), this, SLOT(slotAmountSelected()));
  connect(m_amountRangeButton, SIGNAL(clicked()), this, SLOT(slotAmountRangeSelected()));

  connect(m_amountEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_amountFromEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_amountToEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  m_amountButton->setChecked(true);
  slotAmountSelected();
}

void KFindTransactionDlg::slotAmountSelected(void)
{
  m_amountEdit->setEnabled(true);
  m_amountFromEdit->setEnabled(false);
  m_amountToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotAmountRangeSelected(void)
{
  m_amountEdit->setEnabled(false);
  m_amountFromEdit->setEnabled(true);
  m_amountToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupPayeesPage(void)
{
  m_payeesView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_payeesView->header()->hide();
  m_payeesView->setAlternatingRowColors(true);

  loadPayees();
  m_emptyPayeesButton->setCheckState(Qt::Unchecked);

  connect(m_allPayeesButton, SIGNAL(clicked()), this, SLOT(slotSelectAllPayees()));
  connect(m_clearPayeesButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllPayees()));
  connect(m_emptyPayeesButton, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::loadPayees(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyPayee> list;
  QList<MyMoneyPayee>::Iterator it_l;

  list = file->payeeList();
  // load view
  for (it_l = list.begin(); it_l != list.end(); ++it_l) {
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(m_payeesView, (*it_l).name(), QString(), (*it_l).id());
    connect(item, SIGNAL(stateChanged(bool)), this, SLOT(slotUpdateSelections()));
    item->setCheckState(0, Qt::Checked);
  }
}
void KFindTransactionDlg::slotSelectAllPayees(void)
{
  selectAllItems(m_payeesView, true);
}

void KFindTransactionDlg::slotDeselectAllPayees(void)
{
  selectAllItems(m_payeesView, false);
}

void KFindTransactionDlg::setupDetailsPage(void)
{
  connect(m_typeBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  connect(m_stateBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  connect(m_validityBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));

  connect(m_nrButton, SIGNAL(clicked()), this, SLOT(slotNrSelected()));
  connect(m_nrRangeButton, SIGNAL(clicked()), this, SLOT(slotNrRangeSelected()));
  connect(m_nrEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_nrFromEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_nrToEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  m_nrButton->setChecked(true);
  slotNrSelected();
}

void KFindTransactionDlg::slotNrSelected(void)
{
  m_nrEdit->setEnabled(true);
  m_nrFromEdit->setEnabled(false);
  m_nrToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotNrRangeSelected(void)
{
  m_nrEdit->setEnabled(false);
  m_nrFromEdit->setEnabled(true);
  m_nrToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::addItemToFilter(const opTypeE op, const QString& id)
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
  }
}

void KFindTransactionDlg::scanCheckListItems(const QTreeWidgetItem* item, const opTypeE op)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < item->childCount(); ++i) {
    it_v = item->child(i);
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if (it_c) {
      if (it_c->flags() & Qt::ItemIsUserCheckable) {
        if (it_c->checkState(0) & Qt::Checked)
          addItemToFilter(op, (*it_c).id());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::scanCheckListItems(const QTreeWidget* view, const opTypeE op)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
    it_v = view->invisibleRootItem()->child(i);
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if (it_c) {
      if (it_c->flags() == Qt::ItemIsUserCheckable) {
        if (it_c->checkState(0) & Qt::Checked)
          addItemToFilter(op, (*it_c).id());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::setupFilter(void)
{
  m_filter.clear();

  // Text tab
  if (!m_textEdit->text().isEmpty()) {
    QRegExp exp(m_textEdit->text(), m_caseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, !m_regExp->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp);
    m_filter.setTextFilter(exp, m_textNegate->currentIndex() != 0);
  }

  // Account tab
  if (!m_accountsView->allItemsSelected()) {
    // retrieve a list of selected accounts
    QStringList list;
    m_accountsView->selectedItems(list);

    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected
    if (!KMyMoneyGlobalSettings::expertMode()) {
      QStringList missing;
      QStringList::const_iterator it_a, it_b;
      for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
        MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
        if (acc.accountType() == MyMoneyAccount::Investment) {
          for (it_b = acc.accountList().constBegin(); it_b != acc.accountList().constEnd(); ++it_b) {
            if (!list.contains(*it_b)) {
              missing.append(*it_b);
            }
          }
        }
      }
      list += missing;
    }

    m_filter.addAccount(list);
  }

  // Date tab
  if (m_dateRange->currentItem() != 0) {
    m_filter.setDateFilter(m_fromDate->date(), m_toDate->date());
  }

  // Amount tab
  if ((m_amountButton->isChecked() && m_amountEdit->isValid())) {
    m_filter.setAmountFilter(m_amountEdit->value(), m_amountEdit->value());

  } else if ((m_amountRangeButton->isChecked()
              && (m_amountFromEdit->isValid() || m_amountToEdit->isValid()))) {

    MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
    if (m_amountFromEdit->isValid())
      from = m_amountFromEdit->value();
    if (m_amountToEdit->isValid())
      to = m_amountToEdit->value();

    m_filter.setAmountFilter(from, to);
  }

  // Categories tab
  if (!m_categoriesView->allItemsSelected()) {
    m_filter.addCategory(m_categoriesView->selectedItems());
  }

  // Payees tab
  if (m_emptyPayeesButton->isChecked()) {
    m_filter.addPayee(QString());

  } else if (!allItemsSelected(m_payeesView)) {
    scanCheckListItems(m_payeesView, addPayeeToFilter);
  }

  // Details tab
  if (m_typeBox->currentIndex() != 0)
    m_filter.addType(m_typeBox->currentIndex());

  if (m_stateBox->currentIndex() != 0)
    m_filter.addState(m_stateBox->currentIndex());

  if (m_validityBox->currentIndex() != 0)
    m_filter.addValidity(m_validityBox->currentIndex());

  if (m_nrButton->isChecked() && !m_nrEdit->text().isEmpty())
    m_filter.setNumberFilter(m_nrEdit->text(), m_nrEdit->text());

  if (m_nrRangeButton->isChecked()
      && (!m_nrFromEdit->text().isEmpty() || !m_nrToEdit->text().isEmpty())) {
    m_filter.setNumberFilter(m_nrFromEdit->text(), m_nrToEdit->text());
  }
}

void KFindTransactionDlg::slotSearch(void)
{
  // setup the filter from the dialog widgets
  setupFilter();

  // filter is setup, now fill the register
  slotRefreshView();

  m_register->setFocus();
}

void KFindTransactionDlg::slotRefreshView(void)
{
  m_needReload = true;
  if (isVisible()) {
    loadView();
    m_needReload = false;
  }
}

void KFindTransactionDlg::showEvent(QShowEvent* event)
{
  if (m_needReload) {
    loadView();
    m_needReload = false;
  }
  KFindTransactionDlgDecl::showEvent(event);
}

void KFindTransactionDlg::loadView(void)
{
  // setup sort order
  m_register->setSortOrder(KMyMoneyGlobalSettings::sortSearchView());

  // clear out old data
  m_register->clear();

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

    KMyMoneyRegister::Register::transactionFactory(m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
    { // debug stuff
      if (split.shares().isNegative()) {
        payment += split.shares().abs();
      } else {
        deposit += split.shares().abs();
      }
    }
  }

  // add the group markers
  m_register->addGroupMarkers();

  // sort the transactions according to the sort setting
  m_register->sortItems();

  // remove trailing and adjacent markers
  m_register->removeUnwantedGroupMarkers();

  // turn on the ledger lens for the register
  m_register->setLedgerLensForced();

  m_register->updateRegister(true);

  m_register->setFocusToTop();
  m_register->selectItem(m_register->focusItem());

#ifdef KMM_DEBUG
  m_foundText->setText(i18np("Found %1 matching transaction (D %2 / P %3 = %4)",
                             "Found %1 matching transactions (D %2 / P %3 = %4)", splitCount, deposit.formatMoney("", 2), payment.formatMoney("", 2), (deposit - payment).formatMoney("", 2)));
#else
  m_foundText->setText(i18np("Found %1 matching transaction", "Found %1 matching transactions", splitCount));
#endif

  m_tabWidget->setTabEnabled(m_tabWidget->indexOf(m_resultPage), true);
  m_tabWidget->setCurrentIndex(m_tabWidget->indexOf(m_resultPage));

  QTimer::singleShot(10, this, SLOT(slotRightSize()));
}

void KFindTransactionDlg::slotRightSize(void)
{
  m_register->update();
}

void KFindTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  // Columns
  // 1 = Date
  // 2 = Account
  // 4 = Detail
  // 5 = C
  // 6 = Payment
  // 7 = Deposit

  // don't forget the resizer
  KFindTransactionDlgDecl::resizeEvent(ev);

  if (!m_register->isVisible())
    return;

  // resize the register
  int w = m_register->contentsRect().width();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  m_register->adjustColumn(1);
  m_register->adjustColumn(2);
  m_register->adjustColumn(5);

  m_register->setColumnWidth(6, m_debitWidth);
  m_register->setColumnWidth(7, m_creditWidth);

  for (int i = 0; i < m_register->columnCount(); ++i) {
    switch (i) {
      case 4:     // skip the one, we want to set
        break;
      default:
        w -= m_register->columnWidth(i);
        break;
    }
  }

  m_register->setColumnWidth(4, w);
}


void KFindTransactionDlg::slotSelectTransaction(void)
{
  QList<KMyMoneyRegister::RegisterItem*> list = m_register->selectedItems();
  if (!list.isEmpty()) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t) {
      emit transactionSelected(t->split().accountId(), t->transaction().id());
      hide();
    }
  }
}

bool KFindTransactionDlg::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if (o->isWidgetType()) {
    if (e->type() == QEvent::KeyPress) {
      const QWidget* w = dynamic_cast<const QWidget*>(o);
      QKeyEvent *k = static_cast<QKeyEvent *>(e);
      if (w == m_register) {
        switch (k->key()) {
          default:
            break;

          case Qt::Key_Return:
          case Qt::Key_Enter:
            rc = true;
            slotSelectTransaction();
            break;
        }
      }
    }
  }
  return rc;
}

void KFindTransactionDlg::slotShowHelp(void)
{
  QString anchor = m_helpAnchor[m_criteriaTab->currentWidget()];
  if (anchor.isEmpty())
    anchor = QString("details.search");

  KToolInvocation::invokeHelp(anchor);
}

void KFindTransactionDlg::slotSortOptions(void)
{
  QPointer<KSortOptionDlg> dlg = new KSortOptionDlg(this);

  dlg->setSortOption(KMyMoneyGlobalSettings::sortSearchView(), QString());
  dlg->hideDefaultButton();

  if (dlg->exec() == QDialog::Accepted) {
    QString sortOrder = dlg->sortOption();
    if (sortOrder != KMyMoneyGlobalSettings::sortSearchView()) {
      KMyMoneyGlobalSettings::setSortSearchView(sortOrder);
      slotRefreshView();
    }
  }
  delete dlg;
}


// vim:cin:si:ai:et:ts=2:sw=2:


#include "kfindtransactiondlg.moc"
