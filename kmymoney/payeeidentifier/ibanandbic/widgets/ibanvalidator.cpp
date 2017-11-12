/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian David <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ibanvalidator.h"

#include "../ibanbic.h"
#include <KLocalizedString>

#include "widgetenums.h"

ibanValidator::ibanValidator(QObject* parent)
    : QValidator(parent)
{

}

QValidator::State ibanValidator::validate(QString& string, int&) const
{
  // Check country code and set it uppercase
  if (string.length() >= 1) {
    if (!string.at(0).isLetter())
      return Invalid;
    if (string.at(0).isLower())
      string[0] = string.at(0).toUpper();
  }

  if (string.length() >= 2) {
    if (!string.at(1).isLetterOrNumber())
      return Invalid;
    if (string.at(1).isLower())
      string[1] = string.at(1).toUpper();
  }

  // Check rest of the iban
  int characterCount = qMin(string.length(), 2);
  for (int i = 2; i < string.length(); ++i) {
    if (string.at(i).isLetterOrNumber()) {
      ++characterCount;
    } else if (!string.at(i).isSpace()) {
      return Invalid;
    }
  }

  if (characterCount > 32)
    return Invalid;

  if (characterCount > 5) {
    return Acceptable;
  }

  return Intermediate;
}

QPair< eWidgets::ValidationFeedback::MessageType, QString > ibanValidator::validateWithMessage(const QString& string)
{
  // string.length() > 32 should not happen because all line edits should have this validator installed
  if (string.length() < 5)
    return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Error, i18n("This IBAN is too short."));

  if (!payeeIdentifiers::ibanBic::validateIbanChecksum(payeeIdentifiers::ibanBic::ibanToElectronic(string)))
    return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Warning, i18n("This IBAN is invalid."));

  return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::None, QString());
}

void ibanValidator::fixup(QString& string) const
{
  string = payeeIdentifiers::ibanBic::ibanToPaperformat(string);
}
