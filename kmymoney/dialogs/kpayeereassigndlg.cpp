/***************************************************************************
                          kpayeereassigndlg.cpp
                             -------------------
    copyright            : (C) 2005 by Andreas Nicolai <ghorwin@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kpayeereassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QLineEdit>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kpayeereassigndlg.h"

#include <kmymoneymvccombo.h>

/** This lookup table needs to be in sync with KPayeeReassignDlg::OperationType enum */
static const char * labelText[KPayeeReassignDlg::TypeCount] = {
  I18N_NOOP("To be able to merge previous selected payees, please select a payee from the list below or create a new one."),
  I18N_NOOP("The transactions associated with the selected payees need to be re-assigned to a different payee before the selected payees can be deleted. Please select a payee from the list below."),
};

class KPayeeReassignDlgPrivate
{
  Q_DISABLE_COPY(KPayeeReassignDlgPrivate)

public:
  KPayeeReassignDlgPrivate() :
    ui(new Ui::KPayeeReassignDlg)
  {
  }

  ~KPayeeReassignDlgPrivate()
  {
    delete ui;
  }

  Ui::KPayeeReassignDlg *ui;
  KPayeeReassignDlg::OperationType m_type;
};

KPayeeReassignDlg::KPayeeReassignDlg(KPayeeReassignDlg::OperationType type, QWidget* parent) :
  QDialog(parent),
  d_ptr(new KPayeeReassignDlgPrivate)
{
  Q_D(KPayeeReassignDlg);
  d->ui->setupUi(this);
  d->m_type = type;
  auto mandatory = new KMandatoryFieldGroup(this);
  mandatory->add(d->ui->payeeCombo);
  mandatory->setOkButton(d->ui->buttonBox->button(QDialogButtonBox::Ok));
  d->ui->textLabel1->setText(i18n(labelText[d->m_type]));
}

KPayeeReassignDlg::~KPayeeReassignDlg()
{
  Q_D(KPayeeReassignDlg);
  delete d;
}

QString KPayeeReassignDlg::show(const QList<MyMoneyPayee>& payeeslist)
{
  Q_D(KPayeeReassignDlg);
  if (payeeslist.isEmpty())
    return QString(); // no payee available? nothing can be selected...

  d->ui->payeeCombo->loadPayees(payeeslist);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // allow to return the text (new payee) if type is Merge
  if (d->m_type == TypeMerge && d->ui->payeeCombo->selectedItem().isEmpty())
    return d->ui->payeeCombo->lineEdit()->text();

  // otherwise return index of selected payee
  return d->ui->payeeCombo->selectedItem();
}


bool KPayeeReassignDlg::addToMatchList() const
{
  Q_D(const KPayeeReassignDlg);
  return d->ui->m_copyToMatchList->isChecked();
}

void KPayeeReassignDlg::accept()
{
  Q_D(KPayeeReassignDlg);
  // force update of d->ui->payeeCombo
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (d->m_type == TypeDelete && d->ui->payeeCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow new payees to be created. Please pick a payee from the list."), i18n("Payee creation"));
  } else {
    QDialog::accept();
  }
}
