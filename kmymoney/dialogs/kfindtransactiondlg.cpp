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

#include <config-kmymoney.h>

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
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountselector.h>
#include <mymoneyfile.h>
#include <kmymoneyglobalsettings.h>
#include <register.h>
#include <transaction.h>
#include <ktoolinvocation.h>
#include "ui_kfindtransactiondlgdecl.h"
#include "ui_ksortoptiondlg.h"

enum ItemRoles {
  ItemIdRole = Qt::UserRole
};

struct KSortOptionDlg::Private {
  Ui::KSortOptionDlg ui;
};

KSortOptionDlg::KSortOptionDlg(QWidget *parent)
    : KDialog(parent), d(new Private)
{
  d->ui.setupUi(this);
  init();
}

KSortOptionDlg::~KSortOptionDlg()
{
  delete d;
}

void KSortOptionDlg::init()
{
  setButtons(ButtonCodes(KDialog::None));
  d->ui.m_cancelButton->setGuiItem(KStandardGuiItem::cancel());
  d->ui.m_okButton->setGuiItem(KStandardGuiItem::ok());
  d->ui.m_helpButton->setGuiItem(KStandardGuiItem::help());
}

void KSortOptionDlg::setSortOption(const QString& option, const QString& def)
{
  if (option.isEmpty()) {
    d->ui.m_sortOption->setSettings(def);
    d->ui.m_useDefault->setChecked(true);
  } else {
    d->ui.m_sortOption->setSettings(option);
    d->ui.m_useDefault->setChecked(false);
  }
}

QString KSortOptionDlg::sortOption() const
{
  QString rc;
  if (!d->ui.m_useDefault->isChecked()) {
    rc = d->ui.m_sortOption->settings();
  }
  return rc;
}

void KSortOptionDlg::hideDefaultButton()
{
  d->ui.m_useDefault->hide();
}


KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, bool withEquityAccounts) :
    KDialog(parent),
    m_needReload(false),
    m_ui(new Ui::KFindTransactionDlgDecl)
{
  m_ui->setupUi(this);
  setMainWidget(m_ui->m_tabWidget);

  setButtons(KDialog::Help | KDialog::Close | KDialog::Reset | KDialog::Apply);

  m_ui->ButtonGroup1->setId(m_ui->m_amountButton, 0);
  m_ui->ButtonGroup1->setId(m_ui->m_amountRangeButton, 1);

  m_ui->m_register->installEventFilter(this);
  m_ui->m_tabWidget->setTabEnabled(m_ui->m_tabWidget->indexOf(m_ui->m_resultPage), false);

  // 'cause we don't have a separate setupTextPage
  connect(m_ui->m_textEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));
  // if return is pressed trigger a search (slotSearch checks if it's possible to perform the search)
  connect(m_ui->m_textEdit, SIGNAL(returnPressed()), this, SLOT(slotSearch()));

  setupAccountsPage(withEquityAccounts);
  setupCategoriesPage();
  setupDatePage();
  setupAmountPage();
  setupPayeesPage();
  setupTagsPage();
  setupDetailsPage();

  // We don't need to add the default into the list (see ::slotShowHelp() why)
  // m_helpAnchor[m_ui->m_textTab] = QString("details.search");
  m_helpAnchor[m_ui->m_accountTab] = QString("details.search.account");
  m_helpAnchor[m_ui->m_dateTab] = QString("details.search.date");
  m_helpAnchor[m_ui->m_amountTab] = QString("details.search.amount");
  m_helpAnchor[m_ui->m_categoryTab] = QString("details.search.category");
  m_helpAnchor[m_ui->m_payeeTab] = QString("details.search.payee");
  m_helpAnchor[m_ui->m_tagTab] = QString("details.search.tag"); //FIXME-ALEX update Help
  m_helpAnchor[m_ui->m_detailsTab] = QString("details.search.details");

  // setup the register
  QList<KMyMoneyRegister::Column> cols;
  cols << KMyMoneyRegister::DateColumn;
  cols << KMyMoneyRegister::AccountColumn;
  cols << KMyMoneyRegister::DetailColumn;
  cols << KMyMoneyRegister::ReconcileFlagColumn;
  cols << KMyMoneyRegister::PaymentColumn;
  cols << KMyMoneyRegister::DepositColumn;
  m_ui->m_register->setupRegister(MyMoneyAccount(), cols);
  m_ui->m_register->setSelectionMode(QTableWidget::SingleSelection);

  connect(m_ui->m_register, SIGNAL(editTransaction()), this, SLOT(slotSelectTransaction()));
  connect(m_ui->m_register->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSortOptions()));

  slotUpdateSelections();

  // setup the connections
  connect(this, SIGNAL(applyClicked()), this, SLOT(slotSearch()));
  connect(this, SIGNAL(resetClicked()), this, SLOT(slotReset()));
  connect(this, SIGNAL(resetClicked()), m_ui->m_accountsView, SLOT(slotSelectAllAccounts()));
  connect(this, SIGNAL(resetClicked()), m_ui->m_categoriesView, SLOT(slotSelectAllAccounts()));
  connect(this, SIGNAL(closeClicked()), this, SLOT(deleteLater()));
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotShowHelp()));

  // only allow searches when a selection has been made
  enableButtonApply(false);
  setButtonGuiItem(KDialog::Apply, KStandardGuiItem::find());
  setButtonToolTip(KDialog::Apply, i18nc("@info:tooltip for find transaction apply button", "Search transactions"));
  connect(this, SIGNAL(selectionNotEmpty(bool)), this, SLOT(enableButtonApply(bool)));

  // get signal about engine changes
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));

  slotUpdateSelections();

  m_ui->m_textEdit->setFocus();
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  delete m_ui;
}

