/*
    SPDX-FileCopyrightText: 2016-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneydateedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDateTimeEdit>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

class KMyMoneyDateEditSettings
{
public:
    QDateEdit::Section initialSection{QDateEdit::DaySection};
};

Q_GLOBAL_STATIC(KMyMoneyDateEditSettings, s_globalKMyMoneyDateEditSettings)

class KMyMoneyDateEditPrivate
{
public:
    KMyMoneyDateEditPrivate(KMyMoneyDateEdit* qq)
        : q(qq)
        , m_originalDay(0)
        , m_emptyDateAllowed(false)
        , m_dateValidity(false)
        , m_lastKeyPressWasEscape(false)
    {
        int sectionIndex(0);
        bool lastWasDelimiter(false);
        m_sections.resize(3);
        const auto format(q->locale().dateFormat(QLocale::ShortFormat).toLower());

        for (int pos = 0; pos < format.length(); ++pos) {
            const auto ch = format[pos];
            if (ch == QLatin1Char('d')) {
                m_sections[sectionIndex] = QDateTimeEdit::DaySection;
                lastWasDelimiter = false;
            } else if (ch == QLatin1Char('m')) {
                m_sections[sectionIndex] = QDateTimeEdit::MonthSection;
                lastWasDelimiter = false;
            } else if (ch == QLatin1Char('y')) {
                m_sections[sectionIndex] = QDateTimeEdit::YearSection;
                lastWasDelimiter = false;
            } else {
                if (!m_validDelims.contains(ch)) {
                    m_validDelims.append(ch);
                }
                if (!lastWasDelimiter) {
                    ++sectionIndex;
                    lastWasDelimiter = true;
                }
            }
        }
    }

    /**
     * Returns @c true if the character @a ch is a
     * valid delimiter otherwise @c false.
     */
    bool isValidDelimiter(const QChar& ch) const
    {
        return m_validDelims.contains(ch);
    }

    /**
     * Returns the section that the cursor currently
     * resides in
     */
    QDateTimeEdit::Section sectionByCursorPos() const
    {
        const auto text(q->lineEdit()->text());
        const auto pos(q->lineEdit()->cursorPosition());
        int sectionIndex(0);

        for (int idx(0); idx < pos; ++idx) {
            if (isValidDelimiter(text[idx])) {
                ++sectionIndex;
            }
        }
        return m_sections[sectionIndex];
    }

    /**
     * Returns the location of the @a section in
     * the date string
     */
    int partBySection(QDateTimeEdit::Section section) const
    {
        for (int idx(0); idx < m_sections.size(); ++idx) {
            if (m_sections[idx] == section) {
                return idx;
            }
        }
        return -1;
    }

    /**
     * Returns @c true if the character at position
     * @a pos is a delimiter, @c false otherwise.
     */
    bool isDelimiterAtPos(int pos) const
    {
        if (pos < 0 || pos >= q->lineEdit()->text().length()) {
            return false;
        }
        return isValidDelimiter(q->lineEdit()->text().at(pos));
    }

    /**
     * Dissects the current input into the individual parts
     * and returns them as a vector.
     */
    QVector<QString> editParts() const
    {
        const auto text(q->lineEdit()->text());
        QVector<QString> parts(3);
        int partIndex(0);
        for (int idx(0); idx < text.length(); ++idx) {
            const auto ch(text.at(idx));
            if (isValidDelimiter(ch)) {
                ++partIndex;
            } else {
                parts[partIndex].append(ch);
            }
        }
        return parts;
    }

    /**
     * Returns the QDate of the current input adjusted so that
     * - a missing year information is replaced by the current year
     * - a two digit year string is adjusted into the current century
     *
     * If any of the day or month part is missing, an invalid date is
     * returned. The same applies, when the current input represents
     * an invalid date (e.g. February 31st).
     *
     * In case the input is empty and empty input is allowd, a phony
     * date (which is valid but does not represent a real date) is
     * returned.
     */
    QDate fixupDate()
    {
        if (q->isNull() && m_emptyDateAllowed) {
            return QDate::fromJulianDay(1);
        }
        QVector<QString> parts(editParts());
        const auto dayIndex = partBySection(QDateTimeEdit::DaySection);
        const auto monthIndex = partBySection(QDateTimeEdit::MonthSection);
        const auto yearIndex = partBySection(QDateTimeEdit::YearSection);

        if (parts.at(dayIndex).isEmpty() || parts.at(monthIndex).isEmpty()) {
            return {};
        }

        if (parts.at(yearIndex).isEmpty()) {
            parts[yearIndex] = QString::number(QDate::currentDate().year());
        }
        if (parts.at(yearIndex).length() == 2) {
            parts[yearIndex] = QStringLiteral("%1%2").arg(QString::number(QDate::currentDate().year() / 100), parts[yearIndex]);
        }

        return QDate(parts[yearIndex].toInt(), parts[monthIndex].toInt(), parts[dayIndex].toInt());
    }

    /**
     * Adjust the day of the @a date by the amount provided by @a delta
     * in days. In case the resulting day is not within the bounds of
     * the month of @a date an invalid date is returned.
     */
    QDate adjustDay(QDate date, int delta)
    {
        const auto minValue(1);
        const auto maxValue(date.daysInMonth());
        const auto newDay(date.day() + delta);
        if ((newDay >= minValue) && (newDay <= maxValue)) {
            return QDate(date.year(), date.month(), newDay);
        }
        return {};
    }

    /**
     * Adjust the month of the @a date by the amount provided by @a delta
     * in months. In case the resulting month is not within the bounds of
     * valid months (1..12) an invalid date is returned. In case the
     * original day is larger than the current day, it will be used
     * instead. In case the day is not within the range of valid days
     * for the resulting month the day will be adjusted to the last day
     * of the month.
     */
    QDate adjustMonth(QDate date, int delta)
    {
        const auto minValue(1);
        const auto maxValue(12);
        const auto newMonth(date.month() + delta);

        if ((newMonth >= minValue) && (newMonth <= maxValue)) {
            auto day = date.day();
            if (m_originalDay > day)
                day = m_originalDay;
            auto newDate = QDate(date.year(), newMonth, day);
            if (!newDate.isValid()) {
                const auto maxDays(QDate(date.year(), newMonth, 1).daysInMonth());
                newDate = QDate(date.year(), newMonth, maxDays);
            }
            return newDate;
        }
        return {};
    }

    /**
     * Adjust the year of the @a date by the amount provided by @a delta
     * in years. In case the resulting year is not within the bounds of
     * valid years (1..4000) an invalid date is returned. In case the
     * original day is larger than the current day, it will be used
     * instead. In case the day is not within the range of valid days
     * for the resulting month the day will be adjusted to the last day
     * of the month.
     */
    QDate adjustYear(QDate date, int delta)
    {
        const auto minValue(1);
        const auto maxValue(4000);
        const auto newYear(date.year() + delta);
        if ((newYear >= minValue) && (newYear <= maxValue)) {
            auto day = date.day();
            if (m_originalDay > day)
                day = m_originalDay;
            auto newDate = QDate(newYear, date.month(), day);
            if (!newDate.isValid()) {
                const auto maxDays(QDate(newYear, date.month(), 1).daysInMonth());
                newDate = QDate(newYear, date.month(), maxDays);
            }
            return newDate;
        }
        return {};
    }

    /**
     * Mark the text of the @a section as selected and place the
     * cursor behind the last character of that section.
     *
     * @sa QLineEdit::setSelection(), QLineEdit::setCursorPosition()
     */
    void selectSection(QDateEdit::Section section)
    {
        auto part(partBySection(section));
        const auto text(q->lineEdit()->text());
        const auto length(text.length());
        int start(-1);
        int end(-1);

        for (int pos = 0; pos < length; ++pos) {
            const auto isDelimiter(isValidDelimiter(text.at(pos)));
            if (part == 0) {
                if (start == -1) {
                    start = pos;
                }
                if (isDelimiter) {
                    end = pos;
                    break;
                }
            } else {
                if (isDelimiter) {
                    --part;
                }
            }
        }
        if (end == -1) {
            end = length;
        }
        q->lineEdit()->setCursorPosition(end);

        // in case the text field is empty, we cannot select anything
        if (start >= 0) {
            q->lineEdit()->setSelection(start, end - start);
        }
    }

    /**
     * In case the lineEdit object contains text and represents
     * a valid date, the @a section of the date is adjusted
     * by @a delta units. In case the @a section is
     * QDateEdit::NoSection, delta is added to the days
     * of the date including wrapping into the next/previous
     * month. This may also affect the year when the month
     * wraps.
     *
     * If the new date is valid and a disting section (day,
     * month or year) was selected, the text of this section
     * will be selected as part of the operation.
     */
    void adjustDate(int delta, QDateEdit::Section section)
    {
        if (!q->lineEdit()->text().isEmpty()) {
            auto date = fixupDate();
            if (date.isValid()) {
                switch (section) {
                case QDateEdit::DaySection:
                    date = adjustDay(date, delta);
                    if (date.isValid()) {
                        m_originalDay = date.day();
                    }
                    break;
                case QDateEdit::MonthSection:
                    date = adjustMonth(date, delta);
                    break;
                case QDateEdit::YearSection:
                    date = adjustYear(date, delta);
                    break;
                case QDateEdit::NoSection:
                default:
                    date = date.addDays(delta);
                    m_originalDay = date.day();
                    break;
                }
                if (date.isValid()) {
                    setDate(date);
                    if (section != QDateEdit::NoSection) {
                        selectSection(section);
                    }
                }
            }
        }
    }

    /**
     * Set the @a date into the widget. In case the empty date
     * is allowed and an invalid date is provided, the text of
     * the lineEdit widget will be cleared.
     */
    void setDate(const QDate& date)
    {
        q->KDateComboBox::setDate(date);
        if (m_emptyDateAllowed && !date.isValid()) {
            q->lineEdit()->clear();
        }
    }

    KMyMoneyDateEdit* q;
    QVector<QChar> m_validDelims;
    QVector<QDateTimeEdit::Section> m_sections;
    int m_originalDay;
    bool m_emptyDateAllowed;
    bool m_dateValidity;
    bool m_lastKeyPressWasEscape;
};

