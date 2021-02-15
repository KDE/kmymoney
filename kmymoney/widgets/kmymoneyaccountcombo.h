/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  * A proxy model used to filter all the data from the core accounts model leaving
  * only the name of the accounts so this model can be used in the account
  * completion combo.
  *
  * It shows only the first column (account name) and makes top level items non-selectable.
  *
  * @see AccountsModel
  * @see AccountsFilterProxyModel
  *
  * @author Cristian Onet 2010
  * @author Christian David
  */

template <class baseProxyModel>
class AccountNamesFilterProxyModelTpl : public baseProxyModel
{
public:
  explicit AccountNamesFilterProxyModelTpl(QObject *parent = 0);

  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
};

/**
 * @brief "typedef" for AccountNamesFilterProxyModelTpl<AccountsFilterProxyModel>
 *
 * To create valid Qt moc data this class inherits the template and uses Q_OBJECT.
 *
 * @code
 * typedef AccountNamesFilterProxyModelTpl<AccountsFilterProxyModel> AccountNamesFilterProxyModel;
 * @endcode
 *
 * should work as well.
 */
class AccountNamesFilterProxyModel : public AccountNamesFilterProxyModelTpl<AccountsProxyModel>
{
  Q_OBJECT
public:
  explicit AccountNamesFilterProxyModel(QObject* parent = 0)
      : AccountNamesFilterProxyModelTpl< AccountsProxyModel >(parent) {}
};

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

  void setSelected(const QString& id);
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

protected:
  void wheelEvent(QWheelEvent *ev) override;

protected Q_SLOTS:
  void activated();
  void makeCompletion(const QString& txt) override;
  void selectItem(const QModelIndex& index);

Q_SIGNALS:
  void accountSelected(const QString&);

private:
  class Private;
  QScopedPointer<Private> const d;
};

template <class baseProxyModel>
AccountNamesFilterProxyModelTpl<baseProxyModel>::AccountNamesFilterProxyModelTpl(QObject *parent)
    : baseProxyModel(parent)
{
}

/**
 * Top items are not selectable because they are not real accounts but are only used for grouping.
 */
template <class baseProxyModel>
Qt::ItemFlags AccountNamesFilterProxyModelTpl<baseProxyModel>::flags(const QModelIndex &index) const
{
  if (!index.parent().isValid())
    return baseProxyModel::flags(index) & ~Qt::ItemIsSelectable;
  return baseProxyModel::flags(index);
}

/**
 * Filter all but the first column.
 */
template <class baseProxyModel>
bool AccountNamesFilterProxyModelTpl<baseProxyModel>::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
  Q_UNUSED(source_parent)
  if (source_column == 0)
    return true;
  return false;
}
#endif
// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
