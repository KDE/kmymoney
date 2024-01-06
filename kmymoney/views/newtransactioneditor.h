/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWTRANSACTIONEDITOR_H
#define NEWTRANSACTIONEDITOR_H

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

class NewTransactionEditor : public TransactionEditorBase, TabOrderEditorInterface
{
    Q_OBJECT

public:
    explicit NewTransactionEditor(QWidget* parent = nullptr, const QString& accountId = QString());
    virtual ~NewTransactionEditor();

    /**
     * Returns the currently entered amount
     */
    MyMoneyMoney transactionAmount() const;

    /**
     */
    void loadTransaction(const QModelIndex& index) override;
    QStringList saveTransaction(const QStringList& selectedJournalEntries) override;

    void setAmountPlaceHolderText(const QAbstractItemModel* model) override;

    MyMoneyTransaction transaction() const;

    void loadSchedule(const MyMoneySchedule& schedule);

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
     * This method is used to embed the transaction editor in other dialogs
     * e.g. KEditScheduleDlg
     */
    virtual WidgetHintFrameCollection* widgetHintFrameCollection() const override;

    void setKeepCategoryAmount(bool keepCategoryAmount);

protected:
    bool isTransactionDataValid() const override;

public Q_SLOTS:
    void slotSettingsChanged() override;

protected Q_SLOTS:
    // edit splits directly
    virtual void editSplits();

Q_SIGNALS:
    void postDateChanged(const QDate& date) const;
    void categorySelectionChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
    static QDate  m_lastPostDateUsed;
};

#endif // NEWTRANSACTIONEDITOR_H

