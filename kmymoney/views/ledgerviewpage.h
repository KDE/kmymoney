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

namespace eMenu {
enum class Menu;
enum class Action;
}

class MyMoneyAccount;
class SelectedObjects;
struct AccountBalances;

class LedgerViewPage : public QWidget
{
    Q_OBJECT
protected:
    class Private;
    Private* d;

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

    void selectJournalEntry(const QString& id);

    const SelectedObjects& selections() const;

    /**
     * Execute the @a action based on the selected objects found in @a selections.
     *
     * @retval true in case operation succeeded
     * @retval false in case operation was aborted or failed
     */
    virtual bool executeAction(eMenu::Action action, const SelectedObjects& selections);

    void pushView(LedgerViewPage* view);
    LedgerViewPage* popView();
    bool hasPushedView() const;

    QString accountName();

    QList<int> splitterSizes() const;
    void setSplitterSizes(QList<int> sizes);

    /**
     * Initialize the model for the view if called
     * for the first time. Successive calls don't
     * have an effect.
     *
     * @sa showEvent()
     */
    void prepareToShow();

protected:
    explicit LedgerViewPage(Private& dd, QWidget* parent = nullptr, const QString& configGroupName = QString());
    void showEvent(QShowEvent* event) override;
    void initModel();

public Q_SLOTS:
    void showTransactionForm(bool show);
    void splitterChanged(int pos, int index);
    void slotSettingsChanged();
    virtual void updateSummaryInformation(const QHash<QString, AccountBalances>& balances);

protected Q_SLOTS:
    void startEdit();
    void finishEdit();
    void keepSelection();
    void reloadFilter();
    void slotRequestSelectionChanged(const SelectedObjects& selections) const;

Q_SIGNALS:
    void requestSelectionChanged(const SelectedObjects& selection);
    void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos);

    void transactionSelected(const QModelIndex& idx);
    void aboutToStartEdit();
    void aboutToFinishEdit();

    void resizeSection(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize);
    void sectionResized(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize);
    void moveSection(QWidget* view, int section, int oldIndex, int newIndex);
    void sectionMoved(QWidget* view, int section, int oldIndex, int newIndex);

    void requestView(QWidget* viewWidget, const QString& accountId, const QString& journalEntryId);
};

#endif // LEDGERVIEWPAGE_H

