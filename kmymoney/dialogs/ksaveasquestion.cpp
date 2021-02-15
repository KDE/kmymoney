/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksaveasquestion.h"
#include "kmymoneyenums.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksaveasquestion.h"

KSaveAsQuestion::KSaveAsQuestion(QVector<eKMyMoney::StorageType> fileTypes, QWidget* parent) :
  QDialog(parent),
  ui(new Ui::KSaveAsQuestion)
{
  ui->setupUi(this);
  for (const auto& fileType : fileTypes) {
    switch (fileType) {
      case eKMyMoney::StorageType::XML:
        ui->fileType->addItem(i18n("XML"), static_cast<int>(fileType));
        break;
      case eKMyMoney::StorageType::SQL:
        ui->fileType->addItem(i18n("SQL"), static_cast<int>(fileType));

        break;
      default:
        break;
    }
  }
  const auto ixXML = ui->fileType->findData(static_cast<int>(eKMyMoney::StorageType::XML));
  ui->fileType->setCurrentIndex(ixXML != -1 ? ixXML : 0);
}

KSaveAsQuestion::~KSaveAsQuestion()
{
  delete ui;
}

eKMyMoney::StorageType KSaveAsQuestion::fileType() const
{
  return static_cast<eKMyMoney::StorageType>(ui->fileType->currentData().toInt());
}
