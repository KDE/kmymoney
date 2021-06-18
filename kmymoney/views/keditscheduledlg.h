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

namespace eMyMoney { namespace Schedule {
enum class Occurrence;
}}

/**
 * @author Thomas Baumgart
 */

class KEditScheduleDlgPrivate;
class KEditScheduleDlg : public QDialog
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

protected:
    /**
     * This method adjusts @a _date according to the rules specified by
     * the schedule's weekend option.
     */
    QDate adjustDate(const QDate& _date) const;

private Q_SLOTS:
    /// Overridden for internal reasons. No API changes.
    void accept() override;

private:
    KEditScheduleDlgPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KEditScheduleDlg)
};

#endif
