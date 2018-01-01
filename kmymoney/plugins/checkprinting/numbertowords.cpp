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
#include <KLocalizedString>

MyMoneyMoneyToWordsConverter::MyMoneyMoneyToWordsConverter()
{
  // Single-digit and small number names
  m_smallNumbers << i18nc("@item the number 0", "zero") << i18nc("@item the number 1", "one") << i18nc("@item the number 2", "two") << i18nc("@item the number 3", "three") << i18nc("@item the number 4", "four") << i18nc("@item the number 5", "five") << i18nc("@item the number 6", "six") << i18nc("@item the number 7", "seven") << i18nc("@item the number 8", "eight") << i18nc("@item the number 9", "nine") << i18nc("@item the number 10", "ten") << i18nc("@item the number 11", "eleven") << i18nc("@item the number 12", "twelve") << i18nc("@item the number 13", "thirteen") << i18nc("@item the number 14", "fourteen") << i18nc("@item the number 15", "fifteen") << i18nc("@item the number 16", "sixteen") << i18nc("@item the number 17", "seventeen") << i18nc("@item the number 18", "eighteen") << i18nc("@item the number 19", "nineteen");

  // Tens number names from twenty upwards
  m_tens << "" << "" << i18nc("@item the number 20", "twenty") << i18nc("@item the number 30", "thirty") << i18nc("@item the number 40", "forty") << i18nc("@item the number 50", "fifty") << i18nc("@item the number 60", "sixty") << i18nc("@item the number 70", "seventy") << i18nc("@item the number 80", "eighty") << i18nc("@item the number 90", "ninety");

  // Scale number names for use during recombination
  m_scaleNumbers << "" << i18nc("@item the number 1000", "thousand") << i18nc("@item the number 1,000,000", "million") << i18nc("@item the number 1,000,000,000", "billion");
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
    groupText += m_smallNumbers[hundreds] + i18nc("@item This comes after the hundred value digit", " hundred");

    if (tensUnits != 0)
      groupText += i18nc("@item This comes after the hunder text if the tens unit is different from 0", " and ");
  }

  // Determine the tens and units
  int tens = tensUnits / 10;
  int units = tensUnits % 10;

  // Tens rules
  if (tens >= 2) {
    groupText += m_tens[tens];
    if (units != 0)
      //This comes after the tens text if the unit is not 0
      groupText += QString(" ") + m_smallNumbers[units];
  } else if (tensUnits != 0)
    groupText += m_smallNumbers[tensUnits];

  return groupText;
}

QString MyMoneyMoneyToWordsConverter::convert(const MyMoneyMoney & money, signed64 denom)
{
  // Zero rule
  if (money.isZero())
    return m_smallNumbers[0];

  // hold three-digit groups
  QList<int> digitGroups;

  int integer = static_cast<int>(money.toDouble()); // retain the integer part
  int fraction = qRound((money.toDouble() - integer) * denom);

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
        prefix += appendAnd ? i18nc("@item Appears last as separator", " and ") : i18nc("@item Separator", ", ");

      // Opportunity to add 'and' is ended
      appendAnd = false;

      // Add the three-digit group to the combined string
      combined = prefix + combined;
    }
  }

  if (fraction != 0)
    return i18nc("@label The first argument is the amount in words, the second is the fractional part and the third is the denominator of the fractional part",
                 "%1 and %2/%3", combined, fraction, denom);
  else
    return combined;
}
