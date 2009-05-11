/***************************************************************************
                          reporttable.h
                             -------------------
    begin                : Mon May  7 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REPORTTABLE_H
#define REPORTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


namespace reports {

class KReportChartView;

/**
  * This class serves as interface definition for both a pivottable
  * and a querytable object
  */
class ReportTable
{
protected:
    ReportTable() {}
public:
    virtual ~ReportTable() {}
    virtual QString renderHTML(void) const = 0;
    virtual QString renderCSV(void) const = 0;
    virtual void drawChart(KReportChartView& view) const = 0;
    virtual void dump(const QString& file, const QString& context=QString()) const = 0;
};

}
#endif
// REPORTTABLE_H
// vim:cin:si:ai:et:ts=2:sw=2:
