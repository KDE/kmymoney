/*
    SPDX-FileCopyrightText: 2016-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYDATEEDIT_H
#define KMYMONEYDATEEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// Qt Includes

#include <QDateEdit>
#include <QMetaMethod>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KDateComboBox>

class QKeyEvent;
class KMyMoneyDateEditPrivate;
/**
 * This class provides a date entry widget with a lot more
 * functionality than found in QDateEdit or KDateComboBox.
 *
 * - Use Cursor up/down keys to advance day, month or year
 * - Use +/- keys to advance date across months/years
 * - Pressing T sets today's date
 * - Moving cursor selects sections
 * - Enter date from keyboard (only day and month are
 *   needed, the year will be amended by the current year
 * - works together with WidgetHintFrame to identify
 *   invalid dates during input by emitting dateValidityChanged()
 *   signal
 * - Allows entering a date via keyboard
 * - If only day and month are entered, the current year
 *   will be appended when focus is left
 * - Two digit year entry will be enhanced with current century
 * - provides global option to select initial edit section
 *   (day, month or year) depending on application settings
 *   see setInitialSection()
 * - supports completely empty date to identify no-change
 *   (needs to be enabled, default is off). See setAllowEmptyDate()
 */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyDateEdit : public KDateComboBox
{
    Q_OBJECT

public:
    explicit KMyMoneyDateEdit(QWidget* parent = nullptr);
    virtual ~KMyMoneyDateEdit();

    void keyPressEvent(QKeyEvent* keyEvent) override;

    /**
     * Switches the mode to allow complete empty date (@a allowEmptyDate
     * equals @c true) or not. In case it is not allowed, an invalid date
     * will be replaced with the current date.
     */
    void setAllowEmptyDate(bool allowEmptyDate);

    /**
     * Return the currently entered date. If isNull() returns
     * @c true, this will always return an invalid date even
     * though isValid() returns @c true. This is the case when
     * allowEmptyDate is @c true and the edit area is empty.
     *
     * @return the currently entered date
     */
    QDate date() const;

    /**
     * Return if the current user input is valid. Depending
     * on the setting of allowEmptyDate, the behavior varies.
     * If allowEmptyDate is @c false (the default) the return
     * value is @c false if isNull() returns @c true. If
     * allowEmptyDate is @c true the return value is @c true
     * in this case.
     *
     * @returns if the current user input is valid or not
     *
     * @see isNull(), setAllowEmptyDate()
     */
    bool isValid() const;

    /**
     * Globally set the @a section which is selected when a date
     * is passed using setDate(). If an invalid section is passed
     * the DaySection is used which is also the default.
     *
     * @param section One of QDateEdit::DaySection, QDateEdit::MonthSection or QDateEdit::YearSection.
     *
     * @sa QDateEdit::Section, setDate()
     */
    void setInitialSection(QDateEdit::Section section);

    /**
     * Returns the global setting of the initial section
     *
     * @returns One of DaySection, MonthSection or YearSection.
     */
    QDateEdit::Section initialSection() const;

public Q_SLOTS:
    /**
     * Sets the date shown to @a date. The behavior when passing
     * an invalid date depends on a prior call to setAllowEmptyDate().
     * If called with @c false (the default) this method will set
     * the date to QDate::currentDate(). If ccalled with @c true
     * it clears the edit area to blank.
     *
     * @sa setAllowEmptyDate(), date()
     */
    void setDate(const QDate& date);

    /**
     * Overridden to force the display format to always be
     * short format.
     */
    void setDisplayFormat(QLocale::FormatType format);

Q_SIGNALS:
    /**
     * This signal is send out if the validity of the date
     * changes. It is also send out when a new object is
     * connected to this signal.
     */
    void dateValidityChanged(const QDate& date);

protected:
    void focusOutEvent(QFocusEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void connectNotify(const QMetaMethod& signal) override;

private:
    KMyMoneyDateEditPrivate* d;
};
#endif // KMYMONEYDATEEDIT_H
