/*
    copyright            : (C) Bernd Gonsior <bernd.gonsior@googlemail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
