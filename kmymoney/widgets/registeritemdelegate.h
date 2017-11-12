/***************************************************************************
                             registeritemdelegate.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REGISTERITEMDELEGATE_H
#define REGISTERITEMDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QPainter;
class QModelIndex;
class QStyleOptionViewItem;

namespace KMyMoneyRegister
{
  class Register;
  class RegisterItemDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
    Q_DISABLE_COPY(RegisterItemDelegate)

  public:
    explicit RegisterItemDelegate(Register *parent);
    ~RegisterItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private:
    Register *m_register;
  };

} // namespace

#endif
