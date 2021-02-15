/*
    SPDX-FileCopyrightText: 2015 Thomas Baumgart <Thomas Baumgart <tbaumgart@kde.org>>
    SPDX-License-Identifier: GPL-2.0-or-later
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

#include "ledgermodel.h"
#include "modelenums.h"

class LedgerView;
class MyMoneyMoney;

class LedgerSeparator
{
public:
  explicit LedgerSeparator(eLedgerModel::Role role) : m_role(role) {}
  virtual ~LedgerSeparator() {}

  virtual bool rowHasSeparator(const QModelIndex& index) const = 0;
  virtual QString separatorText(const QModelIndex& index) const = 0;

  virtual void adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const = 0;

  static void setFirstFiscalDate(int firstMonth, int firstDay);
  static void setShowFiscalDate(bool show) { showFiscalDate = show; }
  static void setShowFancyDate(bool show) { showFancyDate = show; }

protected:
  inline QModelIndex nextIndex(const QModelIndex& index) const;

  eLedgerModel::Role         m_role;

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

  virtual void setSortRole(eLedgerModel::Role role);

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

  /**
   * This method returns the row that currently has an editor
   * or -1 if no editor is open
   */
  virtual int editorRow() const;

  void setOnlineBalance(const QDate& date, const MyMoneyMoney& amount, int fraction = 0);

  static void setErroneousColor(const QColor& color);
  static void setImportedColor(const QColor& color);

  static QColor erroneousColor();

protected:
  bool eventFilter(QObject* o, QEvent* event) final override;

protected Q_SLOTS:
  void endEdit();

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&) const;

private:
  class Private;
  Private * const d;

  static QColor m_erroneousColor;
  static QColor m_importedColor;
  static QColor m_separatorColor;
};

#endif // LEDGERDELEGATE_H