void KFindTransactionDlg::slotReset()
{
  m_ui->m_textEdit->setText(QString());
  m_ui->m_regExp->setChecked(false);
  m_ui->m_caseSensitive->setChecked(false);
  m_ui->m_textNegate->setCurrentItem(0);

  m_ui->m_amountEdit->setEnabled(true);
  m_ui->m_amountFromEdit->setEnabled(false);
  m_ui->m_amountToEdit->setEnabled(false);
  m_ui->m_amountEdit->loadText(QString());
  m_ui->m_amountFromEdit->loadText(QString());
  m_ui->m_amountToEdit->loadText(QString());
  m_ui->m_amountButton->setChecked(true);
  m_ui->m_amountRangeButton->setChecked(false);

  m_ui->m_emptyPayeesButton->setChecked(false);
  selectAllItems(m_ui->m_payeesView, true);

  m_ui->m_emptyTagsButton->setChecked(false);
  selectAllItems(m_ui->m_tagsView, true);

  m_ui->m_typeBox->setCurrentIndex(MyMoneyTransactionFilter::allTypes);
  m_ui->m_stateBox->setCurrentIndex(MyMoneyTransactionFilter::allStates);
  m_ui->m_validityBox->setCurrentIndex(MyMoneyTransactionFilter::anyValidity);

  m_ui->m_nrEdit->setEnabled(true);
  m_ui->m_nrFromEdit->setEnabled(false);
  m_ui->m_nrToEdit->setEnabled(false);
  m_ui->m_nrEdit->setText(QString());
  m_ui->m_nrFromEdit->setText(QString());
  m_ui->m_nrToEdit->setText(QString());
  m_ui->m_nrButton->setChecked(true);
  m_ui->m_nrRangeButton->setChecked(false);

  m_ui->m_tabWidget->setTabEnabled(m_ui->m_tabWidget->indexOf(m_ui->m_resultPage), false);
  m_ui->m_tabWidget->setCurrentIndex(m_ui->m_tabWidget->indexOf(m_ui->m_criteriaTab));

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::allDates);
  slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
}