KMyMoneyDateEdit::KMyMoneyDateEdit(QWidget* parent)
    : KDateComboBox(parent)
    , d(new KMyMoneyDateEditPrivate(this))
{
    setOptions(KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::WarnOnInvalid);

    connect(lineEdit(), &QLineEdit::textChanged, this, [&]() {
        const auto newDate(d->fixupDate());
        if (newDate.isValid() != d->m_dateValidity) {
            Q_EMIT dateValidityChanged(newDate);
            d->m_dateValidity = newDate.isValid();
        }
    });

    setDate(QDate::currentDate());

    d->selectSection(s_globalKMyMoneyDateEditSettings()->initialSection);
}

KMyMoneyDateEdit::~KMyMoneyDateEdit()
{
    delete d;
}

void KMyMoneyDateEdit::connectNotify(const QMetaMethod& signal)
{
    // Whenever a new object connects to our dateValidityChanged signal
    // we Q_EMIT the current status right away.
    if (signal == QMetaMethod::fromSignal(&KMyMoneyDateEdit::dateValidityChanged)) {
        const auto newDate(d->fixupDate());
        Q_EMIT dateValidityChanged(newDate);
    }
}

void KMyMoneyDateEdit::setDate(const QDate& date)
{
    // force sending out a single dateValidityChanged signal
    const auto isNewDate(date != this->date());
    QSignalBlocker blocker(lineEdit());
    d->setDate(date);
    d->m_originalDay = date.day();
    if (isNewDate) {
        d->selectSection(s_globalKMyMoneyDateEditSettings()->initialSection);
    }
    const auto newDate = d->fixupDate();
    Q_EMIT dateValidityChanged(newDate);
    d->m_dateValidity = newDate.isValid();
}

