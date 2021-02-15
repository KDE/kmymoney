/***************************************************************************
 *   SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com                 *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***************************************************************************/
#ifndef NUMBERTOWORDS_H
#define NUMBERTOWORDS_H

#include <QStringList>

#include "mymoneymoney.h"

class MyMoneyMoneyToWordsConverter
{
private:
  QString convertTreeDigitGroup(int threeDigitNumber);

public:
  MyMoneyMoneyToWordsConverter();
  QString convert(const MyMoneyMoney & money, signed64 denom = 100);

private:
  QStringList m_smallNumbers;
  QStringList m_tens;
  QStringList m_scaleNumbers;
};

#endif // NUMBERTOWORDS_H
