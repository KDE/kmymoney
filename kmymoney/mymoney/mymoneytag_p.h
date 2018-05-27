/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYMONEYTAG_P_H
#define MYMONEYTAG_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>
#include <QString>
#include <QColor>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"

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

class MyMoneyTagPrivate : public MyMoneyObjectPrivate
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
