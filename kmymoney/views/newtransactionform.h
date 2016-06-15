/***************************************************************************
                          newtransactionform.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NEWTRANSACTIONFORM_H
#define NEWTRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QModelIndex;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class NewTransactionForm : public QFrame
{
  Q_OBJECT
public:
  explicit NewTransactionForm(QWidget* parent = 0);
  virtual ~NewTransactionForm();

public Q_SLOTS:
  void showTransaction(const QString& transactionSplitId);
  void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
  class Private;
  Private * const d;
};

#endif // NEWTRANSACTIONFORM_H

