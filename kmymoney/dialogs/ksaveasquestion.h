/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
