/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYVALIDATIONFEEDBACK_H
#define KMYMONEYVALIDATIONFEEDBACK_H

#include <QWidget>

#include "kmm_base_widgets_export.h"

namespace eWidgets {
namespace ValidationFeedback {
enum class MessageType;
}
}

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
