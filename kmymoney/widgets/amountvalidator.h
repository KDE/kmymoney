/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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
};

#endif
