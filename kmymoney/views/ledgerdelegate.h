/***************************************************************************
                          ledgerdelegate.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEDGERDELEGATE_H
#define LEDGERDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgermodel.h"
class NewTransactionEditor;
class LedgerView;

class LedgerSeperator
{
public:
  LedgerSeperator(LedgerRole::Roles role) : m_role(role) {}
  virtual ~LedgerSeperator() {}

  virtual bool rowHasSeperator(const QModelIndex& index) const = 0;
  virtual QString seperatorText(const QModelIndex& index) const = 0;

  static void setFirstFiscalDate(int firstMonth, int firstDay);
  static void setShowFiscalDate(bool show) { showFiscalDate = show; }
  static void setShowFancyDate(bool show) { showFancyDate = show; }

protected:
  inline QModelIndex nextIndex(const QModelIndex& index) const;

  LedgerRole::Roles         m_role;

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

  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editWidget, const QModelIndex& index) const;

  virtual void setSortRole(LedgerRole::Roles role);

  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * This method returns the row that currently has an editor
   * or -1 if no editor is open
   */
  virtual int editorRow() const;

  static void setErroneousColor(const QColor& color);
  static void setImportedColor(const QColor& color);

  static QColor erroneousColor();

protected:
  bool eventFilter(QObject* o, QEvent* event);

protected Q_SLOTS:
  void endEdit();

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&) const;

private:
  class Private;
  Private * const d;

  static QColor m_erroneousColor;
  static QColor m_importedColor;
  static QColor m_seperatorColor;
};

#endif // LEDGERDELEGATE_H

