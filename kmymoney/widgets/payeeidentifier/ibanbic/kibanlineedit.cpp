/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kibanlineedit.h"

#include "ibanvalidator.h"
#include "kmymoneyvalidationfeedback.h"

KIbanLineEdit::KIbanLineEdit(QWidget* parent)
    : KLineEdit(parent)
{
  ibanValidator *const validatorPtr = new ibanValidator;
  setValidator(validatorPtr);
}

const ibanValidator* KIbanLineEdit::validator() const
{
  return qobject_cast<const ibanValidator*>(KLineEdit::validator());
}
