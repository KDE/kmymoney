/*
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYKEYVALUECONTAINER_P_H
#define MYMONEYKEYVALUECONTAINER_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyKeyValueContainerPrivate
{
public:
    /**
      * This member variable represents the container of key/value pairs.
      */
    QMap<QString, QString>  m_kvp;
};
#endif
