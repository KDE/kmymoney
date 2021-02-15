/*
    SPDX-FileCopyrightText: 2001-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYMONEYVALIDATOR_H
#define KMYMONEYMONEYVALIDATOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDoubleValidator>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyMoneyValidator : public QDoubleValidator
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyMoneyValidator)

public:
  /**
    * Construct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  explicit KMyMoneyMoneyValidator(QObject * parent);
  /**
    * Construct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p digits after the decimal
    * point.
    */
  explicit KMyMoneyMoneyValidator(double bottom, double top, int decimals,
                                  QObject * parent);
  /**
    * Destructs the validator.
    */
  ~KMyMoneyMoneyValidator();

  /** Overloaded for internal reasons. The API is not affected. */
  QValidator::State validate(QString & input, int & pos) const override;
};

#endif
