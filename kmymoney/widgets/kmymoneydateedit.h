/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYDATEEDIT_H
#define KMYMONEYDATEEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateEdit>

// TODO: check if this class is really necessary
class KMM_BASE_WIDGETS_EXPORT KMyMoneyDateEdit : public QDateEdit
{
  Q_OBJECT

public:
  explicit KMyMoneyDateEdit(QWidget* parent = nullptr);

};
#endif // KMYMONEYDATEEDIT_H
