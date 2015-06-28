/***************************************************************************
                          tocitemreport.h  -  description
                             -------------------
    begin                : Sat Jul 03 2010
    copyright            : (C) Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
