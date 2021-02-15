/*
loanattributeswizardpage  -  description
-------------------
    SPDX-FileCopyrightText: 2013 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "loanattributeswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_loanattributeswizardpage.h"

#include "knewinstitutiondlg.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyexception.h"

LoanAttributesWizardPage::LoanAttributesWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::LoanAttributesWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("institution", ui->m_qcomboboxInstitutions);
  connect(ui->m_qcomboboxInstitutions, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QWizardPage::completeChanged);
  connect(ui->m_qbuttonNew, &QAbstractButton::clicked, this, &LoanAttributesWizardPage::slotNewClicked);

  ui->m_qcomboboxInstitutions->clear();

  // Are we forcing the user to use institutions?
  ui->m_qcomboboxInstitutions->addItem(i18n("(No Institution)"));
  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    auto list = file->institutionList();
    std::sort(list.begin(), list.end());

    Q_FOREACH(const MyMoneyInstitution &institution, list) {
      ui->m_qcomboboxInstitutions->addItem(institution.name());
    }
  } catch (const MyMoneyException &e) {
    qDebug("Exception in institution load: %s", e.what());
  }
}

LoanAttributesWizardPage::~LoanAttributesWizardPage()
{
  delete ui;
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
    ui->m_qcomboboxInstitutions->setCurrentItem(i18n("(No Institution)"));
  } else {
    ui->m_qcomboboxInstitutions->setCurrentItem(institutionName, false);
  }
}

void LoanAttributesWizardPage::slotNewClicked()
{
  MyMoneyInstitution institution;

  QPointer<KNewInstitutionDlg> dlg = new KNewInstitutionDlg(institution, this);
  if (dlg->exec() && dlg != 0) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile *file = MyMoneyFile::instance();

      institution = dlg->institution();
      file->addInstitution(institution);
      ft.commit();
      initializePage();
      ui->m_qcomboboxInstitutions->setCurrentItem(institution.name(), false);
    } catch (const MyMoneyException &) {
      KMessageBox::information(this, i18n("Cannot add institution"));
    }
  }
  delete dlg;
}
