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

#include "mymoneyfactory.h"
#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "tabordereditor.h"
#include "transactioneditorbase.h"

class MyMoneySchedule;

class NewTransactionEditor : public TransactionEditorBase, TabOrderEditorInterface, MyMoneyFactory
{
    Q_OBJECT

public:
    typedef enum {
        EditSchedule,
        EnterSchedule,
    } ScheduleEditType;

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

    void loadSchedule(const MyMoneySchedule& schedule, ScheduleEditType editType);

    /**
     * Reimplemented to suppress some events in certain conditions
     */
    bool eventFilter(QObject* o, QEvent* e) override;

    void setShowAccountCombo(bool show) const;
    void setShowNumberWidget(bool show) const;
    void setShowButtons(bool show) const;
    void setAccountId(const QString& accountId);

    QDate postDate() const override;

    /// overridden for internal reasons
    void setReadOnly(bool readOnly) override;

    // Implement TabOrderEditorInterface methods
    /**
     * Setup the tab order dialog with the UI elements of this object
     * Use @a parent as the parent widget and return a pointer
     * to myself.
     */
    QWidget* setupUi(QWidget* parent) override;

    /**
     * Called by the taborder editor to store the new
     * @a tabOrder settings.
     */
    void storeTabOrder(const QStringList& tabOrder) override;

    void setKeepCategoryAmount(bool keepCategoryAmount);

    /**
     * If @a tabOrder is not equal to @c nullptr it is returned instead
     * of our own order object. This allows to embed the transaction editor
     * into the tab order of another widget (e.g. KEditScheduleDlg)
     * If @a tabOrder is equal to @c nullptr a pointer to our own object is
     * returned.
     *
     * @sa tabOrder()
     */
    void setExternalTabOrder(TabOrder* tabOrder);

protected:
    bool isTransactionDataValid() const override;

    /**
     * Return a pointer to our own TabOrder object or the
     * one passed in using setExternalTabOrder.
     *
     * @sa setExternalTabOrder()
     */
    TabOrder* tabOrder() const override;

public Q_SLOTS:
    void slotSettingsChanged() override;

protected Q_SLOTS:
    // edit splits directly
    virtual void editSplits();

Q_SIGNALS:
    void postDateChanged(const QDate& date);
    void categorySelectionChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
    static QDate  m_lastPostDateUsed;
};

#endif // NEWTRANSACTIONEDITOR_H

