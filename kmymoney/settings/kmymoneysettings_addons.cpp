/*
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QFontDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>
#include <QGuiApplication>

// ----------------------------------------------------------------------------
// Project Includes

QFont KMyMoneySettings::listCellFontEx()
{
    if (useSystemFont()) {
        return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    } else {
        return listCellFont();
    }
}

QFont KMyMoneySettings::listHeaderFontEx()
{
    if (useSystemFont()) {
        QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        font.setBold(true);
        return font;
    } else {
        return listHeaderFont();
    }
}

QColor KMyMoneySettings::schemeColor(const SchemeColor color)
{
    switch(color) {
    case SchemeColor::ListBackground1:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Window); //  background, KColorScheme::View, KColorScheme::NormalBackground
    case SchemeColor::ListBackground2:
        return QGuiApplication::palette().color(QPalette::Active,
                                                QPalette::AlternateBase); //  background, KColorScheme::View, KColorScheme::AlternateBackground
    case SchemeColor::ListGrid:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::PlaceholderText); //  foreground, KColorScheme::View, KColorScheme::InactiveText
    case SchemeColor::ListHighlightText:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::HighlightedText); //  foreground, KColorScheme::Selection, KColorScheme::NormalText
    case SchemeColor::ListHighlight:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Highlight); //  background, KColorScheme::Selection, KColorScheme::NormalBackground
    case SchemeColor::WindowText:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Text); //  foreground, KColorScheme::Window, KColorScheme::NormalText
    case SchemeColor::WindowBackground:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Background); //  background, KColorScheme::Window, KColorScheme::NormalBackground
    case SchemeColor::Positive:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Text); //  foreground, KColorScheme::View, KColorScheme::PositiveText
    case SchemeColor::Negative:
        return QGuiApplication::palette().color(QPalette::Active, QPalette::Text); //  foreground, KColorScheme::View, KColorScheme::NegativeText
    case SchemeColor::TransactionImported:
        if (useCustomColors())
            return transactionImportedColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Link); //  background, KColorScheme::View, KColorScheme::LinkBackground
    case SchemeColor::TransactionMatched:
        if (useCustomColors())
            return transactionMatchedColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Link); //  background, KColorScheme::View, KColorScheme::LinkBackground
    case SchemeColor::TransactionErroneous:
        if (useCustomColors())
            return transactionErroneousColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Text); //  foreground, KColorScheme::View, KColorScheme::NegativeText
    case SchemeColor::FieldRequired:
        if (useCustomColors())
            return fieldRequiredColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Background); //  background, KColorScheme::View, KColorScheme::NeutralBackground
    case SchemeColor::GroupMarker:
        if (useCustomColors())
            return groupMarkerColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Link); //  background, KColorScheme::Selection, KColorScheme::LinkBackground
    case SchemeColor::MissingConversionRate:
        if (useCustomColors())
            return missingConversionRateColor();
        else
            return QGuiApplication::palette().color(QPalette::Active, QPalette::Link); //  foreground, KColorScheme::Complementary, KColorScheme::LinkText
    default:
        return QColor();

    }
}

QStringList KMyMoneySettings::listOfItems()
{
    bool prevValue = self()->useDefaults(true);
    QStringList all = itemList().split(',', QString::SkipEmptyParts);
    self()->useDefaults(prevValue);
    QStringList list = itemList().split(',', QString::SkipEmptyParts);

    // now add all from 'all' that are missing in 'list'
    QRegExp exp("-?(\\d+)");
    QStringList::iterator it_s;
    for (it_s = all.begin(); it_s != all.end(); ++it_s) {
        if ((exp.indexIn(*it_s) != -1) && !list.contains(exp.cap(1)) && !list.contains(QString("-%1").arg(exp.cap(1)))) {
            list << *it_s;
        }
    }
    return list;
}

int KMyMoneySettings::firstFiscalMonth()
{
    return fiscalYearBegin() + 1;
}

int KMyMoneySettings::firstFiscalDay()
{
    return fiscalYearBeginDay();
}

QDate KMyMoneySettings::firstFiscalDate()
{
    QDate date = QDate(QDate::currentDate().year(), firstFiscalMonth(), firstFiscalDay());
    if (date > QDate::currentDate())
        date = date.addYears(-1);
    return date;
}

