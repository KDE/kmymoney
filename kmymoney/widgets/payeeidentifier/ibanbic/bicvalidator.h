/*
 * Copyright 2013-2015  Christian Dávid <christian-david@web.de>
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

#ifndef BICVALIDATOR_H
#define BICVALIDATOR_H

#include <QValidator>
#include "kmm_base_widgets_export.h"
#include "kmymoneyvalidationfeedback.h"

namespace eWidgets { namespace ValidationFeedback { enum class MessageType; } }

class KMM_BASE_WIDGETS_EXPORT bicValidator : public QValidator
{
  Q_OBJECT

public:
  explicit bicValidator(QObject* parent = 0);
  QValidator::State validate(QString& , int&) const final override;

  static QPair<eWidgets::ValidationFeedback::MessageType, QString> validateWithMessage(const QString&);
};

#endif // BICVALIDATOR_H
