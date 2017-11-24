/***************************************************************************
                          mymoneytag.h
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>
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

#ifndef MYMONEYTAG_H
#define MYMONEYTAG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneyobject.h"

class QString;
class QColor;
class QDomDocument;

/**
  * This class represents a tag within the MyMoney engine.
  */
class MyMoneyTagPrivate;
class KMM_MYMONEY_EXPORT MyMoneyTag : public MyMoneyObject
{
  Q_DECLARE_PRIVATE(MyMoneyTag)
  MyMoneyTagPrivate* d_ptr;

  KMM_MYMONEY_UNIT_TESTABLE

  public:
    MyMoneyTag();

  explicit MyMoneyTag(const QString& name);

  explicit MyMoneyTag(const QString& name,
                      const QColor& tagColor
                      );
  /**
    * This is the constructor for a tag that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  explicit MyMoneyTag(const QDomElement& node);

  MyMoneyTag(const QString& id,
             const MyMoneyTag& tag);

  MyMoneyTag(const MyMoneyTag & other);
  MyMoneyTag(MyMoneyTag && other);
  MyMoneyTag & operator=(MyMoneyTag other);
  friend void swap(MyMoneyTag& first, MyMoneyTag& second);

  ~MyMoneyTag();

  QString name() const;
  void setName(const QString& val);

  bool isClosed() const;
  void setClosed(bool val);

  QColor tagColor() const;
  void setTagColor(const QColor& val);

  QString notes() const;
  void setNotes(const QString& val);

  // Equality operator
  bool operator == (const MyMoneyTag &) const;
  bool operator <(const MyMoneyTag& right) const;

  void writeXML(QDomDocument& document, QDomElement& parent) const override;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  bool hasReferenceTo(const QString& id) const override;

  static MyMoneyTag null;
};

inline void swap(MyMoneyTag& first, MyMoneyTag& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.m_id, second.m_id);
}

inline MyMoneyTag::MyMoneyTag(MyMoneyTag && other) : MyMoneyTag() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyTag & MyMoneyTag::operator=(MyMoneyTag other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

//inline bool operator==(const MyMoneyTag& lhs, const QString& rhs)
//{
//  return lhs.id() == rhs;
//}

/**
  * Make it possible to hold @ref MyMoneyTag objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTag)

#endif
