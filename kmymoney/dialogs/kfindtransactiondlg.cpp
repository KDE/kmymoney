/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include "kfindtransactiondlg.h"
#include "kfindtransactiondlg_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QKeyEvent>
#include <QList>
#include <QEvent>
#include <QPushButton>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KComboBox>
#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneyedit.h"
#include "kmymoneysettings.h"
#include "register.h"
#include "transaction.h"
#include "daterangedlg.h"

#include "ui_kfindtransactiondlg.h"
#include "ui_ksortoptiondlg.h"


KSortOptionDlg::KSortOptionDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::KSortOptionDlg)
{
  ui->setupUi(this);
}

KSortOptionDlg::~KSortOptionDlg()
{
  delete ui;
}

void KSortOptionDlg::setSortOption(const QString& option, const QString& def)
{
  if (option.isEmpty()) {
    ui->m_sortOption->setSettings(def);
    ui->m_useDefault->setChecked(true);
  } else {
    ui->m_sortOption->setSettings(option);
    ui->m_useDefault->setChecked(false);
  }
}

QString KSortOptionDlg::sortOption() const
{
  QString rc;
  if (!ui->m_useDefault->isChecked()) {
    rc = ui->m_sortOption->settings();
  }
  return rc;
}

void KSortOptionDlg::hideDefaultButton()
{
  ui->m_useDefault->hide();
}

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, bool withEquityAccounts) :
  QDialog(parent),
  d_ptr(new KFindTransactionDlgPrivate(this))
{
  Q_D(KFindTransactionDlg);
  d->init(withEquityAccounts);
}

KFindTransactionDlg::KFindTransactionDlg(KFindTransactionDlgPrivate &dd, QWidget *parent, bool withEquityAccounts) :
  QDialog(parent),
  d_ptr(&dd)
{
  Q_D(KFindTransactionDlg);
  d->init(withEquityAccounts);
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  Q_D(KFindTransactionDlg);
  delete d;
}

void KFindTransactionDlg::slotReset()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_textEdit->setText(QString());
  d->ui->m_regExp->setChecked(false);
  d->ui->m_caseSensitive->setChecked(false);
  d->ui->m_textNegate->setCurrentItem(0);

  d->ui->m_amountEdit->setEnabled(true);
  d->ui->m_amountFromEdit->setEnabled(false);
  d->ui->m_amountToEdit->setEnabled(false);
  d->ui->m_amountEdit->loadText(QString());
  d->ui->m_amountFromEdit->loadText(QString());
  d->ui->m_amountToEdit->loadText(QString());
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

  d->ui->m_tabWidget->setTabEnabled(d->ui->m_tabWidget->indexOf(d->ui->m_resultPage), false);
  d->ui->m_tabWidget->setCurrentIndex(d->ui->m_tabWidget->indexOf(d->ui->m_criteriaTab));

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  d->m_dateRange->slotReset();
  slotUpdateSelections();
}

void KFindTransactionDlg::slotUpdateSelections()
{
  Q_D(KFindTransactionDlg);
  QString txt;

  // Text tab
  if (!d->ui->m_textEdit->text().isEmpty()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Text");
    d->ui->m_regExp->setEnabled(QRegExp(d->ui->m_textEdit->text()).isValid());
  } else
    d->ui->m_regExp->setEnabled(false);

  d->ui->m_caseSensitive->setEnabled(!d->ui->m_textEdit->text().isEmpty());
  d->ui->m_textNegate->setEnabled(!d->ui->m_textEdit->text().isEmpty());

  // Account tab
  if (!d->ui->m_accountsView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Account");
  }

  if (d->m_dateRange->dateRange() != eMyMoney::TransactionFilter::Date::All) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Date");
  }

  // Amount tab
  if ((d->ui->m_amountButton->isChecked() && d->ui->m_amountEdit->isValid())
      || (d->ui->m_amountRangeButton->isChecked()
          && (d->ui->m_amountFromEdit->isValid() || d->ui->m_amountToEdit->isValid()))) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Amount");
  }

  // Categories tab
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Category");
  }

  // Tags tab
  if (!d->allItemsSelected(d->ui->m_tagsView)
      || d->ui->m_emptyTagsButton->isChecked()) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Tags");
  }
  d->ui->m_tagsView->setEnabled(!d->ui->m_emptyTagsButton->isChecked());

  // Payees tab
  if (!d->allItemsSelected(d->ui->m_payeesView)
      || d->ui->m_emptyPayeesButton->isChecked()) {
    if (!txt.isEmpty())
      txt += ", ";
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
      txt += ", ";
    txt += i18n("Details");
  }

  //Show a warning about transfers if Categories are filtered - bug #1523508
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    d->ui->m_transferWarning->setText(i18n("Warning: Filtering by Category will exclude all transfers from the results."));
  } else {
    d->ui->m_transferWarning->setText("");
  }

  // disable the search button if no selection is made
  emit selectionNotEmpty(!txt.isEmpty());

  if (txt.isEmpty()) {
    txt = i18nc("No selection", "(None)");
  }
  d->ui->m_selectedCriteria->setText(i18n("Current selections: %1", txt));
}

