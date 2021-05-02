/*
    SPDX-FileCopyrightText: Bernd Gonsior <bernd.gonsior@googlemail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TOCITEMREPORT_H
#define TOCITEMREPORT_H

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "tocitem.h"
#include "mymoneyreport.h"

/**
 * Class for report items in reports table of contents (TOC).
 */
class TocItemReport : public TocItem
{
private:

    /** Reference to the MyMoneyReport object. */
    MyMoneyReport m_report;

public:

    /** Constructor.
     *
     * @param parent pointer to the parent QWidget
     * @param report reference to the report associated with this TOC-entry
     */
    TocItemReport(QTreeWidgetItem* parent, MyMoneyReport& report);

    /** Returns the report associated with this TOC-entry. */
    MyMoneyReport& getReport();
};

#endif
