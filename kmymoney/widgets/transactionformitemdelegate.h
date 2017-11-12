/***************************************************************************
                             transactionformitemdelegate.h
                             ----------
    begin                : Sun May 14 2006
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

#ifndef TRANSACTIONFORMITEMDELEGATE_H
#define TRANSACTIONFORMITEMDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyTransactionForm
{
  class TransactionForm;
  class TransactionFormItemDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
    Q_DISABLE_COPY(TransactionFormItemDelegate)

  public:
    explicit TransactionFormItemDelegate(TransactionForm *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  private:
    TransactionForm *m_transactionForm;
  };
} // namespace

#endif
