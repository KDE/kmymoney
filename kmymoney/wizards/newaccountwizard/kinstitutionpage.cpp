/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinstitutionpage.h"
#include "kinstitutionpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinstitutionpage.h"

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccounttypepage.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "knewinstitutiondlg.h"
#include "wizardpage.h"

namespace NewAccountWizard
{
  InstitutionPage::InstitutionPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new InstitutionPagePrivate(wizard),StepInstitution, this, wizard)
  {
    Q_D(InstitutionPage);
    d->ui->setupUi(this);
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &InstitutionPage::slotLoadWidgets);
    connect(d_func()->ui->m_newInstitutionButton, &QAbstractButton::clicked, this, &InstitutionPage::slotNewInstitution);
    connect(d_func()->ui->m_institutionComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &InstitutionPage::slotSelectInstitution);

    slotLoadWidgets();
    d_func()->ui->m_institutionComboBox->setCurrentItem(0);
    slotSelectInstitution(0);
  }

  InstitutionPage::~InstitutionPage()
  {
  }

  void InstitutionPage::slotLoadWidgets()
  {
    Q_D(InstitutionPage);
    d_func()->ui->m_institutionComboBox->clear();

    d->m_list.clear();
    d->m_list = MyMoneyFile::instance()->institutionList();
    std::sort(d->m_list.begin(), d->m_list.end());

    QList<MyMoneyInstitution>::const_iterator it_l;
    d->ui->m_institutionComboBox->addItem(QString());
    for (it_l = d->m_list.constBegin(); it_l != d->m_list.constEnd(); ++it_l) {
        d->ui->m_institutionComboBox->addItem((*it_l).name());
      }
  }

  void InstitutionPage::slotNewInstitution()
  {
    Q_D(InstitutionPage);
    MyMoneyInstitution institution;

    KNewInstitutionDlg::newInstitution(institution);

    if (!institution.id().isEmpty()) {
        QList<MyMoneyInstitution>::const_iterator it_l;
        int i = 0;
        for (it_l = d->m_list.constBegin(); it_l != d->m_list.constEnd(); ++it_l) {
            if ((*it_l).id() == institution.id()) {
                // select the item and remember that the very first one is the empty item
                d->ui->m_institutionComboBox->setCurrentIndex(i + 1);
                slotSelectInstitution(i + 1);
                d->ui->m_accountNumber->setFocus();
                break;
              }
            ++i;
          }
      }
  }

  void InstitutionPage::slotSelectInstitution(const int index)
  {
    Q_D(InstitutionPage);
    d->ui->m_accountNumber->setEnabled(index != 0);
    d->ui->m_iban->setEnabled(index != 0);
  }

  void InstitutionPage::selectExistingInstitution(const QString& id)
  {
    Q_D(InstitutionPage);
    for (int i = 0; i < d->m_list.length(); ++i) {
        if (d->m_list[i].id() == id) {
            d->ui->m_institutionComboBox->setCurrentIndex(i + 1);
            slotSelectInstitution(i + 1);
            break;
          }
      }
  }

  const MyMoneyInstitution& InstitutionPage::institution() const
  {
    Q_D(const InstitutionPage);
    static MyMoneyInstitution emptyInstitution;
    if (d->ui->m_institutionComboBox->currentIndex() == 0)
      return emptyInstitution;

    return d->m_list[d->ui->m_institutionComboBox->currentIndex()-1];
  }

  KMyMoneyWizardPage* InstitutionPage::nextPage() const
  {
    Q_D(const InstitutionPage);
    return d->m_wizard->d_func()->m_accountTypePage;
  }

  QWidget* InstitutionPage::initialFocusWidget() const
  {
    Q_D(const InstitutionPage);
    return d->ui->m_institutionComboBox;
  }
}
