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

#include "kmymoneyvalidationfeedback.h"
#include "ui_kmymoneyvalidationfeedback.h"

#include <QIcon>

class KMyMoneyValidationFeedbackPrivate
{
public:
  KMyMoneyValidationFeedback::MessageType type;
};

KMyMoneyValidationFeedback::KMyMoneyValidationFeedback(QWidget *parent) :
    QWidget(parent),
    ui( new Ui::KMyMoneyValidationFeedback ),
    d_ptr( new KMyMoneyValidationFeedbackPrivate )
{
  ui->setupUi(this);
  setHidden( true );
  QSizePolicy newSizePolicy = sizePolicy();
  newSizePolicy.setControlType( QSizePolicy::Label );
  newSizePolicy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
  newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
  setSizePolicy( newSizePolicy );
}

KMyMoneyValidationFeedback::~KMyMoneyValidationFeedback()
{
  Q_D(KMyMoneyValidationFeedback);

  delete ui;
  delete d;
}

/**
 * @todo Set icon size according to text size
 */
void KMyMoneyValidationFeedback::setFeedback(KMyMoneyValidationFeedback::MessageType type, QString message)
{
  Q_D(KMyMoneyValidationFeedback);
  d->type = type;

  if (type == None) {
    if (message.isEmpty() || message == ui->label->text())
      setHidden(true);
  } else {
    setHidden(false);
    ui->label->setText(message);
    QIcon icon;
    switch (type) {
      case Error:
        icon = QIcon::fromTheme("dialog-error");
        break;
      case Positive:
      case Information:
        icon = QIcon::fromTheme("dialog-information");
        break;
      case Warning:
      default:
        icon = QIcon::fromTheme("dialog-warning");
    }
    ui->icon->setPixmap( icon.pixmap(24) );
  }
}

void KMyMoneyValidationFeedback::removeFeedback()
{
  setHidden(true);
}

void KMyMoneyValidationFeedback::removeFeedback(KMyMoneyValidationFeedback::MessageType type, QString message)
{
  Q_D(KMyMoneyValidationFeedback);

  if ( d->type == type && ui->label->text() == message )
    removeFeedback();
}

