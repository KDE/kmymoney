/***************************************************************************
                          mymoneyobserver.h  -  description
                             -------------------
    begin                : Sat May 18 2002
    copyright            : (C) 2000-2005 by Michael Edwardes
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

#ifndef MYMONEYOBSERVER_H
#define MYMONEYOBSERVER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>
class MyMoneySubject;
class QString;

/**
  * This is the base class to be used to construct an
  * observer for usage in a subject/observer relationship
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT MyMoneyObserver {
public:
	virtual ~MyMoneyObserver();
  virtual void update(const QString& id) = 0;

protected:
	MyMoneyObserver();
};

#endif