void KMyMoneyDateEdit::setInitialSection(QDateEdit::Section section)
{
    switch (section) {
    case QDateEdit::DaySection:
    case QDateEdit::MonthSection:
    case QDateEdit::YearSection:
        break;
    default:
        section = QDateEdit::DaySection;
        break;
    }
    s_globalKMyMoneyDateEditSettings()->initialSection = section;
}

QDateEdit::Section KMyMoneyDateEdit::initialSection() const
{
    return s_globalKMyMoneyDateEditSettings()->initialSection;
}

QDate KMyMoneyDateEdit::date() const
{
    const auto date(d->fixupDate());
    if (date.toJulianDay() == 1)
        return {};
    return date;
}

bool KMyMoneyDateEdit::isValid() const
{
    return d->fixupDate().isValid();
}

void KMyMoneyDateEdit::focusOutEvent(QFocusEvent* event)
{
    const auto reason = event->reason();
    if ((reason == Qt::TabFocusReason) || (reason == Qt::BacktabFocusReason)) {
        const auto date(d->fixupDate());
        if (!date.isValid()) {
            // if the editor is quit due to pressing ESC, we don't
            // show an error for having an invalid date.
            if (!d->m_lastKeyPressWasEscape) {
                KMessageBox::error(this, i18nc("@info", "The date you entered is invalid"));
                setFocus();
            }
        } else {
            // skip setting the date, if it is empty and this is allowed
            if (!d->m_emptyDateAllowed || (date != QDate::fromJulianDay(1))) {
                d->setDate(date);
                if (!lineEdit()->text().isEmpty())
                    KDateComboBox::focusOutEvent(event);
            } else {
                // prevent KDateComboBox from showing an error
                QComboBox::focusOutEvent(event);
            }
        }
    } else {
        // prevent KDateComboBox from showing an error
        QComboBox::focusOutEvent(event);
    }
    d->m_lastKeyPressWasEscape = false;
}

