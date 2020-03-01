/*
 * Copyright 2004-2020  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYACCOUNTCOMBO_H
#define KMYMONEYACCOUNTCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsproxymodel.h"
#include "onlinebankingaccountsfilterproxymodel.h"

/**
 * @brief OnlineBankingAccountFilterProxyModel showing only the name column
 *
 * Is equivalent to AccountNamesFilterProxyModel using OnlineBankingAccountFilterProxyModel as base.
 */
typedef AccountNamesFilterProxyModelTpl<OnlineBankingAccountsFilterProxyModel> OnlineBankingAccountNamesFilterProxyModel;


/**
  * @brief A general account selection widget based on a KComboBox
  *
  * This widget allows to select an account from the provided set of accounts. This
  * set is passed as model in the constructor or via setModel(). In case the widget
  * is configured to be editable via setEditable() the combo box contains a lineedit
  * widget. This lineedit provides auto completion.
  *
  * In addition to the KComboBox which supports a list view popup, this widget
  * provides a tree view popup to show the account hierarchy.
  *
  * @author Cristian Onet
  */
class KMyMoneyAccountCombo : public KComboBox
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountCombo)

public:
  explicit KMyMoneyAccountCombo(QSortFilterProxyModel *model, QWidget* parent = nullptr);
  explicit KMyMoneyAccountCombo(QWidget* parent = nullptr);
  ~KMyMoneyAccountCombo();

  const QString& getSelected() const;

  void setModel(QSortFilterProxyModel *model);

  /**
   * Overridden to get specific behavior
   */
  void setEditable(bool isEditable);

  bool eventFilter(QObject* o, QEvent* e) override;

public Q_SLOTS:
  void expandAll();
  void collapseAll();
  void showPopup() override;
  void hidePopup() override;
  void setSelected(const QString& id);
  void setSplitActionVisible(bool show);

protected:
  void wheelEvent(QWheelEvent *ev) override;

protected Q_SLOTS:
  void activated();
  void makeCompletion(const QString& txt) override;
  void selectItem(const QModelIndex& index);

Q_SIGNALS:
  void accountSelected(const QString&);
  void splitDialogRequest();

private:
  void init();

private:
  class Private;
  QScopedPointer<Private> const d;
};

class QAbstractButton;
class KMyMoneyAccountComboSplitHelperPrivate;
class KMyMoneyAccountComboSplitHelper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountComboSplitHelper)

public:
  explicit KMyMoneyAccountComboSplitHelper(QComboBox* accountCombo, QAbstractItemModel *model);
  ~KMyMoneyAccountComboSplitHelper();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
  void splitCountChanged();
  void modelDestroyed();

Q_SIGNALS:
  void accountComboEnabled(bool enabled);

private:
  Q_DECLARE_PRIVATE(KMyMoneyAccountComboSplitHelper);
  QScopedPointer<KMyMoneyAccountComboSplitHelperPrivate>  d_ptr;
};

#endif
// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
