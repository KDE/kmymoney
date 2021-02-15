/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIBANLINEEDIT_H
#define KIBANLINEEDIT_H

#include "kmm_widgets_export.h"

#include <KLineEdit>

class ibanValidator;

class KMM_WIDGETS_EXPORT KIbanLineEdit : public KLineEdit
{
  Q_OBJECT

public:
  explicit KIbanLineEdit(QWidget* parent);
  const ibanValidator* validator() const;
};

#endif // KIBANLINEEDIT_H
