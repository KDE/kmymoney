/***************************************************************************
                          kmymoneycombo.h  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
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

#ifndef KMYMONEYCOMBO_H
#define KMYMONEYCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMutex>
#include <QPaintEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

class kMyMoneyCompletion;
class KMyMoneySelector;
class kMyMoneyLineEdit;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyCombo : public KComboBox
{
  Q_OBJECT
  Q_PROPERTY(QString selectedItem READ selectedItem WRITE setSelectedItem STORED false)
public:
  KMyMoneyCombo(QWidget *w = 0);
  explicit KMyMoneyCombo(bool rw, QWidget *w = 0);

  /**
    * This method is used to turn on/off the hint display and to setup the appropriate text.
    * The hint text is shown in a lighter color if the field is otherwise empty and does
    * not have the keyboard focus.
    *
    * @param hint reference to text. If @a hint is empty, no hint will be shown.
    */
  void setPlaceholderText(const QString& hint) const;

  /**
    * overridden for internal reasons.
    *
    * @param editable make combo box editable (@a true) or selectable only (@a false).
    */
  void setEditable(bool editable);

  /**
    * This method returns a pointer to the completion object of the combo box.
    *
    * @return pointer to kMyMoneyCompletion or derivative.
    */
  kMyMoneyCompletion* completion() const;

  /**
    * This method returns a pointer to the completion object's selector.
    *
    * @return pointer to KMyMoneySelector or derivative.
    */
  KMyMoneySelector* selector() const;

  /**
    * This method returns the ids of the currently selected items
    */
  void selectedItems(QStringList& list) const;

  /**
    * This method returns the id of the first selected item.
    * Usage makes usually only sense when the selection mode
    * of the associated KMyMoneySelector is QListView::Single.
    *
    * @sa KMyMoneySelector::setSelectionMode()
    *
    * @return reference to QString containing the id. If no item
    *         is selected the QString will be empty.
    */
  const QString& selectedItem() const {
    return m_id;
  }

  /**
    * This method selects the item with the respective @a id.
    *
    * @param id reference to QString containing the id
    */
  void setSelectedItem(const QString& id);

  /**
    * This method checks if the position @a pos is part of the
    * area of the drop down arrow.
    */
  bool isInArrowArea(const QPoint& pos) const;

  void setSuppressObjectCreation(bool suppress) {
    m_canCreateObjects = !suppress;
  }

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) {
    KComboBox::setItemText(KComboBox::currentIndex(), txt);
  }

  /**
   * Overridden to support our own completion box
   */
  QSize sizeHint() const;

protected slots:
  virtual void slotItemSelected(const QString& id);

protected:
  /**
    * reimplemented to support our own popup widget
    */
  void mousePressEvent(QMouseEvent *e);

  /**
    * reimplemented to support our own popup widget
    */
  void keyPressEvent(QKeyEvent *e);

  /**
    * reimplemented to support our own popup widget
    */
  void paintEvent(QPaintEvent *);

  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent*);

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentTextById(const QString& id);

  /**
    * Overridden for internal reasons, no API change
    */
  void connectNotify(const QMetaMethod & signal);

  /**
    * Overridden for internal reasons, no API change
    */
  void disconnectNotify(const QMetaMethod & signal);

protected:
  /**
    * This member keeps a pointer to the object's completion object
    */
  kMyMoneyCompletion*    m_completion;

  /**
    * Use our own line edit to provide hint functionality
    */
  kMyMoneyLineEdit*      m_edit;

  /**
    * The currently selected item
    */
  QString                m_id;

signals:
  void itemSelected(const QString& id);
  void objectCreation(bool);
  void createItem(const QString&, QString&);

private:
  QTimer                 m_timer;
  QMutex                 m_focusMutex;
  /**
    * Flag to control object creation. Use setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool                   m_canCreateObjects;

  /**
    * Flag to check whether a focusOutEvent processing is underway or not
    */
  bool                   m_inFocusOutEvent;
};




#endif
