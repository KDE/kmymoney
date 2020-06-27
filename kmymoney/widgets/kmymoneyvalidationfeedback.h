/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYVALIDATIONFEEDBACK_H
#define KMYMONEYVALIDATIONFEEDBACK_H

#include <QWidget>

#include "kmm_base_widgets_export.h"

namespace eWidgets { namespace ValidationFeedback { enum class MessageType; } }

class KMyMoneyValidationFeedbackPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyValidationFeedback : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyValidationFeedback)

public:
  explicit KMyMoneyValidationFeedback(QWidget* parent = nullptr);
  ~KMyMoneyValidationFeedback();

public Q_SLOTS:
  /**
   * @brief Removes the shown feedback
   */
  void removeFeedback();

  /**
   * @brief Removes a specific feedback
   *
   * Removes the feedback only if type and message fit. This is useful
   * if several objects are connected to setFeedback().
   */
  void removeFeedback(eWidgets::ValidationFeedback::MessageType type, QString message);

  /**
   * @brief Show a feedback
   *
   * If type == None and !message.isEmpty() holds, the feedback is only hidden if
   * the currently shown message equals message.
   */
  void setFeedback(eWidgets::ValidationFeedback::MessageType type, QString message);

private:
  KMyMoneyValidationFeedbackPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyValidationFeedback)
};

#endif // KMYMONEYVALIDATIONFEEDBACK_H
