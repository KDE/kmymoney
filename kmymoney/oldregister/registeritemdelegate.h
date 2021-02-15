/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const final override;

  private:
    Register *m_register;
  };

} // namespace

#endif
