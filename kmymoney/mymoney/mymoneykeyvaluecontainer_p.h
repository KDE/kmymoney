/***************************************************************************
                          mymoneykeyvaluecontainer.cpp
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYKEYVALUECONTAINER_P_H
#define MYMONEYKEYVALUECONTAINER_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KVC
{
  enum class Element { Pair };

  uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  enum class Attribute { Key,
                         Value,
                         // insert new entries above this line
                         LastAttribute
                       };

  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

using namespace KVC;

class MyMoneyKeyValueContainerPrivate
{
public:

  static QString getElName(const Element el)
  {
    static const QMap<Element, QString> elNames {
      {Element::Pair, QStringLiteral("PAIR")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Attribute attr)
  {
    static const QMap<Attribute, QString> attrNames {
      {Attribute::Key, QStringLiteral("key")},
      {Attribute::Value, QStringLiteral("value")}
    };
    return attrNames[attr];
  }

  /**
    * This member variable represents the container of key/value pairs.
    */
  QMap<QString, QString>  m_kvp;
};
#endif
