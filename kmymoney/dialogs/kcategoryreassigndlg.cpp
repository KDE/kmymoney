/***************************************************************************
                          kcategoryreassigndlg.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "mymoneyfile.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"

KCategoryReassignDlg::KCategoryReassignDlg(QWidget* parent) :
    KCategoryReassignDlgDecl(parent)
{
  kMandatoryFieldGroup* mandatory = new kMandatoryFieldGroup(this);
  mandatory->add(m_category);
  mandatory->setOkButton(buttonBox->button(QDialogButtonBox::Ok));
}

KCategoryReassignDlg::~KCategoryReassignDlg()
{
}

QString KCategoryReassignDlg::show(const MyMoneyAccount& category)
{
  if (category.id().isEmpty())
    return QString(); // no payee available? nothing can be selected...

  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Income);
  set.addAccountGroup(MyMoneyAccount::Expense);
  set.load(m_category->selector());

  // remove the category we are about to delete
  m_category->selector()->removeItem(category.id());

  // make sure the available categories have the same currency
  QStringList list;
  QStringList::const_iterator it_a;
  m_category->selector()->itemList(list);
  for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
    if (acc.currencyId() != category.currencyId())
      m_category->selector()->removeItem(*it_a);
  }

  // reload the list
  m_category->selector()->itemList(list);

  // if there is no category for reassignment left, we bail out
  if (list.isEmpty()) {
    KMessageBox::sorry(this, QString("<qt>") + i18n("At least one transaction/schedule still references the category <b>%1</b>.  However, at least one category with the same currency must exist so that the transactions/schedules can be reassigned.", category.name()) + QString("</qt>"));
    return QString();
  }

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return index of selected payee
  return m_category->selectedItem();
}


void KCategoryReassignDlg::accept()
{
  // force update of payeeCombo
  buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (m_category->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow new categories to be created. Please pick a category from the list."), i18n("Category creation"));
  } else {
    KCategoryReassignDlgDecl::accept();
  }
}
