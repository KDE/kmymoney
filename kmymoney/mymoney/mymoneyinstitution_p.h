/*
 * Copyright 2000-2001  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYMONEYINSTITUTION_P_H
#define MYMONEYINSTITUTION_P_H

#include "mymoneyinstitution.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QHash>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"

class MyMoneyInstitutionPrivate : public MyMoneyObjectPrivate
{
public:
  /**
    * This member variable keeps the name of the institution
    */
  QString m_name;

  /**
    * This member variable keeps the city of the institution
    */
  QString m_town;

  /**
    * This member variable keeps the street of the institution
    */
  QString m_street;

  /**
    * This member variable keeps the zip-code of the institution
    */
  QString m_postcode;

  /**
    * This member variable keeps the telephone number of the institution
    */
  QString m_telephone;

  /**
    * This member variable keeps the name of the representative of
    * the institution
    */
  QString m_manager;

  /**
    * This member variable keeps the sort code of the institution.
    * FIXME: I have no idea
    * what it is good for. I keep it because it was in the old engine.
    */
  QString m_sortcode;

  /**
    * This member variable keeps the sorted list of the account ids
    * available at this institution
    */
  QStringList m_accountList;
};

#endif
