/*
 * Copyright 2016       Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneymoney.h"

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class AmountValidator : public QDoubleValidator
{
  Q_OBJECT

public:
  /**
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  explicit AmountValidator(QObject * parent);
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p digits after the decimal
    * point.
    */
  explicit AmountValidator(double bottom, double top, int decimals,
                         QObject * parent);
  /**
    * Destructs the validator.
    */
  ~AmountValidator();

  /** Overloaded for internal reasons. The API is not affected. */
  QValidator::State validate(QString & input, int & pos) const override;
};

#endif
