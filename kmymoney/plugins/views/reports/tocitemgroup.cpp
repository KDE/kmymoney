/*
    copyright            : (C) Bernd Gonsior <bernd.gonsior@googlemail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
