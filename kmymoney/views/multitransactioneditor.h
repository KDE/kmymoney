/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MULTITRANSACTIONEDITOR_H
#define MULTITRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "tabordereditor.h"
#include "transactioneditorbase.h"

class MyMoneySchedule;

class MultiTransactionEditor : public TransactionEditorBase, TabOrderEditorInterface
{
    Q_OBJECT

public:
    explicit MultiTransactionEditor(QWidget* parent = nullptr, const QString& accountId = QString());
    virtual ~MultiTransactionEditor();

    /**
     */
    void loadTransaction(const QModelIndex& index) override;
    QStringList saveTransaction(const QStringList& selectedJournalEntries) override;

    void setAmountPlaceHolderText(const QAbstractItemModel* model) override;

    /**
     * Reimplemented to suppress some events in certain conditions
     */
    bool eventFilter(QObject* o, QEvent* e) override;

    void setShowAccountCombo(bool show) const;
    void setShowNumberWidget(bool show) const;
    void setShowButtons(bool show) const;
    void setAccountId(const QString& accountId);

    QDate postDate() const;

    /// overridden for internal reasons
    void setReadOnly(bool readOnly) override;

    // Implement TabOrderEditorInterface methods
    void setupUi(QWidget* parent) override;
    void storeTabOrder(const QStringList& tabOrder) override;

    /**
     * @copydoc TransactionEditorBase::setSelectedJournalEntryIds
     */
    virtual bool setSelectedJournalEntryIds(const QStringList& selectedJournalEntryIds) override;

    /**
     * @copydoc TransactionEditorBase::errorMessage
     */
    virtual QString errorMessage() const override;

protected:
    bool isTransactionDataValid() const override;

public Q_SLOTS:
    void slotSettingsChanged() override;

Q_SIGNALS:
    void postDateChanged(const QDate& date);

private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // MULTITRANSACTIONEDITOR_H
