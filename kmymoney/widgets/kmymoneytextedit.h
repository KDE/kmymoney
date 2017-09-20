/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KMYMONEYTEXTEDIT_H
#define KMYMONEYTEXTEDIT_H

#include <KTextEdit>
#include "kmm_widgets_export.h"

class KMyMoneyTextEditHighlighter;


/**
 * @brief KTextEdit with restricted character set and length
 *
 * Used to set constraints on input. It allows to set readOnly property by
 * slots as well (not possible with KTextEdit).
 */
class KMM_WIDGETS_EXPORT KMyMoneyTextEdit : public KTextEdit
{
  Q_OBJECT

  /**
   * @brief Maximal number of characters allowed
   */
  Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength)

  /**
   * @brief Maximal number of characters allowed per line
   */
  Q_PROPERTY(int maxLineLength READ maxLineLength WRITE setMaxLineLength)

  /**
   * @brief Maximal number of lines
   */
  Q_PROPERTY(int maxLines READ maxLines WRITE setMaxLines)

  /**
   * @brief List of all allowed chars
   */
  Q_PROPERTY(QString allowedChars READ allowedChars WRITE setAllowedChars)

  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly);

public:
  KMyMoneyTextEdit(QWidget* parent = 0);

  int maxLength() const;
  int maxLineLength() const;
  int maxLines() const;
  QString allowedChars() const;
  bool isValid() const;

public Q_SLOTS:
  void setMaxLength(const int& maxLength);
  void setMaxLineLength(const int& maxLineLength);
  void setMaxLines(const int& maxLines);
  void setAllowedChars(const QString& allowedChars);

  /** @brief Slot to set this text edit read only */
  void setReadOnly(bool);

protected:
  virtual void keyReleaseEvent(QKeyEvent* e);
  virtual void keyPressEvent(QKeyEvent* e);

private:
  bool isEventAllowed(QKeyEvent* e) const;
  int m_maxLength;
  int m_maxLineLength;
  int m_maxLines;
  QString m_allowedChars;
  KMyMoneyTextEditHighlighter* m_highligther;

};

#endif // KMYMONEYTEXTEDIT_H
