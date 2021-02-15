/*
 * SPDX-FileCopyrightText: 2013-2014 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneytextedit.h"
#include "kmymoneytextedithighlighter.h"

#include <QKeyEvent>

class KMyMoneyTextEditPrivate
{
  Q_DISABLE_COPY(KMyMoneyTextEditPrivate)
  Q_DECLARE_PUBLIC(KMyMoneyTextEdit)

public:
  explicit KMyMoneyTextEditPrivate(KMyMoneyTextEdit *qq) :
    q_ptr(qq),
    m_maxLength(-1),
    m_maxLineLength(-1),
    m_maxLines(-1),
    m_allowedChars(QString(QString())),
    m_highligther(0)
  {
  }

  bool isEventAllowed(QKeyEvent* e) const
  {
    Q_Q(const KMyMoneyTextEdit);
    const QString text = e->text();
    if (!text.isEmpty()) {
      if (text.at(0).isPrint()) {
        if (!m_allowedChars.contains(text))
          return false;

        // Do not check max lengths etc if something is replaced
        if (q->textCursor().hasSelection())
          return true;

        const QString plainText = q->toPlainText();

        if (m_maxLength != -1 && plainText.length() >= m_maxLength)
          return false;
        if (m_maxLineLength != -1 && q->textCursor().block().length() - 1 >= m_maxLineLength)
          return false;
      } else if (m_maxLines != -1 && text.at(0) == '\r' && q->toPlainText().count('\n') + 1 >= m_maxLines) {
        // Does this work on non-linux OSes as well?
        return false;
      }
    }
    return true;
  }

  KMyMoneyTextEdit *q_ptr;
  int m_maxLength;
  int m_maxLineLength;
  int m_maxLines;
  QString m_allowedChars;
  KMyMoneyTextEditHighlighter* m_highligther;
};


void KMyMoneyTextEdit::setReadOnly(bool readOnly)
{
  KTextEdit::setReadOnly(readOnly);
}

/* KMyMoneyTextEdit */

KMyMoneyTextEdit::KMyMoneyTextEdit(QWidget* parent) :
  KTextEdit(parent),
  d_ptr(new KMyMoneyTextEditPrivate(this))
{
  Q_D(KMyMoneyTextEdit);
  setWordWrapMode(QTextOption::ManualWrap);
  d->m_highligther = new KMyMoneyTextEditHighlighter(this);
}

KMyMoneyTextEdit::~KMyMoneyTextEdit()
{
  Q_D(KMyMoneyTextEdit);
  delete d;
}

bool KMyMoneyTextEdit::isValid() const
{
  Q_D(const KMyMoneyTextEdit);
  const QString text = toPlainText();

  if (d->m_maxLength != -1 && text.length() >= d->m_maxLength)
    return false;

  const QStringList lines = text.split('\n');

  if (d->m_maxLines != -1 && lines.count() >= d->m_maxLines) {
    return false;
  }

  if (d->m_maxLineLength != -1) {
    foreach (QString line, lines) {
      if (line.length() > d->m_maxLineLength)
        return false;
    }
  }

  const int length = text.length();
  for (auto i = 0; i < length; ++i) {
    if (!d->m_allowedChars.contains(text.at(i)))
      return false;
  }
  return true;
}

void KMyMoneyTextEdit::keyReleaseEvent(QKeyEvent* e)
{
  Q_D(KMyMoneyTextEdit);
  if (d->isEventAllowed(e))
    KTextEdit::keyReleaseEvent(e);
}

void KMyMoneyTextEdit::keyPressEvent(QKeyEvent* e)
{
  Q_D(KMyMoneyTextEdit);
  if (d->isEventAllowed(e))
    KTextEdit::keyPressEvent(e);
}

int KMyMoneyTextEdit::maxLength() const
{
  Q_D(const KMyMoneyTextEdit);
  return d->m_maxLength;
}

void KMyMoneyTextEdit::setMaxLength(const int& maxLength)
{
  Q_D(KMyMoneyTextEdit);
  d->m_maxLength = maxLength;
  d->m_highligther->setMaxLength(d->m_maxLength);
}

int KMyMoneyTextEdit::maxLineLength() const
{
  Q_D(const KMyMoneyTextEdit);
  return d->m_maxLineLength;
}

void KMyMoneyTextEdit::setMaxLineLength(const int& maxLineLength)
{
  Q_D(KMyMoneyTextEdit);
  d->m_maxLineLength = maxLineLength;
  d->m_highligther->setMaxLineLength(maxLineLength);
}

int KMyMoneyTextEdit::maxLines() const
{
  Q_D(const KMyMoneyTextEdit);
  return d->m_maxLines;
}

void KMyMoneyTextEdit::setMaxLines(const int& maxLines)
{
  Q_D(KMyMoneyTextEdit);
  d->m_maxLines = maxLines;
  d->m_highligther->setMaxLines(maxLines);
}

QString KMyMoneyTextEdit::allowedChars() const
{
  Q_D(const KMyMoneyTextEdit);
  return d->m_allowedChars;
}

void KMyMoneyTextEdit::setAllowedChars(const QString& allowedChars)
{
  Q_D(KMyMoneyTextEdit);
  d->m_allowedChars = allowedChars;
  d->m_highligther->setAllowedChars(allowedChars);
}
