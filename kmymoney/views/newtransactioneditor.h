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
#include "transactioneditorbase.h"

class MyMoneySchedule;

class NewTransactionEditor : public TransactionEditorBase
{
    Q_OBJECT

public:
    explicit NewTransactionEditor(QWidget* parent = nullptr, const QString& accountId = QString());
    virtual ~NewTransactionEditor();

    /**
     * This method returns true if the user pressed the enter button.
     * It remains false, in case the user pressed the cancel button.
     */
    virtual bool accepted() const override;

    /**
     * Returns the currently entered amount
     */
    MyMoneyMoney transactionAmount() const;

    /**
     */
    void loadTransaction(const QModelIndex& index) override;
    void saveTransaction() override;

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

protected:
    virtual void keyPressEvent(QKeyEvent* e) override;

protected Q_SLOTS:
    virtual void reject();
    virtual void acceptEdit();

    // edit splits directly
    virtual void editSplits();

Q_SIGNALS:
    void postDateChanged(const QDate& date) const;

private:
    class Private;
    QScopedPointer<Private> const d;
    static QDate  m_lastPostDateUsed;
};

#endif // NEWTRANSACTIONEDITOR_H

