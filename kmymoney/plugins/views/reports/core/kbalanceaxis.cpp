/*
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
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

#include "kbalanceaxis.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


KBalanceAxis::KBalanceAxis()
    : KChart::CartesianAxis()
{
}

KBalanceAxis::KBalanceAxis(KChart::AbstractCartesianDiagram* parent)
    : KChart::CartesianAxis(parent)
{
}

const QString KBalanceAxis::customizedLabel(const QString& label) const
{
  //TODO: make precision variable
  int precision = 2;

  //format as money using base currency or the included accounts
  return QLocale().toString(label.toDouble(), precision);
}
