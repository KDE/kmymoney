/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

