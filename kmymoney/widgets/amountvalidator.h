/*
    SPDX-FileCopyrightText: 2016-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AMOUNTVALIDATOR_H
#define AMOUNTVALIDATOR_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDoubleValidator>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class is a specialization of the QDoubleValidator
  * which uses the StandardNotation instead of the
  * ScientificNotation as the default
  *
  * @author Thomas Baumgart
  */
class AmountValidator : public QDoubleValidator
{
    Q_OBJECT

public:
    explicit AmountValidator(QObject * parent);
    explicit AmountValidator(double bottom, double top, int decimals,
                             QObject * parent);

    virtual QValidator::State validate(QString& input, int& pos) const override;
};

#endif
