/***************************************************************************
                          kmymoneydateedit.h
                          -------------------
    copyright            : (C) 2016 by Thomas Baumgart
    email                : tbaumgart@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneydateedit.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KGlobal>
#include <KLocale>

namespace
{
const int DATE_POPUP_TIMEOUT = 1500;
const QDate INVALID_DATE = QDate(1800, 1, 1);
}


KMyMoneyDateEdit::KMyMoneyDateEdit(QWidget* parent)
  : QDateEdit(parent)
{
}

void KMyMoneyDateEdit::setDateFormat(const QString& dateFormat)
{
  QString format = dateFormat.toLower();
  QString order, separator;
  bool lastWasPercent = false;
  for (int i = 0; i < format.length(); ++i) {
    // DD.MM.YYYY is %d.%m.%y
    // dD.mM.YYYY is %e.%n.%y
    // SHORTWEEKDAY, dD SHORTMONTH YYYY is %a, %e %b %Y
    if (lastWasPercent == true) {
      if (format[i] == 'y' || format[i] == 'm' || format[i] == 'n' || format[i] == 'd' || format[i] == 'e') {
        if (format[i] == 'n')
          format[i] = 'm';
        if (format[i] == 'e')
          format[i] = 'd';
        order += format[i];
      }

    } else if (format[i] == '%') {
      lastWasPercent = true;
      continue;

    } else if (separator.isEmpty() && !order.isEmpty())
      separator = format[i];

    if (order.length() == 3)
      break;
    lastWasPercent = false;
  }

  // see if we find a known format. If it's unknown, then we use YMD (international)
  if (order == "mdy") {
    setDisplayFormat(QString("MM%1dd%2yyyy").arg(separator, separator));
  } else if (order == "dmy") {
    setDisplayFormat(QString("dd%1MM%2yyyy").arg(separator, separator));
  } else if (order == "ydm") {
    setDisplayFormat(QString("yyyy%1dd%2MM").arg(separator, separator));
  } else {
    setDisplayFormat(QString("yyyy%1MM%2dd").arg(separator, separator));
  }
}
