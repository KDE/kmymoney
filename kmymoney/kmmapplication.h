/***************************************************************************
                          kmmapplication.h
                             -------------------
    copyright            : (C) 2020 by Dawid Wróbel <me@dawidwrobel.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMMAPPLICATION_H
#define KMMAPPLICATION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QSplashScreen>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"

class KMMApplication : public QApplication
{
public:
  KMMApplication(int &argc, char **argv, KMyMoney *kmymoney);
  int run(std::unique_ptr<QSplashScreen> splash, const QUrl & file, bool noFile);

protected:
  bool event(QEvent * event);
  void openFile(std::unique_ptr<QSplashScreen> &splash, const QUrl &file, bool noFile) const;
};

#endif //KMMAPPLICATION_H
