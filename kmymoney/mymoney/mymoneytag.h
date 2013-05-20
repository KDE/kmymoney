/***************************************************************************
                          mymoneytag.h
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>

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

#include <QString>
#include <QColor>
#include <QMetaType>
class QStringList;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmm_mymoney_export.h>
#include <mymoneyobject.h>

/**
  * This class represents a tag within the MyMoney engine.
  */
class KMM_MYMONEY_EXPORT MyMoneyTag : public MyMoneyObject
{
private:
  // Simple fields
  QString m_name;
  // Closed tags will not be shown in the selector inside a transaction, only in the Tag tab
  bool m_closed;
  // Set the color showed in the ledger
  QColor m_tag_color;
  QString m_notes;

public:
  MyMoneyTag();
  MyMoneyTag(const QString& id, const MyMoneyTag& tag);
  explicit MyMoneyTag(const QString& name,
                      const QColor& tagColor = QColor()
                     );
  /**
    * This is the constructor for a tag that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  MyMoneyTag(const QDomElement& el);

  ~MyMoneyTag();

  // Simple get operations
  const QString& name(void) const            {
    return m_name;
  }
  bool isClosed(void) const {
    return m_closed;
  }
  const QColor& tagColor(void) const         {
    return m_tag_color;
  }
  const QString& notes(void) const           {
    return m_notes;
  }

  // Simple set operations
  void setName(const QString& val)      {
    m_name = val;
  }
  void setTagColor(const QColor& val)      {
    m_tag_color = val;
  }
  void setClosed(bool val) {
    m_closed = val;
  }
  void setNotes(const QString& val)     {
    m_notes = val;
  };

  // Copy constructors
  MyMoneyTag(const MyMoneyTag&);

  // Equality operator
  bool operator == (const MyMoneyTag &) const;
  bool operator <(const MyMoneyTag& right) const;

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

  static MyMoneyTag null;
};

inline bool operator==(const MyMoneyTag& lhs, const QString& rhs)
{
  return lhs.id() == rhs;
}

/**
  * Make it possible to hold @ref MyMoneyTag objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTag)

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
