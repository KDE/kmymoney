/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ibanbicitemedit.h"
#include "ui_ibanbicitemedit.h"

#include "payeeidentifier/ibanandbic/ibanbic.h"
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

  connect(d->ui->ibanEdit, SIGNAL(textChanged(QString)), this, SLOT(updateIdentifier()));
  connect(d->ui->bicEdit, SIGNAL(textChanged(QString)), this, SLOT(updateIdentifier()));

  connect(d->ui->ibanEdit, SIGNAL(textChanged(QString)), this, SIGNAL(ibanChanged(QString)));
  connect(d->ui->bicEdit, SIGNAL(textChanged(QString)), this, SIGNAL(bicChanged(QString)));

  connect(d->ui->ibanEdit, SIGNAL(returnPressed()), this, SLOT(editFinished()));
  connect(d->ui->bicEdit, SIGNAL(returnPressed()), this, SLOT(editFinished()));
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
