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
  m_smallNumbers << i18nc("@item the number 0", "Zero") << i18nc("@item the number 1", "One") << i18nc("@item the number 2", "Two") << i18nc("@item the number 3", "Three") << i18nc("@item the number 4", "Four") << i18nc("@item the number 5", "Five") << i18nc("@item the number 6", "Six") << i18nc("@item the number 7", "Seven") << i18nc("@item the number 8", "Eight") << i18nc("@item the number 9", "Nine") << i18nc("@item the number 10", "Ten") << i18nc("@item the number 11", "Eleven") << i18nc("@item the number 12", "Twelve") << i18nc("@item the number 13", "Thirteen") << i18nc("@item the number 14", "Fourteen") << i18nc("@item the number 15", "Fifteen") << i18nc("@item the number 16", "Sixteen") << i18nc("@item the number 17", "Seventeen") << i18nc("@item the number 18", "Eighteen") << i18nc("@item the number 19", "Nineteen");

  // Tens number names from twenty upwards
  m_tens << "" << "" << i18nc("@item the number 20", "Twenty") << i18nc("@item the number 30", "Thirty") << i18nc("@item the number 40", "Forty") << i18nc("@item the number 50", "Fifty") << i18nc("@item the number 60", "Sixty") << i18nc("@item the number 70", "Seventy") << i18nc("@item the number 80", "Eighty") << i18nc("@item the number 90", "Ninety");

  // Scale number names for use during recombination
  m_scaleNumbers << "" << i18nc("@item the number 1000", "Thousand") << i18nc("@item the number 1,000,000", "Million") << i18nc("@item the number 1,000,000,000", "Billion");
}

// Converts a three-digit group into English words
QString MyMoneyMoneyToWordsConverter::convertTreeDigitGroup(int threeDigitNumber)
{
  // Initialise the return text
  QString groupText;
  QString hundredsText;
  QString tensText;

  // Determine the hundreds and the remainder
  int hundreds = threeDigitNumber / 100;
  int tensUnits = threeDigitNumber % 100;

  // Hundreds rules
  if (hundreds != 0)
    hundredsText = i18nc("@item %1 = hundred value digit", "%1 Hundred", m_smallNumbers[hundreds]);

  // Determine the tens and units
  int tens = tensUnits / 10;
  int units = tensUnits % 10;

  // Tens rules
  if (tens >= 2) {
    if (units != 0) {
      // This comes after the tens text if the unit is not 0
      tensText = i18nc("@item %1 = tens text, %2 = ones text", "%1 %2", m_tens[tens], m_smallNumbers[units]);
    } else
      tensText = m_tens[tens];
  } else if (tensUnits != 0)
    tensText = m_smallNumbers[tensUnits];

  // Combine all parts
  if (tensUnits != 0) {
    if (hundreds != 0)
      groupText = i18nc("@item %1 = hundreds text, %2 = tens and ones text", "%1 and %2", hundredsText, tensText);
    else
      groupText = tensText;
  } else
    groupText = hundredsText;

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
      QString prefix = i18nc("@item %1 = numeral <1000, %2 = scale", "%1 %2", groupText[i], m_scaleNumbers[i]);
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
    return i18nc("@label The first argument is the amount in words, the second is the denominator of the fractional part",
                 "%1 and No/%2", combined, denom);
}
