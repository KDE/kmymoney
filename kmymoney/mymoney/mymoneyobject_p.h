/***************************************************************************
                          mymoneyobject_p.h
                          -------------------
    copyright            : (C) 2005 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#ifndef MYMONEYOBJECT_P_H
#define MYMONEYOBJECT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDomElement>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObjectPrivate
{
public:
  MyMoneyObjectPrivate()
  {
  }

  virtual ~MyMoneyObjectPrivate()
  {
  }

  void setId(const QString& id)
  {
    m_id = id;
  }

  /**
   * This method writes out the members contained in this object.
   */
  void writeBaseXML(QDomDocument& document, QDomElement& el) const
  {
    Q_UNUSED(document);

    el.setAttribute(QStringLiteral("id"), m_id);
  }

  QString m_id;
};

#endif
