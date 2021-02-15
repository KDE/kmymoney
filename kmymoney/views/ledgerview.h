/*
    SPDX-FileCopyrightText: 2015 Thomas Baumgart <Thomas Baumgart <tbaumgart@kde.org>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
   * This method is used to modify the visibility of the
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
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) final override;
  void mousePressEvent(QMouseEvent* event) final override;
  void mouseMoveEvent(QMouseEvent* event) final override;
  void mouseDoubleClickEvent(QMouseEvent* event) final override;
  void wheelEvent(QWheelEvent *event) final override;
  void moveEvent(QMoveEvent *event) final override;
  void resizeEvent(QResizeEvent* event) final override;
  void paintEvent(QPaintEvent* event) final override;
  int sizeHintForRow(int row) const final override;

protected Q_SLOTS:
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) final override;
  void rowsInserted(const QModelIndex& index, int start, int end) final override;
  void rowsAboutToBeRemoved(const QModelIndex& index, int start, int end) final override;
  void currentChanged(const QModelIndex &current, const QModelIndex &previous) final override;

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
  void recalculateBalances() final override {}
};
#endif // LEDGERVIEW_H

