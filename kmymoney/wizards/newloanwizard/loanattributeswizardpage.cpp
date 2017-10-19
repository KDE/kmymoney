/***************************************************************************
loanattributeswizardpage  -  description
-------------------
begin                : Mon Dec 30 2013
copyright            : (C) 2013 by Jeremy Whiting
email                : jpwhiting@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "loanattributeswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewbankdlg.h"
#include "mymoneyfile.h"

LoanAttributesWizardPage::LoanAttributesWizardPage(QWidget *parent)
    : LoanAttributesWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("institution", m_qcomboboxInstitutions);
  connect(m_qcomboboxInstitutions, SIGNAL(currentIndexChanged(int)), this, SIGNAL(completeChanged()));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));

  m_qcomboboxInstitutions->clear();

  // Are we forcing the user to use institutions?
  m_qcomboboxInstitutions->addItem(i18n("(No Institution)"));
  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    QList<MyMoneyInstitution> list;
    file->institutionList(list);
    qSort(list);

    Q_FOREACH(const MyMoneyInstitution &institution, list) {
      m_qcomboboxInstitutions->addItem(institution.name());
    }
  } catch (const MyMoneyException &e) {
    qDebug("Exception in institution load: %s", qPrintable(e.what()));
  }
}

/**
 * Update the "Next" button
 */
bool LoanAttributesWizardPage::isComplete() const
{
  return true;
}

void LoanAttributesWizardPage::initializePage()
{
}

void LoanAttributesWizardPage::setInstitution(const QString &institutionName)
{
  if (institutionName.isEmpty()) {
    m_qcomboboxInstitutions->setCurrentItem(i18n("(No Institution)"));
  } else {
    m_qcomboboxInstitutions->setCurrentItem(institutionName, false);
  }
}

void LoanAttributesWizardPage::slotNewClicked()
{
  MyMoneyInstitution institution;

  QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution, this);
  if (dlg->exec() && dlg != 0) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile *file = MyMoneyFile::instance();

      institution = dlg->institution();
      file->addInstitution(institution);
      ft.commit();
      initializePage();
      m_qcomboboxInstitutions->setCurrentItem(institution.name(), false);
    } catch (const MyMoneyException &) {
      KMessageBox::information(this, i18n("Cannot add institution"));
    }
  }
  delete dlg;
}
