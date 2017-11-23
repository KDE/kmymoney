/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian David <christian-david@web.de>
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

#ifndef BICVALIDATOR_H
#define BICVALIDATOR_H

#include <QValidator>
#include "payeeidentifier_iban_bic_widgets_export.h"
#include "kmymoneyvalidationfeedback.h"

namespace eWidgets { namespace ValidationFeedback { enum class MessageType; } }

class PAYEEIDENTIFIER_IBAN_BIC_WIDGETS_EXPORT bicValidator : public QValidator
{
  Q_OBJECT

public:
  explicit bicValidator(QObject* parent = 0);
  virtual QValidator::State validate(QString& , int&) const;

  static QPair<eWidgets::ValidationFeedback::MessageType, QString> validateWithMessage(const QString&);
};

#endif // BICVALIDATOR_H
