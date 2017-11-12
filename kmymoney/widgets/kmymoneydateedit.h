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

#ifndef KMYMONEYDATEEDIT_H
#define KMYMONEYDATEEDIT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateEdit>

// TODO: check if this class is really necessary
class KMyMoneyDateEdit : public QDateEdit
{
  Q_OBJECT

public:
  explicit KMyMoneyDateEdit(QWidget* parent = nullptr);

};
#endif // KMYMONEYDATEEDIT_H
