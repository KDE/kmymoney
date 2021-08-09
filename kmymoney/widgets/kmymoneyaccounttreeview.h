/*
    SPDX-FileCopyrightText: 2010-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYACCOUNTTREEVIEW_H
#define KMYMONEYACCOUNTTREEVIEW_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObject;
class AccountsViewProxyModel;

namespace eAccountsModel {
enum class Column;
}
namespace eView {
enum class Intent;
}
enum class View;

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMyMoneyAccountTreeViewPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyAccountTreeView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyAccountTreeView)

public:
    explicit KMyMoneyAccountTreeView(QWidget* parent = nullptr);
    ~KMyMoneyAccountTreeView();

    AccountsViewProxyModel *init(View view);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

protected Q_SLOTS:
    void customContextMenuRequested(const QPoint);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

Q_SIGNALS:
    void selectByObject(const MyMoneyObject&, eView::Intent);
    void selectByVariant(const QVariantList&, eView::Intent);
    void returnPressed();

private:
    KMyMoneyAccountTreeViewPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KMyMoneyAccountTreeView)

private Q_SLOTS:
    void slotColumnToggled(const eAccountsModel::Column column, const bool show);
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
