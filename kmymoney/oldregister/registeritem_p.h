/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef REGISTERITEM_P_H
#define REGISTERITEM_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class QDate;
class QString;

namespace KMyMoneyRegister { class Register; }
namespace KMyMoneyRegister { class RegisterItem; }

using namespace KMyMoneyRegister;

namespace KMyMoneyRegister
{
  class RegisterItemPrivate
  {
  public:
    RegisterItemPrivate() :
      m_parent(nullptr),
      m_prev(nullptr),
      m_next(nullptr),
      m_startRow(0),
      m_rowsRegister(1),
      m_rowsForm(1),
      m_alternate(false),
      m_needResize(false),
      m_visible(true)
    {
    }

    virtual ~RegisterItemPrivate()
    {
    }

    Register*                m_parent;
    RegisterItem*            m_prev;
    RegisterItem*            m_next;
    int                      m_startRow;
    int                      m_rowsRegister;
    int                      m_rowsForm;
    bool                     m_alternate;
    bool                     m_needResize;
    bool                     m_visible;

    static QDate             nullDate;
    static QString           nullString;
    static MyMoneyMoney      nullValue;

  };
}

#endif
