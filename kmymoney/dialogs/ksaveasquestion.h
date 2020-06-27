/*
 * Copyright 2018       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KSAVEASQUESTION_H
#define KSAVEASQUESTION_H

#include "kmm_base_dialogs_export.h"

#include <QDialog>

namespace Ui { class KSaveAsQuestion; }
namespace eKMyMoney { enum class StorageType; }

class KMM_BASE_DIALOGS_EXPORT KSaveAsQuestion : public QDialog
{
  Q_DISABLE_COPY(KSaveAsQuestion)

public:
  explicit KSaveAsQuestion(QVector<eKMyMoney::StorageType> filetypes, QWidget* parent = nullptr);
  ~KSaveAsQuestion();
  eKMyMoney::StorageType fileType() const;

private:
  Ui::KSaveAsQuestion * const ui;
};

#endif
