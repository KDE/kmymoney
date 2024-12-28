/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IBANVALIDATOR_H
#define IBANVALIDATOR_H

#include "kmm_base_widgets_export.h"

#include <QValidator>

#include "kmymoneyvalidationfeedback.h"

namespace eWidgets {
namespace ValidationFeedback {
enum class MessageType;
}
}

class KMM_BASE_WIDGETS_EXPORT ibanValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ibanValidator(QObject* parent = nullptr);
    State validate(QString& input, int& pos) const final override;
    void fixup(QString& input) const final override;

    static QPair<eWidgets::ValidationFeedback::MessageType, QString> validateWithMessage(const QString&);
};

#endif // IBANVALIDATOR_H
