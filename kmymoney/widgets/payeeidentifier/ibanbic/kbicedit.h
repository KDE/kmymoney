/*
 * SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KBICEDIT_H
#define KBICEDIT_H

#include <QValidator>
#include <KLineEdit>

#include "kmm_base_widgets_export.h"

class QAbstractItemDelegate;

class KMM_BASE_WIDGETS_EXPORT KBicEdit : public KLineEdit
{
  Q_OBJECT

public:
  explicit KBicEdit(QWidget* parent = 0);
  virtual ~KBicEdit();

private:
  QAbstractItemDelegate* m_popupDelegate;
};

#endif // KBICEDIT_H
