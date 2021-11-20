/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SIMPLELEDGERVIEW_H
#define SIMPLELEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"
class SelectedObjects;

class SimpleLedgerViewPrivate;
class SimpleLedgerView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit SimpleLedgerView(QWidget *parent = nullptr);
    ~SimpleLedgerView() override;

    virtual void showTransactionForm(bool = true);

    void updateActions(const SelectedObjects& selections) override;
    void executeAction(eMenu::Action action, const SelectedObjects& selections) override;

public Q_SLOTS:
    void showEvent(QShowEvent* event) override;

    void slotSettingsChanged() override;

private Q_SLOTS:
    void tabSelected(int idx);
    void tabClicked(int idx);
    /**
     * Open the ledger of account @a accountId and make it the current ledger
     */
    void openLedger (QString accountId);
    void closeLedger(int idx);
    void checkTabOrder(int from, int to);
    void setupCornerWidget();
    void slotRequestSelectionChange(const SelectedObjects& selections) const;

    void sectionResized(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize) const;
    void sectionMoved(QWidget* view, int section, int oldIndex, int newIndex) const;

protected:
    bool eventFilter(QObject* o, QEvent* e) override;
    void aboutToShow() override;

Q_SIGNALS:
    void showForms(bool show);
    void settingsChanged();
    void resizeSection(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize) const;
    void moveSection(QWidget* view, int section, int oldIndex, int newIndex) const;

private:
    Q_DECLARE_PRIVATE(SimpleLedgerView)
};

#endif // SIMPLELEDGERVIEW_H

