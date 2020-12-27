/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nationalaccountedit.h"

#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"

#include "ui_nationalaccountedit.h"

struct nationalAccountEdit::Private {
  Ui::nationalAccountEdit ui;
  payeeIdentifier m_identifier;
};

nationalAccountEdit::nationalAccountEdit(QWidget* parent)
    : QWidget(parent),
    d(new Private)
{
  d->ui.setupUi(this);
  setFocusProxy(d->ui.accountNumberEdit);

  connect(d->ui.accountNumberEdit, &QLineEdit::textChanged, this, &nationalAccountEdit::accountNumberChannged);
  connect(d->ui.institutionCodeEdit, &QLineEdit::textChanged, this, &nationalAccountEdit::institutionCodeChanged);

  connect(d->ui.accountNumberEdit, &QLineEdit::returnPressed, this, &nationalAccountEdit::editFinished);
  connect(d->ui.institutionCodeEdit, &QLineEdit::returnPressed, this, &nationalAccountEdit::editFinished);
}

payeeIdentifier nationalAccountEdit::identifier() const
{
  if (!d->m_identifier.isNull()) {
    try {
      payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident(d->m_identifier);
      ident->setAccountNumber(d->ui.accountNumberEdit->text());
      ident->setBankCode(d->ui.institutionCodeEdit->text());
    } catch (const payeeIdentifier::empty &) {
    } catch (const payeeIdentifier::badCast &) {
    }
  }
  return d->m_identifier;
}

void nationalAccountEdit::editFinished()
{
  emit commitData(this);
  emit closeEditor(this);
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
  } catch (const payeeIdentifier::empty &) {
  } catch (const payeeIdentifier::badCast &) {
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
