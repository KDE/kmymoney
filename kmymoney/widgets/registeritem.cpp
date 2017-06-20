/***************************************************************************
                          registeritem.cpp  -  description
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

#include "registeritem.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;

QDate RegisterItem::nullDate;
QString RegisterItem::nullString;
MyMoneyMoney RegisterItem::nullValue;

RegisterItem::RegisterItem() :
    m_parent(0),
    m_prev(0),
    m_next(0),
    m_alternate(false),
    m_needResize(false),
    m_visible(false)
{
  init();
}

RegisterItem::RegisterItem(Register* parent) :
    m_parent(parent),
    m_prev(0),
    m_next(0),
    m_alternate(false),
    m_needResize(false),
    m_visible(false)
{
  init();
  parent->addItem(this);
}

void RegisterItem::init()
{
  m_startRow = 0;
  m_rowsRegister = 1;
  m_rowsForm = 1;
  m_visible = true;
}

RegisterItem::~RegisterItem()
{
  m_parent->removeItem(this);
}

void RegisterItem::setParent(Register* parent)
{
  m_parent = parent;
}

void RegisterItem::setNumRowsRegister(int rows)
{
  if (rows != m_rowsRegister) {
    m_rowsRegister = rows;
    if (m_parent)
      m_parent->forceUpdateLists();
  }
}

bool RegisterItem::markVisible(bool visible)
{
  if (m_visible == visible)
    return false;
  m_visible = visible;
  return true;
}

void RegisterItem::setVisible(bool visible)
{
  if (markVisible(visible) && m_parent) {
    int numRows = m_parent->rowCount();
    if (visible) {
      for (int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        if (numRows > i) {
          m_parent->showRow(i);
          m_parent->setRowHeight(i, rowHeightHint());
        }
      }
    } else {
      for (int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        if (numRows > i) {
          m_parent->hideRow(i);
        }
      }
    }
  }
}

int RegisterItem::rowHeightHint() const
{
  if (!m_visible)
    return 0;

  if (m_parent) {
    return m_parent->rowHeightHint();
  }

  QFontMetrics fm(KMyMoneyGlobalSettings::listCellFont());
  return fm.lineSpacing() + 6;
}
