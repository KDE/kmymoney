/*
 * Copyright 2008       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KLOADTEMPLATEDLG_H
#define KLOADTEMPLATEDLG_H

#include "kmm_templates_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

template <typename T> class QList;

namespace Ui { class KLoadTemplateDlg; }

class MyMoneyTemplate;
class KLoadTemplateDlgPrivate;

/// This dialog lets the user load more account templates
class KMM_TEMPLATES_EXPORT KLoadTemplateDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KLoadTemplateDlg)
  Q_DECLARE_PRIVATE(KLoadTemplateDlg)

public:
  explicit KLoadTemplateDlg(QWidget *parent = nullptr);
  ~KLoadTemplateDlg();

  QList<MyMoneyTemplate> templates() const;

private Q_SLOTS:
  void slotHelp();

private:
  KLoadTemplateDlgPrivate * const d_ptr;
};

#endif
