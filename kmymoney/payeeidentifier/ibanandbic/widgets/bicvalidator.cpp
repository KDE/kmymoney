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

#include "bicvalidator.h"

#include <KLocalizedString>

#include "payeeidentifier/ibanbic/ibanbic.h"
#include "widgetenums.h"

bicValidator::bicValidator(QObject* parent)
    : QValidator(parent)
{
}

QValidator::State bicValidator::validate(QString &string, int&) const
{
  for (int i = 0; i < qMin(string.length(), 6); ++i) {
    if (!string.at(i).isLetter())
      return Invalid;
    if (string.at(i).isLower())
      string[i] = string.at(i).toUpper();
  }

  for (int i = 6; i < string.length(); ++i) {
    if (!string.at(i).isLetterOrNumber())
      return Invalid;
    if (string.at(i).isLower())
      string[i] = string.at(i).toUpper();
  }

  if (string.length() > 11)
    return Invalid;
  else if (string.length() == 8 || string.length() == 11) {
    return Acceptable;
  }
  return Intermediate;
}

QPair< eWidgets::ValidationFeedback::MessageType, QString > bicValidator::validateWithMessage(const QString& string)
{
  // Do not show an error message if no BIC is given.
  if (string.length() != 8 && string.length() != 11)
    return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Error, i18n("A valid BIC is 8 or 11 characters long."));

  if (payeeIdentifiers::ibanBic::isBicAllocated(string) == payeeIdentifiers::ibanBic::bicNotAllocated)
    return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Error, i18n("The given BIC is not assigned to any credit institute."));

  return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::None, QString());

}
