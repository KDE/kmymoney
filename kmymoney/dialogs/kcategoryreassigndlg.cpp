/*
    SPDX-FileCopyrightText: 2007-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcategoryreassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcategoryreassigndlg.h"

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyenums.h"

KCategoryReassignDlg::KCategoryReassignDlg(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::KCategoryReassignDlg)
{
  ui->setupUi(this);
  auto mandatory = new KMandatoryFieldGroup(this);
  mandatory->add(ui->m_category);
  mandatory->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok));
}

KCategoryReassignDlg::~KCategoryReassignDlg()
{
  delete ui;
}

QString KCategoryReassignDlg::show(const MyMoneyAccount& category)
{
  if (category.id().isEmpty())
    return QString(); // no payee available? nothing can be selected...

  AccountSet set;
  set.addAccountGroup(eMyMoney::Account::Type::Income);
  set.addAccountGroup(eMyMoney::Account::Type::Expense);
  set.load(ui->m_category->selector());

  // remove the category we are about to delete
  ui->m_category->selector()->removeItem(category.id());

  // make sure the available categories have the same currency
  QStringList list;
  QStringList::const_iterator it_a;
  ui->m_category->selector()->itemList(list);
  for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
    if (acc.currencyId() != category.currencyId())
      ui->m_category->selector()->removeItem(*it_a);
  }

  // reload the list
  ui->m_category->selector()->itemList(list);

  // if there is no category for reassignment left, we bail out
  if (list.isEmpty()) {
    KMessageBox::sorry(this, QString("<qt>") + i18n("At least one transaction/schedule still references the category <b>%1</b>.  However, at least one category with the same currency must exist so that the transactions/schedules can be reassigned.", category.name()) + QString("</qt>"));
    return QString();
  }

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return index of selected payee
  return ui->m_category->selectedItem();
}


void KCategoryReassignDlg::accept()
{
  // force update of payeeCombo
  ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (ui->m_category->selectedItem().isEmpty())
    KMessageBox::information(this, i18n("This dialog does not allow new categories to be created. Please pick a category from the list."), i18n("Category creation"));
  else
    QDialog::accept();
}
