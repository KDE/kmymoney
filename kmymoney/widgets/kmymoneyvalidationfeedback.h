/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#ifndef KMYMONEYVALIDATIONFEEDBACK_H
#define KMYMONEYVALIDATIONFEEDBACK_H

#include <QWidget>

#include "kmm_widgets_export.h"

namespace Ui
{
class KMyMoneyValidationFeedback;
}

class KMM_WIDGETS_EXPORT KMyMoneyValidationFeedback : public QWidget
{
  Q_OBJECT

public:
  KMyMoneyValidationFeedback(QWidget *parent = 0);
  ~KMyMoneyValidationFeedback();

  enum MessageType {
    None,
    Positive,
    Information,
    Warning,
    Error
  };

public slots:
  /**
   * @brief Removes the shown feedback
   */
  void removeFeedback();

  /**
   * @brief Removes a sepecific feedback
   *
   * Removes the feedback only if type and message fit. This is useful
   * if several objects are connected to setFeedback().
   */
  void removeFeedback(KMyMoneyValidationFeedback::MessageType type, QString message);

  /**
   * @brief Show a feedback
   *
   * If type == None and !message.isEmpty() holds, the feedback is only hidden if
   * the currently shown message equals message.
   */
  void setFeedback(KMyMoneyValidationFeedback::MessageType type, QString message);

private:
  Ui::KMyMoneyValidationFeedback* ui;

  class Private;
  Private* d_ptr;
  Q_DECLARE_PRIVATE();
};

#endif // KMYMONEYVALIDATIONFEEDBACK_H
