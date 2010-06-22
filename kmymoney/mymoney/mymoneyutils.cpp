/***************************************************************************
                          mymoneyutils.cpp  -  description
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

#include "mymoneyutils.h"

#include <iostream>

#include "mymoneyaccount.h"

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <QRegExp>

#ifdef _CHECK_MEMORY

#undef new
#undef _CheckMemory_Leak
#undef _CheckMemory_FreeAll

_CheckMemory chkmem;
bool enable = false;

_CheckMemoryEntry::_CheckMemoryEntry(void *p, int line, size_t size, const char *file)
    : m_p(p), m_line(line), m_size(size), m_file(file)
{
}

_CheckMemoryEntry::_CheckMemoryEntry()
    : m_p(0), m_line(0), m_size(0)
{
}


/////////////////////////////////////////////////////////////////////////////////////////
_CheckMemory::_CheckMemory()
{
  outfunc = (_CheckMemoryOutFunc *)0;
}

_CheckMemory::_CheckMemory(_CheckMemoryOutFunc *out)
{
  outfunc = out;
}

_CheckMemory::~_CheckMemory()
{
}

_CheckMemoryOutFunc *_CheckMemory::SetOutFunc(_CheckMemoryOutFunc *out)
{
  _CheckMemoryOutFunc *old;
  old = outfunc;
  outfunc = out;
  return old;
}

void _CheckMemory::Output(const char *fmt, ...)
{
  va_list args;
  char buf[128];
  va_start(args, fmt);
  if (outfunc) {
    vsprintf(buf, fmt, args);
    outfunc(buf);
  } else {
    vfprintf(stderr, fmt, args);
    putc('\n', stderr);
  }
  va_end(args);
}

int _CheckMemory::TableCount(void)
{
  return table.size();
}

bool _CheckMemory::CheckMemoryLeak(bool freeall)
{
  bool d = false;
  size_t total = 0;
  int freec = 0;
  CheckMemoryTable::ConstIterator it;

  for (it = table.begin(); it != table.end(); ++it) {
    if ((*it).pointer() != 0) {
      total += (*it).size();
      freec++;
      if (d == false) {
        Output("CheckMemory++: CheckMemoryLeak: Memory leak detected!");
        Output("Position  |Size(bytes)  |Allocated at");
        d = true;
      }
      if (d == true)
        Output("%p |%-13d|%s:%d", (*it).pointer(), (int)(*it).size(), (*it).file(), (*it).line());
    }
  }
  if (d == true)
    Output("You have forgotten to free %d object(s), %d bytes of memory.", freec, (int)total);
  else
    Output("CheckMemory++: CheckMemoryLeak: No memory leak detected.");
  if (freeall == true)
    FreeAll();
  return true;
}

void _CheckMemory::FreeAll()
{
  size_t total = 0;
  int freec = 0;
  CheckMemoryTable::Iterator it;

  for (it = table.begin(); it != table.end(); it = table.begin()) {
    if ((*it).pointer() != 0) {
      total += (*it).size();
      freec++;
      Output("CheckMemory++: FreeAll: freed %d bytes of memory at %p.", (int)(*it).size(), (*it).pointer());
      free((*it).pointer());
    }
    table.remove(it);
  }
  Output("CheckMemory++: FreeAll: Totally freed %d objects, %d bytes of memory.", freec, (int)total);
}

void _CheckMemory_Init(_CheckMemoryOutFunc *out)
{
  if (enable != true) {
    chkmem.Restart();
    chkmem.SetOutFunc(out);
    enable = true;
  }
}

void _CheckMemory_End()
{
  if (enable != false) {
    chkmem.Restart();
    chkmem.SetOutFunc(0);
    enable = false;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void *operator new(size_t s, const char *file, int line) throw()
{
  void *p = malloc(s);

  if (p == 0) throw;
  if (enable == true) {
    _CheckMemoryEntry entry(p, line, s, file);
    chkmem.table[p] = entry;
  }
  return p;
}

void * operator new [](size_t s, const char *file, int line) throw()
{
  void *p = malloc(s);

  if (p == 0) throw;
  if (enable == true) {
    _CheckMemoryEntry entry(p, line, s, file);
    chkmem.table[p] = entry;
  }
  return p;
}

void operator delete(void *p) throw()
{
  if (enable == true) {
    CheckMemoryTable::Iterator it;
    it = chkmem.table.find(p);
    if (it != chkmem.table.end()) {
      chkmem.table.remove(it);
    }
  }
  free(p);
}

void operator delete [](void *p) throw()
{
  if (enable == true) {
    CheckMemoryTable::Iterator it;
    it = chkmem.table.find(p);
    if (it != chkmem.table.end()) {
      chkmem.table.remove(it);
    }
  }
  free(p);
}

#endif // _CHECK_MEMORY

QString MyMoneyUtils::getFileExtension(QString strFileName)
{
  QString strTemp;
  if (!strFileName.isEmpty()) {
    //find last . delminator
    int nLoc = strFileName.lastIndexOf('.');
    if (nLoc != -1) {
      strTemp = strFileName.right(strFileName.length() - (nLoc + 1));
      return strTemp.toUpper();
    }
  }
  return strTemp;
}

int MyMoneyTracer::m_indentLevel = 0;
int MyMoneyTracer::m_onoff = 0;

MyMoneyTracer::MyMoneyTracer(const char* name)
{
  if (m_onoff) {
    QRegExp exp("(.*)::(.*)");
    if (exp.indexIn(name) != -1) {
      m_className = exp.cap(1);
      m_memberName = exp.cap(2);
    } else {
      m_className = QString(name);
      m_memberName.clear();
    }
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
  m_indentLevel += 2;
}

MyMoneyTracer::MyMoneyTracer(const QString& className, const QString& memberName) :
    m_className(className),
    m_memberName(memberName)
{
  if (m_onoff) {
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
  m_indentLevel += 2;
}

MyMoneyTracer::~MyMoneyTracer()
{
  m_indentLevel -= 2;
  if (m_onoff) {
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "LEAVE: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
}

void MyMoneyTracer::printf(const char *format, ...) const
{
  if (m_onoff) {
    va_list args;
    va_start(args, format);
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent);

    vfprintf(stderr, format, args);
    putc('\n', stderr);
    va_end(args);
  }
}

void MyMoneyTracer::onOff(int onOff)
{
  m_onoff = onOff;
}

void MyMoneyTracer::on(void)
{
  m_onoff = 1;
}

void MyMoneyTracer::off(void)
{
  m_onoff = 0;
}

QString dateToString(const QDate& date)
{
  if (!date.isNull() && date.isValid())
    return date.toString(Qt::ISODate);

  return QString();
}

QDate stringToDate(const QString& str)
{
  if (str.length()) {
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (!date.isNull() && date.isValid())
      return date;
  }
  return QDate();
}

QString QStringEmpty(const QString& val)
{
  if (!val.isEmpty())
    return QString(val);

  return QString();
}

unsigned long extractId(const QString& txt)
{
  int pos;
  unsigned long rc = 0;

  pos = txt.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    rc = txt.mid(pos).toInt();
  }
  return rc;
}

