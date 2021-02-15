/*
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#ifndef CHARVALIDATOR_H
#define CHARVALIDATOR_H

#include <QValidator>

class charValidator : public QValidator
{
  Q_OBJECT

public:
  explicit charValidator(QObject* parent = 0, const QString& characters = QString());
  QValidator::State validate(QString& , int&) const final override;

  void setAllowedCharacters(const QString&);

private:
  QString m_allowedCharacters;
};

#endif // CHARVALIDATOR_H
