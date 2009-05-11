/***************************************************************************
                          kbalancewarning.h
                             -------------------
    begin                : Mon Feb  9 2009
    copyright            : (C) 2009 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBALANCEWARNING_H
#define KBALANCEWARNING_H
// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
class QString;
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

class KBalanceWarning : public QObject
{
  Q_OBJECT
public:
  KBalanceWarning(QObject* parent, const char* name = 0);
  virtual ~KBalanceWarning();

public slots:
  void slotShowMessage(QWidget* parent, const MyMoneyAccount& account, const QString& msg);

private:
  class Private;
  Private* d;
};

#endif
