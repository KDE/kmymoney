/***************************************************************************
                          tocitemgroup.h  -  description
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
#ifndef TOCITEMGROUP_H
#define TOCITEMGROUP_H

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "tocitem.h"

/**
 * Class for reportgroup items in reports table of contents (TOC).
 */
class TocItemGroup : public TocItem
{
public:

  /** Constructor.
   *
   * @param parent pointer to the parent QWidget
   * @param groupNo group number
   * @param title group title in i18n-form
   */
  TocItemGroup(QTreeWidget* parent, int groupNo, QString title);
};

#endif
