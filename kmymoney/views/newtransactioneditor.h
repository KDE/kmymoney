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

#include "transactioneditorbase.h"
#include "mymoneymoney.h"

class NewTransactionEditor : public TransactionEditorBase
{
    Q_OBJECT

public:
    explicit NewTransactionEditor(QWidget* parent = 0, const QString& accountId = QString());
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

    /**
     * Reimplemented to suppress some events in certain conditions
     */
    bool eventFilter(QObject* o, QEvent* e) override;

protected:
    virtual void keyPressEvent(QKeyEvent* e) override;

protected Q_SLOTS:
    virtual void reject();
    virtual void acceptEdit();

    virtual void editSplits();

    virtual void numberChanged(const QString& newNumber);
    virtual void categoryChanged(const QString& accountId);
    virtual void costCenterChanged(int costCenterIndex);
    virtual void postdateChanged(const QDate& date);
    virtual void payeeChanged(int payeeIndex);
    virtual void tagsChanged(const QStringList& tagIds);

    void valueChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
    static QDate  m_lastPostDateUsed;
};

#endif // NEWTRANSACTIONEDITOR_H

