/***************************************************************************
                          registeritem_p.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
