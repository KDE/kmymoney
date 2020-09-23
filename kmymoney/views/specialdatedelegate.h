/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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

