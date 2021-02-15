/*
 * SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

  connect(d->ui.accountNumberEdit, SIGNAL(textChanged(QString)), this, SIGNAL(accountNumberChannged(QString)));
  connect(d->ui.institutionCodeEdit, SIGNAL(textChanged(QString)), this, SIGNAL(institutionCodeChanged(QString)));

  connect(d->ui.accountNumberEdit, SIGNAL(returnPressed()), this, SLOT(editFinished()));
  connect(d->ui.institutionCodeEdit, SIGNAL(returnPressed()), this, SLOT(editFinished()));
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
