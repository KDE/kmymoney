/***************************************************************************
                          kmymoneychecklistitem  -  description
                             -------------------
    begin                : Wed Jun 28 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYCHECKLISTITEM_H
#define KMYMONEYCHECKLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <q3listview.h>
#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyListViewItem;

/**
  * This class implements a derived version of a QCheckListItem that
  * allows the storage of an engine object id with the object and emits
  * a signal upon state change.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyCheckListItem : public QObject, public Q3CheckListItem
{
  friend class KMyMoneyListViewItem;

  Q_OBJECT
public:
  KMyMoneyCheckListItem(Q3ListView *parent, const QString& txt, const QString& key, const QString& id, Type type = Q3CheckListItem::CheckBox);
  KMyMoneyCheckListItem(Q3ListView *parent, Q3ListViewItem* after, const QString& txt, const QString& key, const QString& id, Type type = Q3CheckListItem::CheckBox);
  KMyMoneyCheckListItem(Q3ListViewItem *parent, const QString& txt, const QString& key, const QString& id, Type type = Q3CheckListItem::CheckBox);
  ~KMyMoneyCheckListItem();

  const QString& id(void) const {
    return m_id;
  };

  /**
    * use my own paint method
    */
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

  /**
    * use my own backgroundColor method
    */
  const QColor backgroundColor();

  /**
    * see K3ListViewItem::isAlternate()
    */
  bool isAlternate(void);

  /**
    * This method returns a const reference to the key passed to the constructor. The column
    * defines what is returned: For @a column equals 0, the first character passed as @a key to
    * the constructor concatenated with the value returned by text(0) is returned. For @a column
    * equals to 1, the @a key as passed to the constructor except the first character is returned.
    */
  QString key(int column, bool ascending) const;

signals:
  void stateChanged(bool);

protected:
  virtual void stateChange(bool);

private:
  QString              m_key;
  QString              m_id;
  // copied from K3ListViewItem()
  unsigned int         m_isOdd : 1;
  unsigned int         m_isKnown : 1;
  unsigned int         m_unused : 30;
};

#endif
