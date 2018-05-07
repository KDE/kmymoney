/***************************************************************************
                          mymoneytracer.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#ifndef MYMONEYTRACER_H
#define MYMONEYTRACER_H

#include "kmm_mymoney_export.h"

#include "qglobal.h"

#ifdef __GNUC__
#  define KMM_PRINTF_FORMAT(x, y) __attribute__((format(__printf__, x, y)))
#else
#  define KMM_PRINTF_FORMAT(x, y) /*NOTHING*/
#endif

class QString;

void timestamp(const char* txt);
void timestamp_reset();

class MyMoneyTracerPrivate;
class KMM_MYMONEY_EXPORT MyMoneyTracer
{
  Q_DISABLE_COPY(MyMoneyTracer)

public:
  explicit MyMoneyTracer(const char* prettyName);
#define MYMONEYTRACER(a) MyMoneyTracer a(Q_FUNC_INFO)

  explicit MyMoneyTracer(const QString& className, const QString& methodName);
  ~MyMoneyTracer();

  /**
    * This method allows to trace a printf like formatted text
    *
    * @param format format mask
    */
  void printf(const char *format, ...) const KMM_PRINTF_FORMAT(2, 3);

  static void off();
  static void on();
  static void onOff(int onOff);

private:
  MyMoneyTracerPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(MyMoneyTracer)
};

#endif