void KFindTransactionDlg::slotUpdateSelections()
{
  QString txt;

  // Text tab
  if (!m_ui->m_textEdit->text().isEmpty()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Text");
    m_ui->m_regExp->setEnabled(QRegExp(m_ui->m_textEdit->text()).isValid());
  } else
    m_ui->m_regExp->setEnabled(false);

  m_ui->m_caseSensitive->setEnabled(!m_ui->m_textEdit->text().isEmpty());
  m_ui->m_textNegate->setEnabled(!m_ui->m_textEdit->text().isEmpty());

  // Account tab
  if (!m_ui->m_accountsView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Account");
  }

  // Date tab
  if (m_ui->m_dateRange->currentItem() != 0) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Date");
  }

  // Amount tab
  if ((m_ui->m_amountButton->isChecked() && m_ui->m_amountEdit->isValid())
      || (m_ui->m_amountRangeButton->isChecked()
          && (m_ui->m_amountFromEdit->isValid() || m_ui->m_amountToEdit->isValid()))) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Amount");
  }

  // Categories tab
  if (!m_ui->m_categoriesView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Category");
  }

  // Tags tab
  if (!allItemsSelected(m_ui->m_tagsView)
      || m_ui->m_emptyTagsButton->isChecked()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Tags");
  }
  m_ui->m_tagsView->setEnabled(!m_ui->m_emptyTagsButton->isChecked());

  // Payees tab
  if (!allItemsSelected(m_ui->m_payeesView)
      || m_ui->m_emptyPayeesButton->isChecked()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Payees");
  }
  m_ui->m_payeesView->setEnabled(!m_ui->m_emptyPayeesButton->isChecked());

  // Details tab
  if (m_ui->m_typeBox->currentIndex() != 0
      || m_ui->m_stateBox->currentIndex() != 0
      || m_ui->m_validityBox->currentIndex() != 0
      || (m_ui->m_nrButton->isChecked() && m_ui->m_nrEdit->text().length() != 0)
      || (m_ui->m_nrRangeButton->isChecked()
          && (m_ui->m_nrFromEdit->text().length() != 0 || m_ui->m_nrToEdit->text().length() != 0))) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Details");
  }

  //Show a warning about transfers if Categories are filtered - bug #1523508
  if (!m_ui->m_categoriesView->allItemsSelected()) {
    m_ui->m_transferWarning->setText(i18n("Warning: Filtering by Category will exclude all transfers from the results."));
  } else {
    m_ui->m_transferWarning->setText("");
  }

  // disable the search button if no selection is made
  emit selectionNotEmpty(!txt.isEmpty());

  if (txt.isEmpty()) {
    txt = i18nc("No selection", "(None)");
  }
  m_ui->m_selectedCriteria->setText(i18n("Current selections: %1", txt));
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

void KFindTransactionDlg::setupAccountsPage(bool withEquityAccounts)
{
  m_ui->m_accountsView->setSelectionMode(QTreeWidget::MultiSelection);
  AccountSet accountSet;
  accountSet.addAccountGroup(MyMoneyAccount::Asset);
  accountSet.addAccountGroup(MyMoneyAccount::Liability);
  if (withEquityAccounts)
    accountSet.addAccountGroup(MyMoneyAccount::Equity);
  //set the accountset to show closed account if the settings say so
  accountSet.setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());
  accountSet.load(m_ui->m_accountsView);
  connect(m_ui->m_accountsView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::selectAllItems(QTreeWidget* view, const bool state)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
    it_v = view->invisibleRootItem()->child(i);
    if (it_v->flags() & Qt::ItemIsUserCheckable) {
      it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
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
    QVariant idData = it_v->data(0, ItemIdRole);
    if (it_v->flags() & Qt::ItemIsUserCheckable && list.contains(idData.toString())) {
      it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
    }
    selectSubItems(it_v, list, state);
  }

  slotUpdateSelections();
}

void KFindTransactionDlg::setupCategoriesPage()
{
  m_ui->m_categoriesView->setSelectionMode(QTreeWidget::MultiSelection);
  AccountSet categorySet;
  categorySet.addAccountGroup(MyMoneyAccount::Income);
  categorySet.addAccountGroup(MyMoneyAccount::Expense);
  categorySet.load(m_ui->m_categoriesView);
  connect(m_ui->m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
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
    QVariant idData = it_v->data(0, ItemIdRole);
    if (list.contains(idData.toString()))
      it_v->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
    selectSubItems(it_v, list, state);
  }
}

void KFindTransactionDlg::setupDatePage()
{
  int i;
  for (i = MyMoneyTransactionFilter::allDates; i < MyMoneyTransactionFilter::dateOptionCount; ++i) {
    MyMoneyTransactionFilter::translateDateRange(static_cast<MyMoneyTransactionFilter::dateOptionE>(i), m_startDates[i], m_endDates[i]);
  }

  connect(m_ui->m_dateRange, SIGNAL(itemSelected(int)), this, SLOT(slotDateRangeChanged(int)));
  connect(m_ui->m_fromDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));
  connect(m_ui->m_toDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));

  slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
}

void KFindTransactionDlg::slotDateRangeChanged(int idx)
{
  switch (idx) {
    case MyMoneyTransactionFilter::allDates:
    case MyMoneyTransactionFilter::userDefined:
      m_ui->m_fromDate->loadDate(QDate());
      m_ui->m_toDate->loadDate(QDate());
      break;
    default:
      m_ui->m_fromDate->loadDate(m_startDates[idx]);
      m_ui->m_toDate->loadDate(m_endDates[idx]);
      break;
  }
  slotUpdateSelections();
}

