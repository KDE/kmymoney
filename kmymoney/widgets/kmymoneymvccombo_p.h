/*
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2010-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYMVCCOMBO_P_H
#define KMYMONEYMVCCOMBO_P_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QCompleter;

class KMyMoneyMVCComboPrivate
{
public:
    KMyMoneyMVCComboPrivate() :
        m_canCreateObjects(false),
        m_inFocusOutEvent(false),
        m_completer(nullptr)
    {
    }

    /**
      * Flag to control object creation. Use
      * KMyMoneyMVCCombo::setSuppressObjectCreation()
      * to modify it's setting. Defaults to @a false.
      */
    bool                  m_canCreateObjects;

    /**
      * Flag to check whether a focusOutEvent processing is underway or not
      */
    bool                  m_inFocusOutEvent;

    QCompleter            *m_completer;
    /**
      * This is just a cache to be able to implement the old interface.
      */
    mutable QString                m_id;
};

#endif
