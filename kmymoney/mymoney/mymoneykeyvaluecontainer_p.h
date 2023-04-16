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
     * Returns a string representing of a boolean value
     *
     * @param value the boolean value
     * @retval "yes" for @c true
     * @retval "no" for @c false
     */
    QString toString(bool value) const;

    /**
     * Returns a string representing an int value
     *
     * @param value the integer value
     * @returns value as QString
     */
    QString toString(int value) const;

    /**
      * This member variable represents the container of key/value pairs.
      */
    QMap<QString, QString>  m_kvp;
};
#endif
