/***************************************************************************
                         interestcategorywizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "interestcategorywizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestcategorywizardpage.h"

#include "knewaccountdlg.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "icons/icons.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

using namespace Icons;

InterestCategoryWizardPage::InterestCategoryWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::InterestCategoryWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestAccountEdit", ui->m_interestAccountEdit, "selectedItems");

  connect(ui->m_interestAccountEdit, &KMyMoneySelector::stateChanged, this, &QWizardPage::completeChanged);
  ui->m_interestAccountEdit->removeButtons();

  // load button icons
  KGuiItem createCategoryButtonItem(i18n("&Create..."),
                                    Icons::get(Icon::DocumentNew),
                                    i18n("Create a new category"),
                                    i18n("Use this to open the new account editor"));
  KGuiItem::assign(ui->m_createCategoryButton, createCategoryButtonItem);
  connect(ui->m_createCategoryButton, &QAbstractButton::clicked, this, &InterestCategoryWizardPage::slotCreateCategory);
}

InterestCategoryWizardPage::~InterestCategoryWizardPage()
{
  delete ui;
}

/**
 * Update the "Next" button
 */
bool InterestCategoryWizardPage::isComplete() const
{
  return ui->m_interestAccountEdit->selectedItems().count() > 0;
}

void InterestCategoryWizardPage::slotCreateCategory()
{
  MyMoneyAccount acc, base;
  MyMoneyFile* file = MyMoneyFile::instance();

  if (field("borrowButton").toBool()) {
    base = file->expense();
    acc.setAccountType(eMyMoney::Account::Type::Expense);
  } else {
    base = file->income();
    acc.setAccountType(eMyMoney::Account::Type::Income);
  }
  acc.setParentAccountId(base.id());

  QPointer<KNewAccountDlg> dlg = new KNewAccountDlg(acc, true, true, nullptr, QString());
  if (dlg->exec() == QDialog::Accepted) {
    acc = dlg->account();

    MyMoneyFileTransaction ft;
    try {
      QString id;
      id = file->createCategory(base, acc.name());
      if (id.isEmpty())
        throw MYMONEYEXCEPTION("failure while creating the account hierarchy");

      ft.commit();

      ui->m_interestAccountEdit->setSelected(id);

    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to add account: %1", e.what()));
    }
  }
  delete dlg;
}
