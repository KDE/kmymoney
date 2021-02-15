/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ibanbicitemedit.h"
#include "ui_ibanbicitemedit.h"

#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/payeeidentifiertyped.h"

struct ibanBicItemEdit::Private {
  Ui::ibanBicItemEdit* ui;
  payeeIdentifier m_identifier;
};

ibanBicItemEdit::ibanBicItemEdit(QWidget* parent)
    : QWidget(parent),
    d(new Private)
{
  d->ui = new Ui::ibanBicItemEdit;
  d->ui->setupUi(this);
  setFocusProxy(d->ui->ibanEdit);

  connect(d->ui->ibanEdit, &KIbanLineEdit::textChanged, this, &ibanBicItemEdit::updateIdentifier);
  connect(d->ui->bicEdit, &KBicEdit::textChanged, this, &ibanBicItemEdit::updateIdentifier);

  connect(d->ui->ibanEdit, &KIbanLineEdit::textChanged, this, &ibanBicItemEdit::ibanChanged);
  connect(d->ui->bicEdit, &KBicEdit::textChanged, this, &ibanBicItemEdit::bicChanged);

  connect(d->ui->ibanEdit, &KIbanLineEdit::returnPressed, this, &ibanBicItemEdit::editFinished);
  connect(d->ui->bicEdit, &KBicEdit::returnPressed, this, &ibanBicItemEdit::editFinished);
}

void ibanBicItemEdit::editFinished()
{
  emit commitData(this);
  emit closeEditor(this);
}

payeeIdentifier ibanBicItemEdit::identifier() const
{
  return d->m_identifier;
}

QString ibanBicItemEdit::bic() const
{
  return d->ui->bicEdit->text();
}

QString ibanBicItemEdit::iban() const
{
  return d->ui->ibanEdit->text();
}

void ibanBicItemEdit::setIdentifier(const payeeIdentifier& ident)
{
  try {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> identTyped(ident);
    d->ui->bicEdit->setText(identTyped->storedBic());
    d->ui->ibanEdit->setText(identTyped->paperformatIban());
    d->m_identifier = ident;
  } catch (const payeeIdentifier::empty &) {
  } catch (const payeeIdentifier::badCast &) {
  }
}

void ibanBicItemEdit::setBic(const QString& bic)
{
  d->ui->bicEdit->setText(bic);
}

void ibanBicItemEdit::setIban(const QString& iban)
{
  d->ui->ibanEdit->setText(payeeIdentifiers::ibanBic::ibanToPaperformat(iban));
}

void ibanBicItemEdit::updateIdentifier()
{
  if (d->m_identifier.isNull())
    d->m_identifier = payeeIdentifier(d->m_identifier.id(), new payeeIdentifiers::ibanBic);

  const QString iban = payeeIdentifiers::ibanBic::ibanToElectronic(d->ui->ibanEdit->text());
  const QString bic = d->ui->bicEdit->text();
  bool changed = false;

  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ident(d->m_identifier);
  if (ident->storedBic() != bic) {
    ident->setBic(bic);
    changed = true;
  }

  if (ident->electronicIban() != iban) {
    ident->setElectronicIban(iban);
    changed = true;
  }
  d->m_identifier = ident;

  if (changed) {
    emit identifierChanged(d->m_identifier);
    emit commitData(this);
  }
}