void KFindTransactionDlg::slotDateChanged()
{
  int idx;
  for (idx = MyMoneyTransactionFilter::asOfToday; idx < MyMoneyTransactionFilter::dateOptionCount; ++idx) {
    if (m_ui->m_fromDate->date() == m_startDates[idx]
        && m_ui->m_toDate->date() == m_endDates[idx]) {
      break;
    }
  }
  //if no filter matched, set to user defined
  if (idx == MyMoneyTransactionFilter::dateOptionCount)
    idx = MyMoneyTransactionFilter::userDefined;

  m_ui->m_dateRange->blockSignals(true);
  m_ui->m_dateRange->setCurrentItem(static_cast<MyMoneyTransactionFilter::dateOptionE>(idx));
  m_ui->m_dateRange->blockSignals(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupAmountPage()
{
  connect(m_ui->m_amountButton, SIGNAL(clicked()), this, SLOT(slotAmountSelected()));
  connect(m_ui->m_amountRangeButton, SIGNAL(clicked()), this, SLOT(slotAmountRangeSelected()));

  connect(m_ui->m_amountEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_amountFromEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_amountToEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));

  m_ui->m_amountButton->setChecked(true);
  slotAmountSelected();
}

void KFindTransactionDlg::slotAmountSelected()
{
  m_ui->m_amountEdit->setEnabled(true);
  m_ui->m_amountFromEdit->setEnabled(false);
  m_ui->m_amountToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotAmountRangeSelected()
{
  m_ui->m_amountEdit->setEnabled(false);
  m_ui->m_amountFromEdit->setEnabled(true);
  m_ui->m_amountToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupPayeesPage()
{
  m_ui->m_payeesView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_ui->m_payeesView->header()->hide();
  m_ui->m_payeesView->setAlternatingRowColors(true);

  loadPayees();

  m_ui->m_payeesView->sortItems(0, Qt::AscendingOrder);
  m_ui->m_emptyPayeesButton->setCheckState(Qt::Unchecked);

  connect(m_ui->m_allPayeesButton, SIGNAL(clicked()), this, SLOT(slotSelectAllPayees()));
  connect(m_ui->m_clearPayeesButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllPayees()));
  connect(m_ui->m_emptyPayeesButton, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_payeesView, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::loadPayees()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyPayee> list;
  QList<MyMoneyPayee>::Iterator it_l;

  list = file->payeeList();
  // load view
  for (it_l = list.begin(); it_l != list.end(); ++it_l) {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_ui->m_payeesView);
    item->setText(0, (*it_l).name());
    item->setData(0, ItemIdRole, (*it_l).id());
    item->setCheckState(0, Qt::Checked);
  }
}
void KFindTransactionDlg::slotSelectAllPayees()
{
  selectAllItems(m_ui->m_payeesView, true);
}

void KFindTransactionDlg::slotDeselectAllPayees()
{
  selectAllItems(m_ui->m_payeesView, false);
}

void KFindTransactionDlg::setupTagsPage()
{
  m_ui->m_tagsView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_ui->m_tagsView->header()->hide();
  m_ui->m_tagsView->setAlternatingRowColors(true);

  loadTags();

  m_ui->m_tagsView->sortItems(0, Qt::AscendingOrder);
  m_ui->m_emptyTagsButton->setCheckState(Qt::Unchecked);

  connect(m_ui->m_allTagsButton, SIGNAL(clicked()), this, SLOT(slotSelectAllTags()));
  connect(m_ui->m_clearTagsButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllTags()));
  connect(m_ui->m_emptyTagsButton, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_tagsView, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::loadTags()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyTag> list;
  QList<MyMoneyTag>::Iterator it_l;

  list = file->tagList();
  // load view
  for (it_l = list.begin(); it_l != list.end(); ++it_l) {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_ui->m_tagsView);
    item->setText(0, (*it_l).name());
    item->setData(0, ItemIdRole, (*it_l).id());
    item->setCheckState(0, Qt::Checked);
  }
}
void KFindTransactionDlg::slotSelectAllTags()
{
  selectAllItems(m_ui->m_tagsView, true);
}

void KFindTransactionDlg::slotDeselectAllTags()
{
  selectAllItems(m_ui->m_tagsView, false);
}

void KFindTransactionDlg::setupDetailsPage()
{
  connect(m_ui->m_typeBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_stateBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_validityBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));

  connect(m_ui->m_nrButton, SIGNAL(clicked()), this, SLOT(slotNrSelected()));
  connect(m_ui->m_nrRangeButton, SIGNAL(clicked()), this, SLOT(slotNrRangeSelected()));
  connect(m_ui->m_nrEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_nrFromEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));
  connect(m_ui->m_nrToEdit, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateSelections()));

  m_ui->m_nrButton->setChecked(true);
  slotNrSelected();
}

