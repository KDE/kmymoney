/*
    SPDX-FileCopyrightText: 2007-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KENTERSCHEDULEDLG_H
#define KENTERSCHEDULEDLG_H

#include "kmm_oldregister_export.h"

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
class KMM_OLDREGISTER_EXPORT KEnterScheduleDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KEnterScheduleDlg)

public:
    explicit KEnterScheduleDlg(QWidget *parent, const MyMoneySchedule& schedule);
    ~KEnterScheduleDlg();

    TransactionEditor* startEdit();
    MyMoneyTransaction transaction();

    /**
     * Show (or hide) the extended dialog keys for 'Skip' and 'Ignore'
     * depending on the value of the parameter @a visible which defaults
     * to @a true.
     */
    void showExtendedKeys(bool visible = true);

    /**
     * Return the extended result code. Usage of the returned
     * value only makes sense, once the dialog has been executed.
     * Before execution it returns @a Cancel.
     */
    eDialogs::ScheduleResultCode resultCode() const;

protected:
    /// Overridden for internal reasons. No API changes.
    bool focusNextPrevChild(bool next) override;

    /**
      * This method returns the adjusts @a _date according to
      * the setting of the schedule's weekend option.
      */
    QDate date(const QDate& _date) const;

    void resizeEvent(QResizeEvent* ev) override;

public Q_SLOTS:
    int exec() override;

private Q_SLOTS:
    void slotSetupSize();
    void slotShowHelp();
    void slotIgnore();
    void slotSkip();

private:
    KEnterScheduleDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KEnterScheduleDlg)
};

#endif
