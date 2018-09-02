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