void KMyMoneyDateEdit::focusInEvent(QFocusEvent* event)
{
    KDateComboBox::focusInEvent(event);

    const auto reason = event->reason();
    if (reason == Qt::TabFocusReason) {
        d->selectSection(d->m_sections.at(0));
    } else if (reason == Qt::BacktabFocusReason) {
        d->selectSection(d->m_sections.at(2));
    }
}

void KMyMoneyDateEdit::keyPressEvent(QKeyEvent* keyEvent)
{
    auto dropKey(false);
    const auto pos = lineEdit()->cursorPosition();
    const auto key = keyEvent->key();
    QDate date;
    d->m_lastKeyPressWasEscape = false;

    switch (key) {
    case Qt::Key_Down:
    case Qt::Key_Up:
        if (isValid()) {
            d->adjustDate(key == Qt::Key_Up ? 1 : -1, d->sectionByCursorPos());
        }
        break;

    case Qt::Key_Plus:
    case Qt::Key_Minus:
        if (isValid()) {
            d->adjustDate(key == Qt::Key_Plus ? 1 : -1, QDateEdit::NoSection);
        }
        break;

    case Qt::Key_PageDown:
        date = d->fixupDate();
        break;
    case Qt::Key_PageUp:
        date = d->fixupDate();
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        date = d->fixupDate();
        if (!date.isValid()) {
            KMessageBox::error(this, i18nc("@info", "The date you entered is invalid"));
            keyEvent->accept();
            setFocus();
        } else {
            d->setDate(date);
            if (!lineEdit()->text().isEmpty()) {
                QComboBox::keyPressEvent(keyEvent);
            }
        }
        break;

    case Qt::Key_Backspace:
        // make sure the user does not enter two delimiters in a row
        // by simply removing the duplicate as well
        if (d->isDelimiterAtPos(pos) && d->isDelimiterAtPos(pos - 2)) {
            QComboBox::keyPressEvent(keyEvent);
        }
        QComboBox::keyPressEvent(keyEvent);
        break;

    case Qt::Key_Delete:
        // make sure the user does not enter two delimiters in a row
        // by simply removing the duplicate as well
        if (d->isDelimiterAtPos(pos - 1) && d->isDelimiterAtPos(pos + 1)) {
            QComboBox::keyPressEvent(keyEvent);
        }
        QComboBox::keyPressEvent(keyEvent);
        break;

    case Qt::Key_T:
        d->setDate(QDate::currentDate());
        Q_EMIT dateValidityChanged(QDate::currentDate());
        d->selectSection(s_globalKMyMoneyDateEditSettings()->initialSection);
        break;

    default:
        if (keyEvent->text().length() > 0) {
            const auto ch(keyEvent->text().at(0));

            if (!(d->isValidDelimiter(ch) || (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')))) {
                dropKey = true;
            }

            // prevent two delimters in a row and simply
            // fake an overwrite and select the next section
            if (d->isValidDelimiter(ch)) {
                if (d->isDelimiterAtPos(pos)) {
                    const auto section(d->sectionByCursorPos());
                    const auto part(d->partBySection(section));
                    if (part < 2) {
                        d->selectSection(d->m_sections[part + 1]);
                    }
                    dropKey = true;
                }
            }
        }

        if (!dropKey) {
            const auto oldDate(d->fixupDate());
            const auto oldSection(d->sectionByCursorPos());
            QComboBox::keyPressEvent(keyEvent);
            const auto newDate(d->fixupDate());
            const auto newSection(d->sectionByCursorPos());

            if (newDate.isValid()) {
                if (!oldDate.isValid() || (oldDate.day() != newDate.day())) {
                    d->m_originalDay = newDate.day();
                }
                if (oldSection != newSection) {
                    d->selectSection(newSection);
                }
            }
        }
        break;

    case Qt::Key_Escape:
        d->m_lastKeyPressWasEscape = true;
        QComboBox::keyPressEvent(keyEvent);
        break;
    }
}

void KMyMoneyDateEdit::setDisplayFormat(QLocale::FormatType format)
{
    Q_UNUSED(format)
}

void KMyMoneyDateEdit::setAllowEmptyDate(bool emptyDateAllowed)
{
    d->m_emptyDateAllowed = emptyDateAllowed;
}
