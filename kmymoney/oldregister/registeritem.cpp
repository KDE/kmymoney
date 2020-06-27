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

#include "registeritem.h"
#include "registeritem_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "kmymoneysettings.h"
#include "mymoneyobject.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"
#include "widgetenums.h"

using namespace KMyMoneyRegister;

QDate RegisterItemPrivate::nullDate;
QString RegisterItemPrivate::nullString;
MyMoneyMoney RegisterItemPrivate::nullValue;

RegisterItem::RegisterItem(Register* parent) :
  d_ptr(new RegisterItemPrivate)
{
  Q_D(RegisterItem);
  d->m_parent = parent;
  parent->addItem(this);
}

RegisterItem::RegisterItem(RegisterItemPrivate &dd, Register* parent) :
  d_ptr(&dd)
{
  Q_D(RegisterItem);
  d->m_parent = parent;
  parent->addItem(this);
}

RegisterItem::~RegisterItem()
{
  Q_D(RegisterItem);
  d->m_parent->removeItem(this);
  delete d;
}

bool RegisterItem::isSelected() const
{
  return false;
}

void RegisterItem::setSelected(bool /* selected*/)
{
}

bool RegisterItem::hasFocus() const {
  return false;
}

bool RegisterItem::hasEditorOpen() const
{
  return false;
}

void RegisterItem::setFocus(bool /*focus*/, bool updateLens)
{
  Q_UNUSED(updateLens);
}

QDate RegisterItem::sortPostDate() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullDate;
}

QDate RegisterItem::sortEntryDate() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullDate;
}

const QString& RegisterItem::sortPayee() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullString;
}

MyMoneyMoney RegisterItem::sortValue() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullValue;
}

QString RegisterItem::sortNumber() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullString;
}

const QString& RegisterItem::sortEntryOrder() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullString;
}

eWidgets::eRegister::CashFlowDirection RegisterItem::sortType() const
{
  return eWidgets::eRegister::CashFlowDirection::Deposit;
}

const QString& RegisterItem::sortCategory() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullString;
}

eMyMoney::Split::State RegisterItem::sortReconcileState() const
{
  return eMyMoney::Split::State::MaxReconcileState;
}

const QString RegisterItem::sortSecurity() const
{
  return KMyMoneyRegister::RegisterItemPrivate::nullString;
}

void RegisterItem::setStartRow(int row)
{
  Q_D(RegisterItem);
  d->m_startRow = row;
}

int RegisterItem::startRow() const
{
  Q_D(const RegisterItem);
  return d->m_startRow;
}

QString RegisterItem::id() const
{
  return QString();
}

void RegisterItem::setParent(Register* parent)
{
  Q_D(RegisterItem);
  d->m_parent = parent;
}

Register* RegisterItem::getParent() const
{
  Q_D(const RegisterItem);
  return d->m_parent;
}

void RegisterItem::setNeedResize()
{
  Q_D(RegisterItem);
  d->m_needResize = true;
}

bool RegisterItem::isVisible() const
{
  Q_D(const RegisterItem);
  return d->m_visible;
}

void RegisterItem::setNumRowsRegister(int rows)
{
  Q_D(RegisterItem);
  if (rows != d->m_rowsRegister) {
    d->m_rowsRegister = rows;
    if (d->m_parent)
      d->m_parent->forceUpdateLists();
  }
}

void RegisterItem::setNumRowsForm(int rows)
{
  Q_D(RegisterItem);
  d->m_rowsForm = rows;
}

int RegisterItem::numRowsRegister() const
{
  Q_D(const RegisterItem);
  return d->m_rowsRegister;
}

int RegisterItem::numRowsForm() const
{
  Q_D(const RegisterItem);
  return d->m_rowsForm;
}

int RegisterItem::numColsForm() const
{
  return 1;
}

void RegisterItem::setAlternate(bool alternate)
{
  Q_D(RegisterItem);
  d->m_alternate = alternate;
}

bool RegisterItem::markVisible(bool visible)
{
  Q_D(RegisterItem);
  if (d->m_visible == visible)
    return false;
  d->m_visible = visible;
  return true;
}

void  RegisterItem::setNextItem(RegisterItem* p)
{
  Q_D(RegisterItem);
  d->m_next = p;
}

void  RegisterItem::setPrevItem(RegisterItem* p)
{
  Q_D(RegisterItem);
  d->m_prev = p;
}

RegisterItem*  RegisterItem::nextItem() const
{
  Q_D(const RegisterItem);
  return d->m_next;
}

RegisterItem*  RegisterItem::prevItem() const
{
  Q_D(const RegisterItem);
  return d->m_prev;
}

bool RegisterItem::maybeTip(const QPoint& /* relpos */, int /* row */, int /* col */, QRect& /* r */, QString& /* msg */)
{
  return false;
}

void RegisterItem::setVisible(bool visible)
{
  Q_D(RegisterItem);
  if (markVisible(visible) && d->m_parent) {
    int numRows = d->m_parent->rowCount();
    if (visible) {
      for (int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        if (numRows > i) {
          d->m_parent->showRow(i);
          d->m_parent->setRowHeight(i, rowHeightHint());
        }
      }
    } else {
      for (int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        if (numRows > i) {
          d->m_parent->hideRow(i);
        }
      }
    }
  }
}

int RegisterItem::rowHeightHint() const
{
  Q_D(const RegisterItem);
  if (!d->m_visible)
    return 0;

  if (d->m_parent) {
    return d->m_parent->rowHeightHint();
  }

  QFontMetrics fm(KMyMoneySettings::listCellFontEx());
  return fm.lineSpacing() + 6;
}
