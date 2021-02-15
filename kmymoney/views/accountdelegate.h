/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ACCOUNTDELEGATE_H
#define ACCOUNTDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "mymoneyenums.h"

class MyMoneyMoney;

class AccountDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit AccountDelegate(QObject* parent = 0);
  virtual ~AccountDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

private:
  class Private;
  Private * const d;
};

#endif // ACCOUNTDELEGATE_H

