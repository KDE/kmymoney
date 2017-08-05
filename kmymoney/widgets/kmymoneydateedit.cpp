/***************************************************************************
                          kmymoneydateedit.h
                          -------------------
    copyright            : (C) 2016 by Thomas Baumgart
    email                : tbaumgart@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneydateedit.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

namespace
{
const QDate INVALID_DATE = QDate(1800, 1, 1);
}


KMyMoneyDateEdit::KMyMoneyDateEdit(QWidget* parent)
  : QDateEdit(parent)
{
}
