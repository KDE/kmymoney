/***************************************************************************
                          mymoneysubject.h  -  description
                             -------------------
    begin                : Sat May 18 2002
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

#ifndef MYMONEYSUBJECT_H
#define MYMONEYSUBJECT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <q3ptrlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

class MyMoneyObserver;
class QString;

/**
  * This is the base class to be used to construct a
  * subject for usage in a subject/observer relationship
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT MyMoneySubject {
public:
  virtual ~MyMoneySubject();
  virtual void attach(MyMoneyObserver*);
  virtual void detach(MyMoneyObserver*);
  virtual void notify(const QString& id);

protected:
  MyMoneySubject();

private:
  Q3PtrList<MyMoneyObserver> m_observers;
};

#endif
