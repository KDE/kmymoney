/***************************************************************************
                          knewbankdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
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

#include "knewbankdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLineEdit>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewbankdlg.h"

#include "mymoneyinstitution.h"

class KNewBankDlgPrivate
{
  Q_DISABLE_COPY(KNewBankDlgPrivate)

public:
  KNewBankDlgPrivate() :
    ui(new Ui::KNewBankDlg)
  {
  }

  ~KNewBankDlgPrivate()
  {
    delete ui;
  }

  Ui::KNewBankDlg    *ui;
  MyMoneyInstitution  m_institution;
};

KNewBankDlg::KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent) :
  QDialog(parent),
  d_ptr(new KNewBankDlgPrivate)
{
  Q_D(KNewBankDlg);
  d->ui->setupUi(this);
  d->m_institution = institution;
  setModal(true);

  d->ui->nameEdit->setFocus();
  d->ui->nameEdit->setText(institution.name());
  d->ui->cityEdit->setText(institution.city());
  d->ui->streetEdit->setText(institution.street());
  d->ui->postcodeEdit->setText(institution.postcode());
  d->ui->telephoneEdit->setText(institution.telephone());
  d->ui->bicEdit->setText(institution.value("bic"));
  d->ui->sortCodeEdit->setText(institution.sortcode());

  connect(d->ui->buttonBox, &QDialogButtonBox::accepted, this, &KNewBankDlg::okClicked);
  connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(d->ui->nameEdit, &QLineEdit::textChanged, this, &KNewBankDlg::institutionNameChanged);
  institutionNameChanged(d->ui->nameEdit->text());

  auto requiredFields = new kMandatoryFieldGroup(this);
  requiredFields->setOkButton(d->ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present
  requiredFields->add(d->ui->nameEdit);
}

void KNewBankDlg::institutionNameChanged(const QString &_text)
{
  Q_D(KNewBankDlg);
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!_text.isEmpty());
}

KNewBankDlg::~KNewBankDlg()
{
  Q_D(KNewBankDlg);
  delete d;
}

void KNewBankDlg::okClicked()
{
  Q_D(KNewBankDlg);
  if (d->ui->nameEdit->text().isEmpty()) {
    KMessageBox::information(this, i18n("The institution name field is empty.  Please enter the name."), i18n("Adding New Institution"));
    d->ui->nameEdit->setFocus();
    return;
  }

  d->m_institution.setName(d->ui->nameEdit->text());
  d->m_institution.setTown(d->ui->cityEdit->text());
  d->m_institution.setStreet(d->ui->streetEdit->text());
  d->m_institution.setPostcode(d->ui->postcodeEdit->text());
  d->m_institution.setTelephone(d->ui->telephoneEdit->text());
  d->m_institution.setValue("bic", d->ui->bicEdit->text());
  d->m_institution.setSortcode(d->ui->sortCodeEdit->text());

  accept();
}

const MyMoneyInstitution& KNewBankDlg::institution()
{
  Q_D(KNewBankDlg);
  return d->m_institution;
}
