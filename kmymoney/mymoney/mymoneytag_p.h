/***************************************************************************
                          mymoneytag.cpp
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <alessandro@russo.it>
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

#ifndef MYMONEYTAG_P_H
#define MYMONEYTAG_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>
#include <QString>
#include <QColor>

// ----------------------------------------------------------------------------
// Project Includes

namespace Tag
{
  enum class Attribute {
    Name = 0 ,
    Type,
    TagColor,
    Closed,
    Notes,
    // insert new entries above this line
    LastAttribute
  };
  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

class MyMoneyTagPrivate
{
public:

  MyMoneyTagPrivate() :
    m_closed(false)
  {
  }

  static QString getAttrName(const Tag::Attribute attr)
  {
    static const QHash<Tag::Attribute, QString> attrNames {
      {Tag::Attribute::Name,     QStringLiteral("name")},
      {Tag::Attribute::Type,     QStringLiteral("type")},
      {Tag::Attribute::TagColor, QStringLiteral("tagcolor")},
      {Tag::Attribute::Closed,   QStringLiteral("closed")},
      {Tag::Attribute::Notes,    QStringLiteral("notes")},
    };
    return attrNames[attr];
  }

  // Simple fields
  QString m_name;
  // Closed tags will not be shown in the selector inside a transaction, only in the Tag tab
  bool m_closed;
  // Set the color showed in the ledger
  QColor m_tag_color;
  QString m_notes;
};

#endif
