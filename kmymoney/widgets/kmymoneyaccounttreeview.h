/*
    SPDX-FileCopyrightText: 2010-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYACCOUNTTREEVIEW_H
#define KMYMONEYACCOUNTTREEVIEW_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytreeview.h"

class AccountsProxyModel;
class KMyMoneyAccountTreeViewPrivate;
class SelectedObjects;

namespace eMenu {
enum class Action;
enum class Menu;
}

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyAccountTreeView : public KMyMoneyTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyAccountTreeView)

public:
    explicit KMyMoneyAccountTreeView(QWidget* parent = nullptr);
    ~KMyMoneyAccountTreeView();

    AccountsProxyModel* proxyModel() const;

    /**
     * This method attaches the @a model to the view while
     * inserting the @sa proxyModel() in between them.
     */
    void setModel(QAbstractItemModel* model) override;

    /**
     * This method replaces the existing proxy model with @a model.
     * @a model will be reparented to the this object.
     *
     * @note It is advisable to replace this soon after construction
     * and not during operation.
     */
    void setProxyModel(AccountsProxyModel* model);

protected Q_SLOTS:
    void customContextMenuRequested(const QPoint);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

Q_SIGNALS:
    void requestSelectionChange (const SelectedObjects& selections) const;
    void requestCustomContextMenu(eMenu::Menu contextMenu, const QPoint& pos) const;
    void requestActionTrigger(eMenu::Action action);

private:
    const QScopedPointer<KMyMoneyAccountTreeViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(KMyMoneyAccountTreeView)
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
