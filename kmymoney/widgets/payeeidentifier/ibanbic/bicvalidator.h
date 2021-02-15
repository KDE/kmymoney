/*
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
