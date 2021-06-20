/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KENTERSCHEDULEDLG_H
#define KENTERSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySchedule;
class MyMoneyAccount;
class MyMoneyTransaction;

namespace Ui {
class KEnterScheduleDlg;
}

namespace eDialogs {
enum class ScheduleResultCode;
}

/**
 * @author Thomas Baumgart
 */

class KEnterScheduleDlgPrivate;
class KEnterScheduleDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KEnterScheduleDlg)

public:
    explicit KEnterScheduleDlg(QWidget* parent, const MyMoneySchedule& schedule);
    ~KEnterScheduleDlg();

    /**
     * Returns the transaction that is about to be entered into the journal
     */
    MyMoneyTransaction transaction();

    /**
     * Show (or hide) the extended dialog keys for 'Skip' and 'Ignore'
     * depending on the value of the parameter @a visible which defaults
     * to @a true.
     */
    void setShowExtendedKeys(bool visible = true);

    /**
     * Return the extended result code. Usage of the returned
     * value only makes sense, once the dialog has been executed.
     * Before execution it returns @a Cancel.
     */
    eDialogs::ScheduleResultCode resultCode() const;

protected:
    /**
     * This method returns the adjusts @a _date according to
     * the setting of the schedule's weekend option.
     */
    QDate date(const QDate& _date) const;

public Q_SLOTS:
    int exec() override;

private:
    KEnterScheduleDlgPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KEnterScheduleDlg)
};

#endif
