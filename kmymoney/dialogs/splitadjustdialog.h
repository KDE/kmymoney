/***************************************************************************
                          splitadjustdialog.h
                             -------------------
    begin                : Sat Jul 23 2016
    copyright            : (C) 2016 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
