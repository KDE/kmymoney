/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECIALDATEDELEGATE_H
#define SPECIALDATEDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

class QColor;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "kmmstyleditemdelegate.h"

class LedgerView;
class MyMoneyMoney;

class SpecialDateDelegate : public KMMStyledItemDelegate
{
  Q_OBJECT
public:
  explicit SpecialDateDelegate(LedgerView* parent = 0);
  virtual ~SpecialDateDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

  void setOnlineBalance(const QDate& date, const MyMoneyMoney& amount, int fraction = 0);

  /**
   * Which data (@a role) shall be displayed in the detail column
   * when only a single line is shown. The default is the payee.
   */
  void setSingleLineRole(eMyMoney::Model::Roles role);


protected:
  bool eventFilter(QObject* o, QEvent* event) final override;

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&) const;

private:
  class Private;
  Private * const d;

  static QColor m_separatorColor;
};

#endif // SPECIALDATEDELEGATE_H

