/*
    SPDX-FileCopyrightText: 2026 Dada Krpasundarananda
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSIDEBAR_H
#define ACCOUNTSIDEBAR_H

#include <QTreeWidget>

#include <KConfigGroup>

class QTabWidget;
class MyMoneyAccount;

/**
 * A flat list of all open asset and liability accounts with their
 * balance as of today, shown next to the ledger tabs.
 * Clicking an entry emits accountSelected() with the account id.
 *
 * Per-account visibility and the drag order are stored in the data file
 * so each KMyMoney file keeps its own setup:
 * - SideBarVisibility (account key-value pair): "Normal", "Hidden" or
 *   "AlwaysShown". Missing or empty means Normal.
 * - SideBarOrder (file-level key-value pair): comma-separated account
 *   ids in the order the user dragged them to.
 *
 * The options that control the sidebar itself stay in the config group
 * "AccountSidebar" and apply to all files:
 * - Position:   "right" (default) or "left" of the ledger tabs
 * - ShowHidden: show hidden accounts greyed, so they can be unhidden
 * - HideZero:   hide accounts whose balance today is zero
 * - Visible:    sidebar shown or not (see AccountSidebarVisibleByDefault)
 * - Width:      width of the sidebar in pixels (default: fits the accounts)
 * - HideTabBar: hide the tab bar of the ledger tabs (the tabs keep working)
 *
 * TODO: optional tree view (accounts grouped by parent)
 */
/// whether the sidebar is shown when the user never chose (flip to false for upstream)
constexpr bool AccountSidebarVisibleByDefault = true;

class AccountSidebar : public QTreeWidget
{
    Q_OBJECT
public:
    explicit AccountSidebar(QWidget* parent = nullptr);

    /// true when the user chose "Hide ledger tab bar"
    bool tabBarHidden() const;

    /// keys and values stored in the data file
    static constexpr const char visibilityKey[] = "SideBarVisibility";
    static constexpr const char orderKey[] = "SideBarOrder";
    static constexpr const char tabsKey[] = "SideBarTabsBeforeHide";
    static constexpr const char visibilityNormal[] = "Normal";
    static constexpr const char visibilityHidden[] = "Hidden";
    static constexpr const char visibilityAlwaysShown[] = "AlwaysShown";

    /// ids of the ledger tabs open before the tab bar was hidden; the current one ends with "*"
    static QStringList tabsBeforeHide();
    static void setTabsBeforeHide(const QStringList& ids);

public Q_SLOTS:
    /// rebuild the list from the current file
    void refresh();

    /// empty the list when the current file is closed
    void fileClosed();

    /// put the sidebar left or right of the ledger tabs and remember it
    void setOnLeft(bool left);

    /// the ledger tabs whose tab bar can be hidden from the sidebar menu
    void setTabWidget(QTabWidget* tabs);

    /// highlight the account of the current ledger (empty id: no highlight)
    void selectAccount(const QString& accountId);

Q_SIGNALS:
    void accountSelected(const QString& accountId);
    /// the user switched "Hide ledger tab bar" on or off
    void tabBarHiddenChanged(bool hidden);
    /// the user chose "Hide sidebar" in the context menu
    void hideRequested();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    Qt::DropActions supportedDropActions() const override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    /// Normal / Hidden / AlwaysShown; missing or empty means Normal
    static QString accountVisibility(const MyMoneyAccount& acc);
    static void setAccountVisibility(const QString& accountId, const QString& value);
    /// account ids in the user's order; accounts not listed stay at the end
    static QStringList accountOrder();
    static void setAccountOrder(const QStringList& ids);
    KConfigGroup settings() const;
    void applyTabBarSetting();
    void removeGap();
    void applyWidth();
    void saveWidth();
    int defaultWidth() const;

    QTreeWidgetItem* m_gap = nullptr; ///< empty row showing where a dragged account will land

    QTabWidget* m_tabs = nullptr;
    bool m_widthApplied = false; ///< saved width is applied when first shown
    QString m_selectedId; ///< account of the current ledger, kept when the list is rebuilt
    bool m_refreshing = false; ///< ignore nested refresh while migrating or rebuilding
    bool m_blockRefresh = false; ///< skip dataChanged refresh until a drag has finished
};

#endif
