/***************************************************************************
                          mymoneyutils.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MYMONEYUTILS_H_
#define _MYMONEYUTILS_H_

#ifdef HAVE_CONFIG_H
#include "config-kmymoney.h"
#endif

#include <QString>
#include <QDateTime>
#include <q3valuelist.h>
#include <kmm_mymoney_export.h>
#if 0

//Includes for STL support below
#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

#else

//typedef for data type to store currency with.
typedef long long DLONG;

typedef QString String;
#endif // 0

void timetrace(const char *);
void timestamp(const char *);

//class that has utility functions to use throughout the application.
class MyMoneyUtils
{
public:
  MyMoneyUtils() {}
  ~MyMoneyUtils() {}

  //static function to add the correct file extension at the end of the file name
  static QString getFileExtension(QString strFileName);

};

class KMM_MYMONEY_EXPORT MyMoneyTracer
{
public:
  MyMoneyTracer(const char* prettyName);
#define MYMONEYTRACER(a) MyMoneyTracer a(Q_FUNC_INFO)

  MyMoneyTracer(const QString& className, const QString& methodName);
  ~MyMoneyTracer();

  /**
    * This method allows to trace a printf like formatted text
    *
    * @param format format mask
    */
  void printf(const char *format, ...) const __attribute__ ((format (__printf__, 2, 3)));

  static void off(void);
  static void on(void);
  static void onOff(int onOff);

private:
  QString m_className;
  QString m_memberName;

  static int m_indentLevel;
  static int m_onoff;
};

#ifdef _CHECK_MEMORY

#include <cstddef>
#include <qmap.h>

class _CheckMemoryEntry {
public:
  _CheckMemoryEntry();
  _CheckMemoryEntry(void *p, int line, size_t size, const char *file);
  ~_CheckMemoryEntry() {}

  void * pointer(void) const { return m_p; };
  int line(void) const { return m_line; };
  size_t size(void) const { return m_size; };
  const char* file(void) const { return m_file; };

private:
  void *m_p;
  int m_line;
  size_t m_size;
  QString m_file;
};

typedef QMap<void *, _CheckMemoryEntry> CheckMemoryTable;

typedef void _CheckMemoryOutFunc(const char *);

class KMM_MYMONEY_EXPORT _CheckMemory {
 public:
  _CheckMemory();
  _CheckMemory(_CheckMemoryOutFunc *out);
  virtual ~_CheckMemory();

  _CheckMemoryOutFunc *SetOutFunc(_CheckMemoryOutFunc *out);
  bool CheckMemoryLeak(bool freeall);
  void FreeAll();
  inline void Restart() { table.clear(); };

  int TableCount(void);

 private:
  void Output(const char *fmt,...) __attribute__ ((format (__printf__, 2, 3)));

  CheckMemoryTable table;
  _CheckMemoryOutFunc *outfunc;

  friend void * operator new(size_t s,const char *file,int line) throw();
  friend void * operator new [] (size_t,const char *file,int line) throw();
  friend void operator delete(void *p) throw();
  friend void operator delete [] (void *p) throw();
};

KMM_MYMONEY_EXPORT void * operator new(size_t s,const char *file,int line) throw(); // Normal new operator
KMM_MYMONEY_EXPORT void * operator new [] (size_t s,const char *file,int line) throw(); // Array new operator
KMM_MYMONEY_EXPORT void operator delete(void *p) throw();
KMM_MYMONEY_EXPORT void operator delete [] (void *p) throw();

#define new new(__FILE__,__LINE__)

KMM_MYMONEY_EXPORT extern _CheckMemory chkmem;

KMM_MYMONEY_EXPORT void _CheckMemory_Init(_CheckMemoryOutFunc *out);
KMM_MYMONEY_EXPORT void _CheckMemory_End();
#define _CheckMemory_Leak(freeall) chkmem.CheckMemoryLeak(freeall)
#define _CheckMemory_FreeAll() chkmem.FreeAll()

#endif // _CHECK_MEMORY

/**
 * This function returns a date in the form specified by Qt::ISODate.
 * If the @p date is invalid an empty string will be returned.
 *
 * @param date const reference to date to be converted
 * @return QString containing the converted date
 */
KMM_MYMONEY_EXPORT QString dateToString(const QDate& date);

/**
 * This function returns a date as QDate object as specified by
 * the parameter @p str. @p str must be in Qt::ISODate format.
 * If @p str is empty or contains an invalid date, QDate() is
 * returned.
 *
 * @param str date in Qt::ISODate format
 * @return QDate object
 */
KMM_MYMONEY_EXPORT QDate stringToDate(const QString& str);

KMM_MYMONEY_EXPORT QString QStringEmpty(const QString&);

KMM_MYMONEY_EXPORT unsigned long extractId(const QString& txt);

#endif
