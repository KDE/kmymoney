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

#include "kibanlineedit.h"

#include <QTimer>

#include "ibanvalidator.h"

struct KIbanLineEdit::Private {
  QTimer* timer;
  KMyMoneyValidationFeedback::MessageType delayedMessageType;
  QString delayedMessage;
};

KIbanLineEdit::KIbanLineEdit(QWidget* parent)
    : KLineEdit(parent),
    d_ptr(new KIbanLineEdit::Private)
{
  Q_D();

  ibanValidator *const validatorPtr = new ibanValidator;
  setValidator(validatorPtr);

  d->timer = new QTimer(this);
  d->timer->setSingleShot(true);
  d->timer->setInterval(2000);

  connect(validatorPtr, SIGNAL(feedback(KMyMoneyValidationFeedback::MessageType,QString)), this, SLOT(delayFeedback(KMyMoneyValidationFeedback::MessageType,QString)));
  connect(this, SIGNAL(returnPressed()), SLOT(emitFeedback()));
  connect(d->timer, SIGNAL(timeout()), this, SLOT(emitFeedback()));
}

const ibanValidator* KIbanLineEdit::validator() const
{
  return qobject_cast<const ibanValidator*>(KLineEdit::validator());
}

void KIbanLineEdit::delayFeedback(KMyMoneyValidationFeedback::MessageType type, QString message)
{
  Q_D();
  d->timer->stop();

  if (type < d->delayedMessageType) {
    // Directly show feedback if something got better
    emit validatorFeedback(type, message);
  } else {
    d->timer->start();
  }

  // The timer will execute in the event loop, so setting the
  // messages here is early enough.
  d->delayedMessageType = type;
  d->delayedMessage = message;
}

void KIbanLineEdit::focusOutEvent(QFocusEvent* ev)
{
  KLineEdit::focusOutEvent(ev);
  emitFeedback();
}

void KIbanLineEdit::emitFeedback()
{
  Q_D();
  emit validatorFeedback(d->delayedMessageType, d->delayedMessage);
}
