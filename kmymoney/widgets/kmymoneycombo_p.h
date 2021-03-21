/*
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCOMBO_P_H
#define KMYMONEYCOMBO_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMutex>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyCompletion;
class KMyMoneyLineEdit;

class KMyMoneyComboPrivate
{
public:
    KMyMoneyComboPrivate() :
        m_completion(nullptr),
        m_edit(nullptr),
        m_canCreateObjects(false),
        m_inFocusOutEvent(false)
    {
    }

    virtual ~KMyMoneyComboPrivate()
    {
    }

    /**
      * This member keeps a pointer to the object's completion object
      */
    KMyMoneyCompletion*    m_completion;

    /**
      * Use our own line edit to provide hint functionality
      */
    KMyMoneyLineEdit*      m_edit;

    /**
      * The currently selected item
      */
    QString                m_id;

    QTimer                 m_timer;
    QMutex                 m_focusMutex;
    /**
      * Flag to control object creation. Use setSuppressObjectCreation()
      * to modify it's setting. Defaults to @a false.
      */
    bool                   m_canCreateObjects;

    /**
      * Flag to check whether a focusOutEvent processing is underway or not
      */
    bool                   m_inFocusOutEvent;
};

#endif
