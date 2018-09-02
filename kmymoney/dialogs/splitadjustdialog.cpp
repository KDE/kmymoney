/*
 * Copyright 2016-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "splitadjustdialog.h"
#include "ui_splitadjustdialog.h"

#include <QButtonGroup>

#include <KLocalizedString>

class SplitAdjustDialog::Private {
public:
  Private()
    : ui(new Ui_SplitAdjustDialog),
      buttonGroup(nullptr)
    {}

  ~Private()
  {
    delete ui;
  }
  Ui_SplitAdjustDialog*	ui;
  QButtonGroup* buttonGroup;
};

SplitAdjustDialog::SplitAdjustDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , d(new Private)
{
  d->ui->setupUi(this);
  d->buttonGroup = new QButtonGroup(this);
  d->buttonGroup->addButton(d->ui->continueBtn, SplitAdjustContinue);
  d->buttonGroup->addButton(d->ui->changeBtn, SplitAdjustChange);
  d->buttonGroup->addButton(d->ui->distributeBtn, SplitAdjustDistribute);
  d->buttonGroup->addButton(d->ui->leaveBtn, SplitAdjustLeaveAsIs);
}

SplitAdjustDialog::~SplitAdjustDialog()
{
}

SplitAdjustDialog::Options SplitAdjustDialog::selectedOption() const
{
  return static_cast<Options>(d->buttonGroup->checkedId());
}

void SplitAdjustDialog::setValues(QString transactionSum, QString splitSum, QString diff, int splitCount)
{
  // now modify the text items of the dialog to contain the correct values
  QString q = i18n("The total amount of this transaction is %1 while "
                    "the sum of the splits is %2. The remaining %3 are "
                    "unassigned.", transactionSum, splitSum, diff);
  d->ui->explanation->setText(q);

  q = i18n("Change &total amount of transaction to %1.", splitSum);
  d->ui->changeBtn->setText(q);

  q = i18n("&Distribute difference of %1 among all splits.", diff);
  d->ui->distributeBtn->setText(q);
  // FIXME remove the following line once distribution among
  //       all splits is implemented
  d->ui->distributeBtn->hide();

  // if we have only two splits left, we don't allow leaving sth. unassigned.
  if (splitCount < 3) {
    q = i18n("&Leave total amount of transaction at %1.", transactionSum);
  } else {
    q = i18n("&Leave %1 unassigned.", diff);
  }
  d->ui->leaveBtn->setText(q);
}
