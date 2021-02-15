/*
 * SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
