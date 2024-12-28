/*
    SPDX-FileCopyrightText: 2019-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INVESTTRANSACTIONEDITOR_H
#define INVESTTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
class QWidget;

// ----------------------------------------------------------------------------
// Project Includes

#include "tabordereditor.h"
#include "transactioneditorbase.h"

class MyMoneyMoney;

class InvestTransactionEditor : public TransactionEditorBase, TabOrderEditorInterface
{
    Q_OBJECT

public:
    explicit InvestTransactionEditor(QWidget* parent = nullptr, const QString& accountId = QString());
    virtual ~InvestTransactionEditor();

    /**
     */
    void loadTransaction(const QModelIndex& index) override;
    QStringList saveTransaction(const QStringList& selectedJournalEntries) override;

    /**
     * Reimplemented to suppress some events in certain conditions
     */
    bool eventFilter(QObject* o, QEvent* e) override;

    /**
     * Returns the transaction amount
     */
    MyMoneyMoney totalAmount() const;

    // Implement TabOrderEditorInterface methods
    void setupUi(QWidget* parent) override;
    void storeTabOrder(const QStringList& tabOrder) override;

    QDate postDate() const override;

protected:
    bool isTransactionDataValid() const override;

public Q_SLOTS:
    void slotSettingsChanged() override;

protected Q_SLOTS:
    void activityChanged (int index);

    void updateWidgets();
    void updateTotalAmount();

private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // INVESTTRANSACTIONEDITOR_H

