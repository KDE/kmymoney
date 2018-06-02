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

#ifndef SPLITADJUSTDIALOG_H
#define SPLITADJUSTDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QScopedPointer>

class SplitAdjustDialog : public QDialog
{
  Q_OBJECT
public:
  explicit SplitAdjustDialog(QWidget* parent, Qt::WindowFlags f = 0);
  virtual ~SplitAdjustDialog();

  void setValues(QString transactionSum, QString splitSum, QString diff, int splitCount);

  enum Options {
    SplitAdjustContinue,
    SplitAdjustChange,
    SplitAdjustDistribute,
    SplitAdjustLeaveAsIs
  };

  Options selectedOption() const;
  
private:
  class Private;
  QScopedPointer<Private> d;
};

#endif // SPLITADJUSTDIALOG_H
