/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERVIEWPAGE_H
#define LEDGERVIEWPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace eMenu { enum class Menu; }

class MyMoneyAccount;
class SelectedObjects;

class LedgerViewPage : public QWidget
{
  Q_OBJECT
public:
  explicit LedgerViewPage(QWidget* parent = 0, const QString& configGroupName = QString());
  virtual ~LedgerViewPage();

  virtual void setAccount(const MyMoneyAccount& id);
  virtual QString accountId() const;

  /**
   * This method is used to modify the visibility of the
   * empty entry at the end of the ledger. The default
   * for the parameter @a show is @c true.
   */
  void setShowEntryForNewTransaction(bool show = true);

  void selectTransaction(const QString& id);

  const SelectedObjects& selections() const;

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
  void showTransactionForm(bool show);
  void splitterChanged(int pos, int index);
  void slotSettingsChanged();

protected Q_SLOTS:
  void startEdit();
  void finishEdit();
  void keepSelection();
  void reloadFilter();
  void slotRequestSelectionChanged(const SelectedObjects& selections) const;

Q_SIGNALS:
  void requestSelectionChanged(const SelectedObjects& selection) const;
  void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos) const;

  void transactionSelected(const QModelIndex& idx);
  void aboutToStartEdit();
  void aboutToFinishEdit();

private:
  class Private;
  Private * const d;
};

#endif // LEDGERVIEWPAGE_H

