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

#ifndef KMYMONEYTEXTEDITHIGHLIGHTER_H
#define KMYMONEYTEXTEDITHIGHLIGHTER_H

#include <Sonnet/Highlighter>

class KMyMoneyTextEditHighlighter : public Sonnet::Highlighter
{
public:
  KMyMoneyTextEditHighlighter(QTextEdit* parent = 0);

  void setAllowedChars(const QString& chars);
  void setMaxLength(const int& length);
  void setMaxLines(const int& lines);
  void setMaxLineLength(const int& length);

protected:
  virtual void highlightBlock(const QString& text);

private:
  QString m_allowedChars;
  int m_maxLines;
  int m_maxLineLength;
  int m_maxLength;
};

#endif // KMYMONEYTEXTEDITHIGHLIGHTER_H
