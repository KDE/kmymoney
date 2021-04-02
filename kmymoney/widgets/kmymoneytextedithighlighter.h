/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYTEXTEDIT_H
#define KMYMONEYTEXTEDIT_H

#include <Sonnet/Highlighter>

class KMyMoneyTextEditHighlighterPrivate;
class KMyMoneyTextEditHighlighter : public Sonnet::Highlighter
{
    Q_DISABLE_COPY(KMyMoneyTextEditHighlighter)

public:
    explicit KMyMoneyTextEditHighlighter(QTextEdit* parent = nullptr); // krazy:exclude=qclasses
    ~KMyMoneyTextEditHighlighter();

    void setAllowedChars(const QString& chars);
    void setMaxLength(const int& length);
    void setMaxLines(const int& lines);
    void setMaxLineLength(const int& length);

protected:
    void highlightBlock(const QString& text) final override;

private:
    KMyMoneyTextEditHighlighterPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KMyMoneyTextEditHighlighter)
};

#endif // KMYMONEYTEXTEDIT_H
