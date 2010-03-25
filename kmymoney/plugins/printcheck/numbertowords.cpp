/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "numbertowords.h"

// KDE includes
#include <klocale.h>
#include <kglobal.h>

MyMoneyMoneyToWordsConverter::MyMoneyMoneyToWordsConverter()
{
  // Single-digit and small number names
  m_smallNumbers << i18n("zero") << i18n("one") << i18n("two") << i18n("three") << i18n("four") << i18n("five") << i18n("six") << i18n("seven") << i18n("eight") << i18n("nine") << i18n("ten") << i18n("eleven") << i18n("twelve") << i18n("thirteen") << i18n("fourteen") << i18n("fifteen") << i18n("sixteen") << i18n("seventeen") << i18n("eighteen") << i18n("nineteen");

  // Tens number names from twenty upwards
  m_tens << "" << "" << i18n("twenty") << i18n("thirty") << i18n("forty") << i18n("fifty") << i18n("sixty") << i18n("seventy") << i18n("eighty") << i18n("ninety");

  // Scale number names for use during recombination
  m_scaleNumbers << "" << i18n("thousand") << i18n("million") << i18n("billion");
}

// Converts a three-digit group into English words
QString MyMoneyMoneyToWordsConverter::convertTreeDigitGroup(int threeDigitNumber)
{
  // Initialise the return text
  QString groupText;

  // Determine the hundreds and the remainder
  int hundreds = threeDigitNumber / 100;
  int tensUnits = threeDigitNumber % 100;

  // Hundreds rules
  if (hundreds != 0) {
    groupText += m_smallNumbers[hundreds] + i18nc("This comes after the hunder value digit", " hundred");

    if (tensUnits != 0)
      groupText += i18nc("This comes after the hunder text if the tens unit is different from 0", " and ");
  }

  // Determine the tens and units
  int tens = tensUnits / 10;
  int units = tensUnits % 10;

  // Tens rules
  if (tens >= 2) {
    groupText += m_tens[tens];
    if (units != 0)
      groupText += i18nc("This comes after the tens text if the unit is not 0", " ") + m_smallNumbers[units];
  } else if (tensUnits != 0)
    groupText += m_smallNumbers[tensUnits];

  return groupText;
}

QString MyMoneyMoneyToWordsConverter::convert(const MyMoneyMoney & money)
{
  // Zero rule
  if (static_cast<int>(money) == 0)
    return m_smallNumbers[0];

  // hold three-digit groups
  QList<int> digitGroups;

  int precision = KGlobal::locale()->fracDigits();
  int integer = static_cast<int>(money.toDouble()); // retain the integer part
  int fraction = static_cast<int>((money.toDouble() - integer) * MyMoneyMoney::precToDenom(precision));

  // Extract the three-digit groups
  for (int i = 0; i < 4; i++) {
    digitGroups.push_back(integer % 1000);
    integer /= 1000;
  }

  // Convert each three-digit group to words
  QStringList groupText;

  for (int i = 0; i < 4; i++)
    groupText << convertTreeDigitGroup(digitGroups[i]);

  // Recombine the three-digit groups
  QString combined = groupText[0];

  // Determine whether an 'and' is needed
  bool appendAnd = (digitGroups[0] > 0) && (digitGroups[0] < 100) && (fraction == 0);

  // Process the remaining groups in turn, smallest to largest
  for (int i = 1; i < 4; i++) {
    // Only add non-zero items
    if (digitGroups[i] != 0) {
      // Build the string to add as a prefix
      QString prefix = groupText[i] + ' ' + m_scaleNumbers[i];
      if (!combined.isEmpty())
        prefix += appendAnd ? i18nc("Appears last as separator", " and ") : i18nc("Separator", ", ");

      // Opportunity to add 'and' is ended
      appendAnd = false;

      // Add the three-digit group to the combined string
      combined = prefix + combined;
    }
  }

  if (fraction != 0)
    return i18nc("The first argument is the amount in words, the second is the fractional part and the third is the denominator of the fractional part",
                 "%1 and %2/%3", combined, fraction, MyMoneyMoney::precToDenom(precision));
  else
    return combined;
}
