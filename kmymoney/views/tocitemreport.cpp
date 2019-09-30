/***************************************************************************
                          tocitemreport.cpp  -  description
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

#include "tocitemreport.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include <KGlobal>
#include <KLocale>

// ----------------------------------------------------------------------------
// Project Includes

TocItemReport::TocItemReport(QTreeWidgetItem* parent, MyMoneyReport& report):
    TocItem(parent, QStringList() << report.name() << report.comment())
{
  QDate startDate,endDate;
  if (report.dateFilter(startDate, endDate)) {
    setText(2, KGlobal::locale()->formatDate(startDate, KLocale::ShortDate));
    setText(3, KGlobal::locale()->formatDate(endDate, KLocale::ShortDate));
  }
  m_report = report;

  type = TocItem::REPORT;

  QString tocTyp = QString::number(type);
  QString id = report.name();

  QStringList key;
  key << tocTyp << id;

  QVariant data(key);
  this->setData(0, Qt::UserRole, data);
}

MyMoneyReport& TocItemReport::getReport()
{
  return m_report;
}
