/***************************************************************************
                          tocitemreport.cpp  -  description
                             -------------------
    begin                : Sat Jul 03 2010
    copyright            : (C) Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "tocitemreport.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

TocItemReport::TocItemReport(QTreeWidgetItem* parent, MyMoneyReport& report):
    TocItem(parent, QStringList() << report.name() << report.comment())
{
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
