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

#ifndef LEDGERDELEGATE_H
#define LEDGERDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>
class QColor;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class LedgerView;
class MyMoneyMoney;

class LedgerSeparator
{
public:
  explicit LedgerSeparator(eMyMoney::Model::Roles role) : m_role(role) {}
  virtual ~LedgerSeparator() {}

  virtual bool rowHasSeparator(const QModelIndex& index) const = 0;
  virtual QString separatorText(const QModelIndex& index) const = 0;

  virtual void adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const = 0;

  static void setFirstFiscalDate(int firstMonth, int firstDay);
  static void setShowFiscalDate(bool show) { showFiscalDate = show; }
  static void setShowFancyDate(bool show) { showFancyDate = show; }

protected:
  inline QModelIndex nextIndex(const QModelIndex& index) const;

  eMyMoney::Model::Roles        m_role;

  static QDate firstFiscalDate;
  static bool  showFiscalDate;
  static bool  showFancyDate;
};



class LedgerDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit LedgerDelegate(LedgerView* parent = 0);
  virtual ~LedgerDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
  void setEditorData(QWidget* editWidget, const QModelIndex& index) const final override;

  virtual void setSortRole(eMyMoney::Model::Roles role);

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

  /**
   * This method returns the row that currently has an editor
   * or -1 if no editor is open
   */
  virtual int editorRow() const;

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

protected Q_SLOTS:
  void endEdit();

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&);

private:
  class Private;
  Private * const d;

  static QColor m_erroneousColor;
  static QColor m_importedColor;
  static QColor m_separatorColor;
};

#endif // LEDGERDELEGATE_H

