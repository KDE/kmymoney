/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneyvalidationfeedback.h"
#include "ui_kmymoneyvalidationfeedback.h"

#include <QIcon>
#include "icons/icons.h"
#include "widgetenums.h"

using namespace eWidgets;
using namespace Icons;

class KMyMoneyValidationFeedbackPrivate
{
  Q_DISABLE_COPY(KMyMoneyValidationFeedbackPrivate)

public:
  KMyMoneyValidationFeedbackPrivate() :
    ui(new Ui::KMyMoneyValidationFeedback),
    type(ValidationFeedback::MessageType::None)
  {
  }

  ~KMyMoneyValidationFeedbackPrivate()
  {
    delete ui;
  }

  Ui::KMyMoneyValidationFeedback *ui;
  ValidationFeedback::MessageType type;
};

KMyMoneyValidationFeedback::KMyMoneyValidationFeedback(QWidget *parent) :
    QWidget(parent),
    d_ptr(new KMyMoneyValidationFeedbackPrivate)
{
  Q_D(KMyMoneyValidationFeedback);
  d->ui->setupUi(this);
  setHidden(true);
  QSizePolicy newSizePolicy = sizePolicy();
  newSizePolicy.setControlType(QSizePolicy::Label);
  newSizePolicy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
  newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
  setSizePolicy(newSizePolicy);
}

KMyMoneyValidationFeedback::~KMyMoneyValidationFeedback()
{
  Q_D(KMyMoneyValidationFeedback);
  delete d;
}

/**
 * @todo Set icon size according to text size
 */
void KMyMoneyValidationFeedback::setFeedback(ValidationFeedback::MessageType type, QString message)
{
  Q_D(KMyMoneyValidationFeedback);
  d->type = type;

  if (type == ValidationFeedback::MessageType::None) {
    if (message.isEmpty() || message == d->ui->label->text())
      setHidden(true);
  } else {
    setHidden(false);
    d->ui->label->setText(message);
    QIcon icon;
    switch (type) {
      case ValidationFeedback::MessageType::Error:
        icon = Icons::get(Icon::DialogError);
        break;
      case ValidationFeedback::MessageType::Positive:
      case ValidationFeedback::MessageType::Information:
        icon = Icons::get(Icon::DialogInformation);
        break;
      case ValidationFeedback::MessageType::Warning:
      default:
        icon = Icons::get(Icon::DialogWarning);
    }
    d->ui->icon->setPixmap(icon.pixmap(24));
  }
}

void KMyMoneyValidationFeedback::removeFeedback()
{
  setHidden(true);
}

void KMyMoneyValidationFeedback::removeFeedback(ValidationFeedback::MessageType type, QString message)
{
  Q_D(KMyMoneyValidationFeedback);
  if (d->type == type && d->ui->label->text() == message)
    removeFeedback();
}

