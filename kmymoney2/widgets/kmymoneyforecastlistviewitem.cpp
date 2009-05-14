/***************************************************************************
                          kmymoneylistviewitem  -  description
                             -------------------
    begin                : Sun Nov 25 2007
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <qpalette.h>
#include <qpen.h>
#include <qcolor.h>
#include <qpainter.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyforecastlistviewitem.h"

#include <kmymoneyglobalsettings.h>

KMyMoneyForecastListViewItem::KMyMoneyForecastListViewItem (Q3ListView* parent, Q3ListViewItem* after, bool isNegative) :
  K3ListViewItem(parent, after),
  m_negative(isNegative)
{
}

KMyMoneyForecastListViewItem::~KMyMoneyForecastListViewItem()
{
}

void KMyMoneyForecastListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  QColor textColour;
  if(m_negative == true) {
    textColour = KMyMoneyGlobalSettings::listNegativeValueColor(); //if the item is marked is marked as negative, all columns will be painted negative
  } else {
    textColour = m_columnsColor[column]; //otherwise, respect the color for each column
  }
  _cg.setColor(QColorGroup::Text, textColour); 
  
  K3ListViewItem::paintCell(p, _cg, column, width, alignment);
}

void KMyMoneyForecastListViewItem::setNegative(bool isNegative)
{
  m_negative = isNegative;
}

void KMyMoneyForecastListViewItem::setText( int column, const QString &text, const bool &negative)
{
  //if negative set the map to negative color according to KMyMoneySettings
  if(negative) {
    m_columnsColor[column] = KMyMoneyGlobalSettings::listNegativeValueColor();
  } else {
    m_columnsColor[column] = QColorGroup::Text;
  }
  
  K3ListViewItem::setText(column, text);
}