void KFindTransactionDlg::slotNrSelected()
{
  m_ui->m_nrEdit->setEnabled(true);
  m_ui->m_nrFromEdit->setEnabled(false);
  m_ui->m_nrToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotNrRangeSelected()
{
  m_ui->m_nrEdit->setEnabled(false);
  m_ui->m_nrFromEdit->setEnabled(true);
  m_ui->m_nrToEdit->setEnabled(true);
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
    case addTagToFilter:
      m_filter.addTag(id);
      break;
  }
}

void KFindTransactionDlg::scanCheckListItems(const QTreeWidgetItem* item, const opTypeE op)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < item->childCount(); ++i) {
    it_v = item->child(i);
    QVariant idData = it_v->data(0, ItemIdRole);
    if (it_v->flags() & Qt::ItemIsUserCheckable) {
      if (it_v->checkState(0) == Qt::Checked)
        addItemToFilter(op, idData.toString());
    }
    scanCheckListItems(it_v, op);
  }
}

void KFindTransactionDlg::scanCheckListItems(const QTreeWidget* view, const opTypeE op)
{
  QTreeWidgetItem* it_v;

  for (int i = 0; i < view->invisibleRootItem()->childCount(); ++i) {
    it_v = view->invisibleRootItem()->child(i);
    QVariant idData = it_v->data(0, ItemIdRole);
    if (it_v->flags() & Qt::ItemIsUserCheckable) {
      if (it_v->checkState(0) == Qt::Checked) {
        addItemToFilter(op, idData.toString());
      }
    }
    scanCheckListItems(it_v, op);
  }
}

void KFindTransactionDlg::setupFilter()
{
  m_filter.clear();

  // Text tab
  if (!m_ui->m_textEdit->text().isEmpty()) {
    QRegExp exp(m_ui->m_textEdit->text(), m_ui->m_caseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, !m_ui->m_regExp->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp);
    m_filter.setTextFilter(exp, m_ui->m_textNegate->currentIndex() != 0);
  }

  // Account tab
  if (!m_ui->m_accountsView->allItemsSelected()) {
    // retrieve a list of selected accounts
    QStringList list;
    m_ui->m_accountsView->selectedItems(list);

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
  if (m_ui->m_dateRange->currentItem() != 0) {
    m_filter.setDateFilter(m_ui->m_fromDate->date(), m_ui->m_toDate->date());
  }

  // Amount tab
  if ((m_ui->m_amountButton->isChecked() && m_ui->m_amountEdit->isValid())) {
    m_filter.setAmountFilter(m_ui->m_amountEdit->value(), m_ui->m_amountEdit->value());

  } else if ((m_ui->m_amountRangeButton->isChecked()
              && (m_ui->m_amountFromEdit->isValid() || m_ui->m_amountToEdit->isValid()))) {

    MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
    if (m_ui->m_amountFromEdit->isValid())
      from = m_ui->m_amountFromEdit->value();
    if (m_ui->m_amountToEdit->isValid())
      to = m_ui->m_amountToEdit->value();

    m_filter.setAmountFilter(from, to);
  }

  // Categories tab
  if (!m_ui->m_categoriesView->allItemsSelected()) {
    m_filter.addCategory(m_ui->m_categoriesView->selectedItems());
  }

  // Tags tab
  if (m_ui->m_emptyTagsButton->isChecked()) {
    m_filter.addTag(QString());

  } else if (!allItemsSelected(m_ui->m_tagsView)) {
    scanCheckListItems(m_ui->m_tagsView, addTagToFilter);
  }

  // Payees tab
  if (m_ui->m_emptyPayeesButton->isChecked()) {
    m_filter.addPayee(QString());

  } else if (!allItemsSelected(m_ui->m_payeesView)) {
    scanCheckListItems(m_ui->m_payeesView, addPayeeToFilter);
  }

  // Details tab
  if (m_ui->m_typeBox->currentIndex() != 0)
    m_filter.addType(m_ui->m_typeBox->currentIndex());

  if (m_ui->m_stateBox->currentIndex() != 0)
    m_filter.addState(m_ui->m_stateBox->currentIndex());

  if (m_ui->m_validityBox->currentIndex() != 0)
    m_filter.addValidity(m_ui->m_validityBox->currentIndex());

  if (m_ui->m_nrButton->isChecked() && !m_ui->m_nrEdit->text().isEmpty())
    m_filter.setNumberFilter(m_ui->m_nrEdit->text(), m_ui->m_nrEdit->text());

  if (m_ui->m_nrRangeButton->isChecked()
      && (!m_ui->m_nrFromEdit->text().isEmpty() || !m_ui->m_nrToEdit->text().isEmpty())) {
    m_filter.setNumberFilter(m_ui->m_nrFromEdit->text(), m_ui->m_nrToEdit->text());
  }
}

void KFindTransactionDlg::slotSearch()
{
  // perform the search only if the button is enabled
  if (!isButtonEnabled(KDialog::Apply))
    return;

  // setup the filter from the dialog widgets
  setupFilter();

  // filter is setup, now fill the register
  slotRefreshView();

  m_ui->m_register->setFocus();
}

void KFindTransactionDlg::slotRefreshView()
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
  KDialog::showEvent(event);
}

