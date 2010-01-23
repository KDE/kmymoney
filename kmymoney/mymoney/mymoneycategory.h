/***************************************************************************
                          mymoneycategory.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYCATEGORY_H
#define MYMONEYCATEGORY_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QStringList>

/**
  * @deprecated This class represents an Income or Expense category. Please don't
  *             use it anymore, as it will be removed sooner or later.
  */
class MyMoneyCategory
{
  bool m_income; // if false, m_income == expense
  QString m_name;
  QStringList m_minorCategories;

  friend QDataStream &operator<<(QDataStream &, MyMoneyCategory &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyCategory &);

public:
  MyMoneyCategory();
  MyMoneyCategory(const bool income, const QString name);
  MyMoneyCategory(const bool income, const QString name, QStringList minors);
  ~MyMoneyCategory();

  // Simple get operations
  QString name(void) {
    return m_name;
  }
  QStringList& minorCategories(void) {
    return m_minorCategories;
  }

  // Simple set operations
  bool isIncome(void) {
    return m_income;
  }
  void setIncome(const bool val) {
    m_income = val;
  }
  void setName(const QString val) {
    m_name = val;
  }

  bool setMinorCategories(QStringList values);
  bool addMinorCategory(const QString val);
  bool removeMinorCategory(const QString val);
  bool renameMinorCategory(const QString oldVal, const QString newVal);
  bool addMinorCategory(QStringList values);
  bool removeAllMinors(void);
  QString firstMinor(void);

  void clear(void);

  // Copy constructors
  MyMoneyCategory(const MyMoneyCategory&);
  MyMoneyCategory& operator = (const MyMoneyCategory&);
};

#endif
