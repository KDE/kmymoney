/***************************************************************************
                          kmymoneymvccombo.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYMVCCOMBO_H
#define KMYMONEYMVCCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QCompleter>
/*#include <QTimer>
#include <QMutex>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QList>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStandardItemModel>*/

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneypayee.h>

/*#include <mymoneyutils.h>
#include <mymoneysplit.h>
#include <register.h>
#include <mymoneyaccount.h>
#include <transaction.h>

#include <mymoneytransactionfilter.h>
#include <mymoneyscheduled.h>

class kMyMoneyCompletion;
class KMyMoneySelector;
class kMyMoneyLineEdit;*/

/**
  * @author Cristian Onet
  * This class will replace the KMyMoneyCombo class when all widgets will use the MVC
  */
class KMyMoneyMVCCombo : public KComboBox
{
  Q_OBJECT

public:
  KMyMoneyMVCCombo(QWidget* parent = 0);
  explicit KMyMoneyMVCCombo(bool editable, QWidget* parent = 0);

  void setHint(const QString& hint) const;

  /**
    * This method returns the id of the first selected item.
    *
    * @return reference to QString containing the id. If no item
    *         is selected the QString will be empty.
    */
  const QString& selectedItem(void) const;

  /**
    * This method selects the item with the respective @a id.
    *
    * @param id reference to QString containing the id
    */
  void setSelectedItem(const QString& id);

protected slots:
  void activated(int index);

protected:
  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent* );

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentTextById(const QString& id);

  /**
    * Overridden for internal reasons, no API change
    */
  void connectNotify(const char* signal);

  /**
    * Overridden for internal reasons, no API change
    */
  void disconnectNotify(const char* signal);

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) { KComboBox::setItemText(KComboBox::currentIndex(), txt); }

signals:
  void itemSelected(const QString& id);
  void objectCreation(bool);
  void createItem(const QString&, QString&);

private:
  /**
    * This is just a cache to be able to implement the old interface.
    */
  mutable QString m_id;

  /**
    * Flag to control object creation. Use setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool            m_canCreateObjects;

  /**
    * Flag to check whether a focusOutEvent processing is underway or not
    */
  bool            m_inFocusOutEvent;
};

/**
  * This class implements a text based payee selector.
  * When initially used, the widget has the functionality of a KComboBox object.
  * Whenever a key is pressed, the set of loaded payees is searched for
  * payees names which match the currently entered text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * a payee is selected, the selection box is closed. Other key-strokes are
  * directed to the parent object to manipulate the text.  The visible contents of
  * the selection box is updated with every key-stroke.
  *
  * This object is a replacement of the kMyMoneyPayee object and should be used
  * for new code.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyPayeeCombo : public KMyMoneyMVCCombo

{
   Q_OBJECT
public:
  KMyMoneyPayeeCombo(QWidget* parent = 0);

  void loadPayees(const QList<MyMoneyPayee>& list);
};


#endif
