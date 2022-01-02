/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KEDITSCHEDULEDLG_H
#define KEDITSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySchedule;
class MyMoneyAccount;
class MyMoneyTransaction;
class TransactionEditor;
#include "tabordereditor.h"

namespace eMyMoney { namespace Schedule {
enum class Occurrence;
}}

/**
 * @author Thomas Baumgart
 */

class KEditScheduleDlgPrivate;
class KEditScheduleDlg : public QDialog, TabOrderEditorInterface
{
    Q_OBJECT
    Q_DISABLE_COPY(KEditScheduleDlg)

public:
    explicit KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget* parent = nullptr);
    ~KEditScheduleDlg();

    /**
     * Returns the edited schedule.
     *
     * @return MyMoneySchedule The schedule details.
     **/
    const MyMoneySchedule& schedule();

    static void newSchedule(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence);
    static void editSchedule(const MyMoneySchedule& inputSchedule);

    // Implement TabOrderEditorInterface methods
    void setupUi(QWidget* parent) override;
    void storeTabOrder(const QStringList& tabOrder) override;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool focusNextPrevChild(bool next) override;

private Q_SLOTS:
    /// Overridden for internal reasons. No API changes.
    void accept() override;

private:
    KEditScheduleDlgPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KEditScheduleDlg)
};

#endif
