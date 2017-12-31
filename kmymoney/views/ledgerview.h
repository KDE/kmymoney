/***************************************************************************
                          ledgerview.h
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

#ifndef LEDGERVIEW_H
#define LEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

class LedgerView : public QTableView
{
  Q_OBJECT
public:
  explicit LedgerView(QWidget* parent = 0);
  virtual ~LedgerView();

  virtual void setAccount(const MyMoneyAccount& acc);
  virtual QString accountId() const;

  /**
   * This method is used to modify the visibilty of the
   * empty entry at the end of the ledger. The default
   * for the parameter @a show is @c true.
   */
  void setShowEntryForNewTransaction(bool show = true);

  /**
   * Returns true if the sign of the values displayed has
   * been inverted depending on the account type.
   */
  bool showValuesInverted() const;

public Q_SLOTS:
  /**
   * This method scrolls the ledger so that the current item is visible
   */
  void ensureCurrentItemIsVisible();

  /**
   * Overridden for internal reasons. No change in base functionality
   */
  void edit(const QModelIndex& index) { QTableView::edit(index); }

protected:
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseDoubleClickEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent *event);
  virtual void moveEvent(QMoveEvent *event);
  virtual void resizeEvent(QResizeEvent* event);
  virtual void paintEvent(QPaintEvent* event);
  virtual int sizeHintForRow(int row) const;

protected Q_SLOTS:
  virtual void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);
  virtual void rowsInserted(const QModelIndex& index, int start, int end);
  virtual void rowsAboutToBeRemoved(const QModelIndex& index, int start, int end);
  virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);

  virtual void adjustDetailColumn(int newViewportWidth);
  virtual void adjustDetailColumn();

  virtual void recalculateBalances();

  virtual void accountChanged();

Q_SIGNALS:
  void transactionSelected(const QString& transactionSplitId);
  void aboutToStartEdit();
  void aboutToFinishEdit();

protected:
  class Private;
  Private * const d;
};

class SplitView : public LedgerView
{
  Q_OBJECT
public:
  explicit SplitView(QWidget* parent = 0);
  virtual ~SplitView();

protected Q_SLOTS:
  virtual void recalculateBalances() {}
};
#endif // LEDGERVIEW_H

