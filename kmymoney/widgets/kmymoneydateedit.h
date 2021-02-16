/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
