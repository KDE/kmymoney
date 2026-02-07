/*
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez
    SPDX-FileCopyrightText: 2010 Cristian Oneț
    SPDX-FileCopyrightText: 2015 Christian Dávid
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

/**
 * Helper class for KReportView.
 *
 * This is a named list of reports, which will be one section
 * in the list of default reports
 *
 * @author Ace Jones
 */
class ReportGroup : public QList<MyMoneyReport>
{
private:
    QString m_name; ///< the title of the group in non-translated form
    QString m_title; ///< the title of the group in i18n-ed form
public:
    ReportGroup()
    {
    }
    ReportGroup(const QString& name, const QString& title)
        : m_name(name)
        , m_title(title)
    {
    }
    const QString& name() const
    {
        return m_name;
    }
    const QString& title() const
    {
        return m_title;
    }
};
