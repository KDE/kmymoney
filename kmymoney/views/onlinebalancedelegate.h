/*
 * SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ONLINEBALANCEDELEGATE_H
#define ONLINEBALANCEDELEGATE_H

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

class OnlineBalanceDelegate : public KMMStyledItemDelegate
{
  Q_OBJECT
public:
  explicit OnlineBalanceDelegate(LedgerView* parent = 0);
  virtual ~OnlineBalanceDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

  void setOnlineBalance(const QDate& date, const MyMoneyMoney& amount, int fraction = 0);

  /**
   * Which data (@a role) shall be displayed in the detail column
   * when only a single line is shown. The default is the payee.
   */
  void setSingleLineRole(eMyMoney::Model::Roles role);

  static void setErroneousColor(const QColor& color);
  static void setImportedColor(const QColor& color);

  static QColor erroneousColor();

protected:
  bool eventFilter(QObject* o, QEvent* event) final override;

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&) const;

private:
  class Private;
  Private * const d;

  static QColor m_erroneousColor;
  static QColor m_importedColor;
  static QColor m_separatorColor;
};

#endif // ONLINEBALANCEDELEGATE_H

