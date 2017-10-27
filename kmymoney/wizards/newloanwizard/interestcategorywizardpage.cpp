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

#include "knewaccountdlg.h"
#include "mymoneyfile.h"
#include "icons/icons.h"

using namespace Icons;

InterestCategoryWizardPage::InterestCategoryWizardPage(QWidget *parent)
    : InterestCategoryWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestAccountEdit", m_interestAccountEdit, "selectedItems");

  connect(m_interestAccountEdit, SIGNAL(stateChanged()), this, SIGNAL(completeChanged()));
  m_interestAccountEdit->removeButtons();

  // load button icons
  KGuiItem createCategoryButtonItem(i18n("&Create..."),
                                    QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                                    i18n("Create a new category"),
                                    i18n("Use this to open the new account editor"));
  KGuiItem::assign(m_createCategoryButton, createCategoryButtonItem);
  connect(m_createCategoryButton, SIGNAL(clicked()), this, SLOT(slotCreateCategory()));
}

/**
 * Update the "Next" button
 */
bool InterestCategoryWizardPage::isComplete() const
{
  return m_interestAccountEdit->selectedItems().count() > 0;
}

void InterestCategoryWizardPage::slotCreateCategory()
{
  MyMoneyAccount acc, base;
  MyMoneyFile* file = MyMoneyFile::instance();

  if (field("borrowButton").toBool()) {
    base = file->expense();
    acc.setAccountType(eMyMoney::Account::Expense);
  } else {
    base = file->income();
    acc.setAccountType(eMyMoney::Account::Income);
  }
  acc.setParentAccountId(base.id());

  QPointer<KNewAccountDlg> dlg = new KNewAccountDlg(acc, true, true);
  if (dlg->exec() == QDialog::Accepted) {
    acc = dlg->account();

    MyMoneyFileTransaction ft;
    try {
      QString id;
      id = file->createCategory(base, acc.name());
      if (id.isEmpty())
        throw MYMONEYEXCEPTION("failure while creating the account hierarchy");

      ft.commit();

      m_interestAccountEdit->setSelected(id);

    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to add account: %1", e.what()));
    }
  }
  delete dlg;
}
