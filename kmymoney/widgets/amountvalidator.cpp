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

#include "amountvalidator.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

AmountValidator::AmountValidator(QObject * parent) :
    QDoubleValidator(parent)
{
}

AmountValidator::AmountValidator(double bottom, double top, int decimals,
    QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
}

AmountValidator::~AmountValidator()
{
}

/*
 * The code of the following function is taken from kdeui/knumvalidator.cpp
 * and adjusted to always use the monetary symbols defined in the KDE System Settings
 */
QValidator::State AmountValidator::validate(QString & input, int & _p) const

{
  QString s = input;
  QLocale locale;
  // ok, we have to re-format the number to have:
  // 1. decimalSymbol == '.'
  // 2. negativeSign  == '-'
  // 3. positiveSign  == <empty>
  // 4. thousandsSeparator() == <empty> (we don't check that there
  //    are exactly three decimals between each separator):
  QString decimalPoint = locale.decimalPoint(),
              n = locale.negativeSign(),
                  p = locale.positiveSign(),
                      separatorCharacter = locale.groupSeparator();
  // first, delete p's and t's:
  if (!p.isEmpty())
    for (int idx = s.indexOf(p) ; idx >= 0 ; idx = s.indexOf(p, idx))
      s.remove(idx, p.length());


  if (!separatorCharacter.isEmpty())
    for (int idx = s.indexOf(separatorCharacter) ; idx >= 0 ; idx = s.indexOf(separatorCharacter, idx))
      s.remove(idx, separatorCharacter.length());

  // then, replace the d's and n's
  if ((!n.isEmpty() && n.indexOf('.') != -1) ||
      (!decimalPoint.isEmpty() && decimalPoint.indexOf('-') != -1)) {
    // make sure we don't replace something twice:
    qWarning() << "KDoubleValidator: decimal symbol contains '-' or "
    "negative sign contains '.' -> improve algorithm" << endl;
    return Invalid;
  }

  if (!decimalPoint.isEmpty() && decimalPoint != ".")
    for (int idx = s.indexOf(decimalPoint) ; idx >= 0 ; idx = s.indexOf(decimalPoint, idx + 1))
      s.replace(idx, decimalPoint.length(), ".");

  if (!n.isEmpty() && n != "-")
    for (int idx = s.indexOf(n) ; idx >= 0 ; idx = s.indexOf(n, idx + 1))
      s.replace(idx, n.length(), "-");

  // TODO: port KF5 (support for paren around negative numbers)
#if 0
  // Take care of monetary parens around the value if selected via
  // the locale settings.
  // If the lead-in or lead-out paren is present, remove it
  // before passing the string to the QDoubleValidator
  if (l->negativeMonetarySignPosition() == KLocale::ParensAround
      || l->positiveMonetarySignPosition() == KLocale::ParensAround) {
    QRegExp regExp("^(\\()?([\\d-\\.]*)(\\))?$");
    if (s.indexOf(regExp) != -1) {
      s = regExp.cap(2);
    }
  }
#endif

  // check for non numeric values (QDoubleValidator allows an 'e', we don't)
  QRegExp nonNumeric("[^\\d-\\.]+");
  if (s.indexOf(nonNumeric) != -1)
    return Invalid;

  // check for minus sign trailing the number
  QRegExp trailingMinus("^([^-]*)\\w*-$");
  if (s.indexOf(trailingMinus) != -1) {
    s = QString("-%1").arg(trailingMinus.cap(1));
  }

  // check for the maximum allowed number of decimal places
  int decPos = s.indexOf('.');
  if (decPos != -1) {
    if (decimals() == 0)
      return Invalid;
    if (((int)(s.length()) - decPos) > decimals())
      return Invalid;
  }

  // If we have just a single minus sign, we are done
  if (s == QString("-"))
    return Acceptable;

  QValidator::State rc = QDoubleValidator::validate(s, _p);
  // TODO: port KF5 (support for paren around negative numbers)
#if 0
  if (rc == Acceptable) {
    // If the numeric value is acceptable, we check if the parens
    // are ok. If only the lead-in is present, the return value
    // is intermediate, if only the lead-out is present then it
    // definitely is invalid. Nevertheless, we check for parens
    // only, if the locale settings have it enabled.
    if (l->negativeMonetarySignPosition() == KLocale::ParensAround
        || l->positiveMonetarySignPosition() == KLocale::ParensAround) {
      int tmp = input.count('(') - input.count(')');
      if (tmp > 0)
        rc = Intermediate;
      else if (tmp < 0)
        rc = Invalid;
    }
  }
#endif
  return rc;
}
