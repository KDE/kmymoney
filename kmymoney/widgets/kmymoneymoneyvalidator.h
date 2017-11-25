/***************************************************************************
                          kmymoneymoneyvalidator.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYVALIDATOR_H
#define KMYMONEYVALIDATOR_H

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
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  explicit KMyMoneyMoneyValidator(QObject * parent);
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
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
