/*
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KWELCOMEPAGE_H
#define KWELCOMEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  * Generates welcome page
  *
  * @author Alvaro Soliverez
  *
  * @short Generates the welcome page
**/
class KWelcomePage
{
public:

    KWelcomePage();
    ~KWelcomePage();

    static const QString welcomePage();

protected:

    static bool isGroupHeader(const QString& item);
    static bool isGroupItem(const QString& item);
};

#endif
