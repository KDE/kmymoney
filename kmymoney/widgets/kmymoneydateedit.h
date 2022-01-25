/*
    SPDX-FileCopyrightText: 2016-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYDATEEDIT_H
#define KMYMONEYDATEEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateEdit>

/**
 * This class is identical to QDateEdit but forces
 * the date to be shown in the format returned by
 * QLocale().dateFormat(QLocale::ShortFormat)
 * but always with four digit year representation
 */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyDateEdit : public QDateEdit
{
    Q_OBJECT

public:
    explicit KMyMoneyDateEdit(QWidget* parent = nullptr);
    KMyMoneyDateEdit(const QDate& date, QWidget* parent);

    // overridden to protect against uic generated calls
    // Use QDateEdit::setDisplayFormat() if you need to
    // ever change the display format via code
    void setDisplayFormat(const QString& format);

    static QDate invalid_date();
};
#endif // KMYMONEYDATEEDIT_H