void KFindTransactionDlg::loadView()
{
  // setup sort order
  m_ui->m_register->setSortOrder(KMyMoneyGlobalSettings::sortSearchView());

  // clear out old data
  m_ui->m_register->clear();

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

    KMyMoneyRegister::Register::transactionFactory(m_ui->m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
    { // debug stuff
      if (split.shares().isNegative()) {
        payment += split.shares().abs();
      } else {
        deposit += split.shares().abs();
      }
    }
  }

  // add the group markers
  m_ui->m_register->addGroupMarkers();

  // sort the transactions according to the sort setting
  m_ui->m_register->sortItems();

  // remove trailing and adjacent markers
  m_ui->m_register->removeUnwantedGroupMarkers();

  // turn on the ledger lens for the register
  m_ui->m_register->setLedgerLensForced();

  m_ui->m_register->updateRegister(true);

  m_ui->m_register->setFocusToTop();
  m_ui->m_register->selectItem(m_ui->m_register->focusItem());

#ifdef KMM_DEBUG
  m_ui->m_foundText->setText(i18np("Found %1 matching transaction (D %2 / P %3 = %4)",
                                   "Found %1 matching transactions (D %2 / P %3 = %4)", splitCount, deposit.formatMoney("", 2), payment.formatMoney("", 2), (deposit - payment).formatMoney("", 2)));
#else
  m_ui->m_foundText->setText(i18np("Found %1 matching transaction", "Found %1 matching transactions", splitCount));
#endif

  m_ui->m_tabWidget->setTabEnabled(m_ui->m_tabWidget->indexOf(m_ui->m_resultPage), true);
  m_ui->m_tabWidget->setCurrentIndex(m_ui->m_tabWidget->indexOf(m_ui->m_resultPage));

  QTimer::singleShot(10, this, SLOT(slotRightSize()));
}

void KFindTransactionDlg::slotRightSize()
{
  m_ui->m_register->update();
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
  KDialog::resizeEvent(ev);

  if (!m_ui->m_register->isVisible())
    return;

  // resize the register
  int w = m_ui->m_register->contentsRect().width();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  m_ui->m_register->adjustColumn(1);
  m_ui->m_register->adjustColumn(2);
  m_ui->m_register->adjustColumn(5);

  m_ui->m_register->setColumnWidth(6, m_debitWidth);
  m_ui->m_register->setColumnWidth(7, m_creditWidth);

  for (int i = 0; i < m_ui->m_register->columnCount(); ++i) {
    switch (i) {
      case 4:     // skip the one, we want to set
        break;
      default:
        w -= m_ui->m_register->columnWidth(i);
        break;
    }
  }

  m_ui->m_register->setColumnWidth(4, w);
}


void KFindTransactionDlg::slotSelectTransaction()
{
  QList<KMyMoneyRegister::RegisterItem*> list = m_ui->m_register->selectedItems();
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
      if (w == m_ui->m_register) {
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

void KFindTransactionDlg::slotShowHelp()
{
  QString anchor = m_helpAnchor[m_ui->m_criteriaTab->currentWidget()];
  if (anchor.isEmpty())
    anchor = QString("details.search");

  KToolInvocation::invokeHelp(anchor);
}

void KFindTransactionDlg::slotSortOptions()
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
