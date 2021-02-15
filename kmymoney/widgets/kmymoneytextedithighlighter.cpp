/*
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneytextedithighlighter.h"

/* The Higligther */

class KMyMoneyTextEditHighlighterPrivate
{
  Q_DISABLE_COPY(KMyMoneyTextEditHighlighterPrivate)

public:
  KMyMoneyTextEditHighlighterPrivate() :
    m_allowedChars(QString(QString())),
    m_maxLines(-1),
    m_maxLineLength(-1),
    m_maxLength(-1)
  {
  }

  QString m_allowedChars;
  int m_maxLines;
  int m_maxLineLength;
  int m_maxLength;
};


KMyMoneyTextEditHighlighter::KMyMoneyTextEditHighlighter(QTextEdit *parent) : // krazy:exclude=qclasses
  Highlighter(parent),
  d_ptr(new KMyMoneyTextEditHighlighterPrivate)
{
}

KMyMoneyTextEditHighlighter::~KMyMoneyTextEditHighlighter()
{
  Q_D(KMyMoneyTextEditHighlighter);
  delete d;
}

void KMyMoneyTextEditHighlighter::setAllowedChars(const QString& chars)
{
  Q_D(KMyMoneyTextEditHighlighter);
  d->m_allowedChars = chars;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLength(const int& length)
{
  Q_D(KMyMoneyTextEditHighlighter);
  d->m_maxLength = length;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLines(const int& lines)
{
  Q_D(KMyMoneyTextEditHighlighter);
  d->m_maxLines = lines;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::setMaxLineLength(const int& length)
{
  Q_D(KMyMoneyTextEditHighlighter);
  d->m_maxLineLength = length;
  rehighlight();
}

void KMyMoneyTextEditHighlighter::highlightBlock(const QString& text)
{
  Q_D(KMyMoneyTextEditHighlighter);
  // Spell checker first
  Highlighter::highlightBlock(text);

  QTextCharFormat invalidFormat;
  invalidFormat.setFontItalic(true);
  invalidFormat.setForeground(Qt::red);
  invalidFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

  // Check used characters
  const int length = text.length();
  for (auto i = 0; i < length; ++i) {
    if (!d->m_allowedChars.contains(text.at(i))) {
      setFormat(i, 1, invalidFormat);
    }
  }

  if (d->m_maxLines != -1) {
    //! @todo Is using QTextBlock::blockNumber() as line number dangerous?
    if (currentBlock().blockNumber() >= d->m_maxLines) {
      setFormat(0, length, invalidFormat);
      return;
    }
  }

  if (d->m_maxLength != -1) {
    const int blockPosition = currentBlock().position();
    if (d->m_maxLength < (length + blockPosition)) {
      setFormat(d->m_maxLength, length - d->m_maxLength - blockPosition, invalidFormat);
      return;
    }
  }

  if (d->m_maxLineLength != -1 && length >= d->m_maxLineLength) {
    setFormat(d->m_maxLineLength, length - d->m_maxLineLength, invalidFormat);
    return;
  }
}
