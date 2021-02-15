/*
    SPDX-FileCopyrightText: 2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
