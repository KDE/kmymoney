/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITDIALOG_H
#define SPLITDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class MyMoneyAccount;
class MyMoneySecurity;
class SplitModel;

/**
 * This dialog allows the user to modify the splits of a transaction.
 * The splits are passed in form of a SplitModel via setModel(). The
 * total amount of the transaction is passed as @a mainAmount. If
 * @a mainAmount equals to MyMoneyMoney::autoCalc then the total
 * will automatically be adjusted.
 */
class SplitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SplitDialog(const MyMoneySecurity& commodity,
                         const MyMoneyMoney& mainAmount,
                         int fraction,
                         const MyMoneyMoney& inversionFactor,
                         QWidget* parent,
                         Qt::WindowFlags f = {});
    virtual ~SplitDialog();

    void setModel(SplitModel* model);
    void setAccountId(const QString& id);
    void setTransactionPayeeId(const QString& id);
    void setReadOnly(bool readOnly);

    /**
     * Returns the amount for the transaction.
     */
    MyMoneyMoney transactionAmount() const;

public Q_SLOTS:
    void accept() final override;
    int exec() final override;

private Q_SLOTS:
    void adjustSummary();

    void disableButtons();
    void enableButtons();

    void newSplit();

protected Q_SLOTS:
    void deleteSelectedSplits();
    void deleteAllSplits();
    void deleteZeroSplits();
    void mergeSplits();
    void selectionChanged();
    void updateButtonState();
    void adjustUnassigned();

protected:
    void resizeEvent(QResizeEvent* ev) final override;
    void adjustSummaryWidth();
    // QBrush m_unassigned_over;
    // QBrush m_unassigned_under;
    QBrush m_unassigned_normal;
    QBrush m_unassigned_error;

private:
    class Private;
    QScopedPointer<Private> d;
};
#endif // SPLITDIALOG_H
