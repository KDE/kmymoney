/***************************************************************************
                          tocitemgroup.cpp  -  description
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

#include "tocitemgroup.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

TocItemGroup::TocItemGroup(QTreeWidget* parent, int groupNo, QString title):
    TocItem(parent, QStringList()
            << ((QString().setNum(groupNo)).append(". ").append(title)))
{
  type = TocItem::GROUP;

  QString tocTyp = QString::number(type);
  QString id = QString::number(groupNo).rightJustified(3, '0');

  QStringList key;
  key << tocTyp << id;

  //set bold font
  QFont font = this->font(0);
  font.setBold(true);
  this->setFont(0, font);

  QVariant data(key);
  this->setData(0, Qt::UserRole, data);
}
