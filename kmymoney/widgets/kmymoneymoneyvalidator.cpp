/***************************************************************************
                          kmymoneymoneyvalidator.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kmymoneymoneyvalidator.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


KMyMoneyMoneyValidator::KMyMoneyMoneyValidator(QObject * parent) :
  QDoubleValidator(parent)
{
  setLocale(QLocale::c());
}

KMyMoneyMoneyValidator::KMyMoneyMoneyValidator(double bottom, double top, int decimals,
                                               QObject * parent) :
  QDoubleValidator(bottom, top, decimals, parent)
{
  setLocale(QLocale::c());
}

KMyMoneyMoneyValidator::~KMyMoneyMoneyValidator()
{
}

/*
 * The code of the following function is taken from kdeui/knumvalidator.cpp
 * and adjusted to always use the monetary symbols defined in the KDE System Settings
 */
QValidator::State KMyMoneyMoneyValidator::validate(QString & input, int & _p) const
{
  Q_UNUSED(_p)
  QString s = input;
  // TODO: port this to kf5
#if 0
  KLocale * l = KLocale::global();
  // ok, we have to re-format the number to have:
  // 1. decimalSymbol == '.'
  // 2. negativeSign  == '-'
  // 3. positiveSign  == <empty>
  // 4. thousandsSeparator() == <empty> (we don't check that there
  //    are exactly three decimals between each separator):
  QString d = l->monetaryDecimalSymbol(),
      n = l->negativeSign(),
      p = l->positiveSign(),
      t = l->monetaryThousandsSeparator();
  // first, delete p's and t's:
  if (!p.isEmpty())
    for (int idx = s.indexOf(p) ; idx >= 0 ; idx = s.indexOf(p, idx))
      s.remove(idx, p.length());


  if (!t.isEmpty())
    for (int idx = s.indexOf(t) ; idx >= 0 ; idx = s.indexOf(t, idx))
      s.remove(idx, t.length());

  // then, replace the d's and n's
  if ((!n.isEmpty() && n.indexOf('.') != -1) ||
      (!d.isEmpty() && d.indexOf('-') != -1)) {
    // make sure we don't replace something twice:
    qWarning() << "KDoubleValidator: decimal symbol contains '-' or "
                  "negative sign contains '.' -> improve algorithm" << endl;
    return Invalid;
  }

  if (!d.isEmpty() && d != ".")
    for (int idx = s.indexOf(d) ; idx >= 0 ; idx = s.indexOf(d, idx + 1))
      s.replace(idx, d.length(), ".");

  if (!n.isEmpty() && n != "-")
    for (int idx = s.indexOf(n) ; idx >= 0 ; idx = s.indexOf(n, idx + 1))
      s.replace(idx, n.length(), "-");

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
  return rc;
#else
  return Acceptable;
#endif
}
