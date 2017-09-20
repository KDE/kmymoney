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

#include "kmymoneytextedit.h"
#include "kmymoneytextedit_p.h"
#include <QKeyEvent>
#include <QDebug>

/* The Higligther */

KMyMoneyTextEditHighlighter::KMyMoneyTextEditHighlighter(QTextEdit * parent)
    : Highlighter(parent),
    m_allowedChars(QString("")),
    m_maxLines(-1),
    m_maxLineLength(-1),
    m_maxLength(-1)
{

}

void KMyMoneyTextEditHighlighter::setAllowedChars(const QString& chars)
{
  m_allowedChars = chars;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLength(const int& length)
{
  m_maxLength = length;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLines(const int& lines)
{
  m_maxLines = lines;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLineLength(const int& length)
{
  m_maxLineLength = length;
  rehighlight();
}

void KMyMoneyTextEdit::setReadOnly(bool readOnly)
{
  KTextEdit::setReadOnly(readOnly);
}

void KMyMoneyTextEditHighlighter::highlightBlock(const QString& text)
{
  // Spell checker first
  Highlighter::highlightBlock(text);

  QTextCharFormat invalidFormat;
  invalidFormat.setFontItalic(true);
  invalidFormat.setForeground(Qt::red);
  invalidFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

  // Check used characters
  const int length = text.length();
  for (int i = 0; i < length; ++i) {
    if (!m_allowedChars.contains(text.at(i))) {
      setFormat(i, 1, invalidFormat);
    }
  }

  if (m_maxLines != -1) {
    //! @todo Is using QTextBlock::blockNumber() as line number dangerous?
    if (currentBlock().blockNumber() >= m_maxLines) {
      setFormat(0, length, invalidFormat);
      return;
    }
  }

  if (m_maxLength != -1) {
    const int blockPosition = currentBlock().position();
    if (m_maxLength < (length + blockPosition)) {
      setFormat(m_maxLength, length - m_maxLength - blockPosition, invalidFormat);
      return;
    }
  }

  if (m_maxLineLength != -1 && length >= m_maxLineLength) {
    setFormat(m_maxLineLength, length - m_maxLineLength, invalidFormat);
    return;
  }
}

/* KMyMoneyTextEdit */

KMyMoneyTextEdit::KMyMoneyTextEdit(QWidget* parent)
    : KTextEdit(parent),
    m_maxLength(-1),
    m_maxLineLength(-1),
    m_maxLines(-1),
    m_allowedChars(QString("")),
    m_highligther(0)
{
  setWordWrapMode(QTextOption::ManualWrap);
  m_highligther = new KMyMoneyTextEditHighlighter(this);
}

bool KMyMoneyTextEdit::isEventAllowed(QKeyEvent* e) const
{
  const QString text = e->text();
  if (!text.isEmpty()) {
    if (text.at(0).isPrint()) {
      if (!m_allowedChars.contains(text))
        return false;

      // Do not check max lengths etc if something is replaced
      if (textCursor().hasSelection())
        return true;

      const QString plainText = toPlainText();

      if (m_maxLength != -1 && plainText.length() >= m_maxLength)
        return false;
      if (m_maxLineLength != -1 && textCursor().block().length() - 1 >= m_maxLineLength)
        return false;
    } else if (m_maxLines != -1 && text.at(0) == '\r' && toPlainText().count('\n') + 1 >= m_maxLines) {
      // Does this work on non-linux OSes as well?
      return false;
    }
  }
  return true;
}

bool KMyMoneyTextEdit::isValid() const
{
  const QString text = toPlainText();

  if (m_maxLength != -1 && text.length() >= m_maxLength)
    return false;

  const QStringList lines = text.split('\n');

  if (m_maxLines != -1 && lines.count() >= m_maxLines) {
    return false;
  }

  if (m_maxLineLength != -1) {
    foreach (QString line, lines) {
      if (line.length() > m_maxLineLength)
        return false;
    }
  }

  const int length = text.length();
  for (int i = 0; i < length; ++i) {
    if (!m_allowedChars.contains(text.at(i)))
      return false;
  }
  return true;
}

void KMyMoneyTextEdit::keyReleaseEvent(QKeyEvent* e)
{
  if (isEventAllowed(e))
    KTextEdit::keyReleaseEvent(e);
}

void KMyMoneyTextEdit::keyPressEvent(QKeyEvent* e)
{
  if (isEventAllowed(e))
    KTextEdit::keyPressEvent(e);
}

int KMyMoneyTextEdit::maxLength() const
{
  return m_maxLength;
}

void KMyMoneyTextEdit::setMaxLength(const int& maxLength)
{
  m_maxLength = maxLength;
  m_highligther->setMaxLength(m_maxLength);
}

int KMyMoneyTextEdit::maxLineLength() const
{
  return m_maxLineLength;
}

void KMyMoneyTextEdit::setMaxLineLength(const int& maxLineLength)
{
  m_maxLineLength = maxLineLength;
  m_highligther->setMaxLineLength(maxLineLength);
}

int KMyMoneyTextEdit::maxLines() const
{
  return m_maxLines;
}

void KMyMoneyTextEdit::setMaxLines(const int& maxLines)
{
  m_maxLines = maxLines;
  m_highligther->setMaxLines(maxLines);
}

QString KMyMoneyTextEdit::allowedChars() const
{
  return m_allowedChars;
}

void KMyMoneyTextEdit::setAllowedChars(const QString& allowedChars)
{
  m_allowedChars = allowedChars;
  m_highligther->setAllowedChars(allowedChars);
}
