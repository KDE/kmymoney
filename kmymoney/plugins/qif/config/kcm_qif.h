/***************************************************************************
 *   SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com       *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***************************************************************************/
#ifndef KCM_QIF_H
#define KCM_QIF_H

#include <KCModule>

class KCMqif : public KCModule
{
public:
  KCMqif(QWidget* parent, const QVariantList& args);
};

#endif // KCM_QIF_H