void KFindTransactionDlg::slotAmountSelected()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_amountEdit->setEnabled(true);
  d->ui->m_amountFromEdit->setEnabled(false);
  d->ui->m_amountToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotAmountRangeSelected()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_amountEdit->setEnabled(false);
  d->ui->m_amountFromEdit->setEnabled(true);
  d->ui->m_amountToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotSelectAllPayees()
{
  Q_D(KFindTransactionDlg);
  d->selectAllItems(d->ui->m_payeesView, true);
}

void KFindTransactionDlg::slotDeselectAllPayees()
{
  Q_D(KFindTransactionDlg);
  d->selectAllItems(d->ui->m_payeesView, false);
}

void KFindTransactionDlg::slotSelectAllTags()
{
  Q_D(KFindTransactionDlg);
  d->selectAllItems(d->ui->m_tagsView, true);
}

void KFindTransactionDlg::slotDeselectAllTags()
{
  Q_D(KFindTransactionDlg);
  d->selectAllItems(d->ui->m_tagsView, false);
}

void KFindTransactionDlg::slotNrSelected()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_nrEdit->setEnabled(true);
  d->ui->m_nrFromEdit->setEnabled(false);
  d->ui->m_nrToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotNrRangeSelected()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_nrEdit->setEnabled(false);
  d->ui->m_nrFromEdit->setEnabled(true);
  d->ui->m_nrToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotSearch()
{
  Q_D(KFindTransactionDlg);
  // perform the search only if the button is enabled
  if (!d->ui->buttonBox->button(QDialogButtonBox::Apply)->isEnabled())
    return;

  // setup the filter from the dialog widgets
  d->setupFilter();

  // filter is setup, now fill the register
  slotRefreshView();

  d->ui->m_register->setFocus();
}

void KFindTransactionDlg::slotRefreshView()
{
  Q_D(KFindTransactionDlg);
  d->m_needReload = true;
  if (isVisible()) {
    d->loadView();
    d->m_needReload = false;
  }
}

void KFindTransactionDlg::showEvent(QShowEvent* event)
{
  Q_D(KFindTransactionDlg);
  if (d->m_needReload) {
    d->loadView();
    d->m_needReload = false;
  }
  QDialog::showEvent(event);
}

void KFindTransactionDlg::slotRightSize()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_register->update();
}

void KFindTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  Q_D(KFindTransactionDlg);
  // Columns
  // 1 = Date
  // 2 = Account
  // 4 = Detail
  // 5 = C
  // 6 = Payment
  // 7 = Deposit

  // don't forget the resizer
  QDialog::resizeEvent(ev);

  if (!d->ui->m_register->isVisible())
    return;

  // resize the register
  int w = d->ui->m_register->contentsRect().width();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  d->ui->m_register->adjustColumn(1);
  d->ui->m_register->adjustColumn(2);
  d->ui->m_register->adjustColumn(5);

  d->ui->m_register->setColumnWidth(6, m_debitWidth);
  d->ui->m_register->setColumnWidth(7, m_creditWidth);

  for (auto i = 0; i < d->ui->m_register->columnCount(); ++i) {
    switch (i) {
      case 4:     // skip the one, we want to set
        break;
      default:
        w -= d->ui->m_register->columnWidth(i);
        break;
    }
  }

  d->ui->m_register->setColumnWidth(4, w);
}


void KFindTransactionDlg::slotSelectTransaction()
{
  Q_D(KFindTransactionDlg);
  auto list = d->ui->m_register->selectedItems();
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
  Q_D(KFindTransactionDlg);
  auto rc = false;

  if (o->isWidgetType()) {
    if (e->type() == QEvent::KeyPress) {
      const QWidget* w = dynamic_cast<const QWidget*>(o);
      QKeyEvent *k = static_cast<QKeyEvent *>(e);
      if (w == d->ui->m_register) {
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
  Q_D(KFindTransactionDlg);
  auto anchor = d->m_helpAnchor[d->ui->m_criteriaTab->currentWidget()];
  if (anchor.isEmpty())
    anchor = QString("details.search");

  KHelpClient::invokeHelp(anchor);
}

void KFindTransactionDlg::slotSortOptions()
{
  QPointer<KSortOptionDlg> dlg = new KSortOptionDlg(this);

  dlg->setSortOption(KMyMoneySettings::sortSearchView(), QString());
  dlg->hideDefaultButton();

  if (dlg->exec() == QDialog::Accepted) {
    QString sortOrder = dlg->sortOption();
    if (sortOrder != KMyMoneySettings::sortSearchView()) {
      KMyMoneySettings::setSortSearchView(sortOrder);
      slotRefreshView();
    }
  }
  delete dlg;
}
