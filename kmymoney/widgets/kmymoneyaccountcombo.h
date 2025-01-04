/*
    SPDX-FileCopyrightText: 2004-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYACCOUNTCOMBO_H
#define KMYMONEYACCOUNTCOMBO_H

#include "config-kmymoney.h"
#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QTreeView;

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
class KMM_BASE_WIDGETS_EXPORT KMyMoneyAccountCombo : public KComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyAccountCombo)

public:
    explicit KMyMoneyAccountCombo(QSortFilterProxyModel *model, QWidget* parent = nullptr);
    explicit KMyMoneyAccountCombo(QWidget* parent = nullptr);
    ~KMyMoneyAccountCombo();

    const QString& getSelected() const;

    /**
     * Clears the cache of the last selected account
     */
    void clearSelection();

    void setModel(QAbstractItemModel* model) KMM_QT6_ONLY(override);

    /**
     * Overridden to get specific behavior
     */
    void setEditable(bool isEditable);

    /**
     * Show the full hierarchical account name in case the combo box
     * is not editable.
     *
     * @sa setEditable
     */
    void setShowFullAccountName(bool showFullName);

    bool eventFilter(QObject* o, QEvent* e) override;

    QTreeView* popup() const;

public Q_SLOTS:
    void expandAll();
    void collapseAll();
    void showPopup() override;
    void hidePopup() override;
    void setSelected(const QString& id);
    void setSplitActionVisible(bool show);

protected:
    void wheelEvent(QWheelEvent *ev) override;
    void setSelected(const QString& id, bool ignoreCache);
    void paintEvent(QPaintEvent* event) override;

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
class KMM_BASE_WIDGETS_EXPORT KMyMoneyAccountComboSplitHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyAccountComboSplitHelper)

public:
    explicit KMyMoneyAccountComboSplitHelper(QComboBox* accountCombo, QAbstractItemModel *model);
    ~KMyMoneyAccountComboSplitHelper();

    /**
     * This method allows to protect the assigned accountCombo
     * object to be changed.
     */
    void setProtectAccountCombo(bool protect);

public Q_SLOTS:
    void updateWidget();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void modelDestroyed();

Q_SIGNALS:
    void accountComboEnabled(bool enabled);
    void accountComboDisabled(bool disabled);

private:
    Q_DECLARE_PRIVATE(KMyMoneyAccountComboSplitHelper)
    QScopedPointer<KMyMoneyAccountComboSplitHelperPrivate>  d_ptr;
};


#endif
// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
