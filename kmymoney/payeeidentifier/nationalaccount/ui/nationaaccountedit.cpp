/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include "nationaaccountedit.h"

#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"

#include "ui_nationaaccountedit.h"

struct nationalAccountEdit::Private {
  Ui::nationalAccountEdit ui;
  payeeIdentifier m_identifier;
};

nationalAccountEdit::nationalAccountEdit(QWidget* parent)
    : QWidget(parent),
    d(new Private)
{
  d->ui.setupUi(this);

  connect(d->ui.accountNumberEdit, SIGNAL(textChanged(QString)), this, SIGNAL(accountNumberChannged(QString)));
  connect(d->ui.institutionCodeEdit, SIGNAL(textChanged(QString)), this, SIGNAL(institutionCodeChanged(QString)));
}

payeeIdentifier nationalAccountEdit::identifier() const
{
  if (!d->m_identifier.isNull()) {
    try {
      payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident(d->m_identifier);
      ident->setAccountNumber(d->ui.accountNumberEdit->text());
      ident->setBankCode(d->ui.institutionCodeEdit->text());
    } catch (payeeIdentifier::exception&) {
    }
  }
  return d->m_identifier;
}

QString nationalAccountEdit::accountNumber() const
{
  return d->ui.accountNumberEdit->text();
}

QString nationalAccountEdit::institutionCode() const
{
  return d->ui.institutionCodeEdit->text();
}

void nationalAccountEdit::setIdentifier(const payeeIdentifier& ident)
{
  try {
    payeeIdentifierTyped<payeeIdentifiers::nationalAccount> identTyped(ident);
    d->ui.accountNumberEdit->setText(identTyped->accountNumber());
    d->ui.institutionCodeEdit->setText(identTyped->bankCode());
    d->m_identifier = ident;
  } catch (payeeIdentifier::exception&) {
  }
}

void nationalAccountEdit::setAccountNumber(const QString& accountNumber)
{
  d->ui.accountNumberEdit->setText(accountNumber);
}

void nationalAccountEdit::setInstitutionCode(const QString& institutionCode)
{
  d->ui.institutionCodeEdit->setText(institutionCode);
}
