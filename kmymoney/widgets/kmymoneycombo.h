/*
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCOMBO_H
#define KMYMONEYCOMBO_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyCompletion;
class KMyMoneySelector;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyComboPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyCombo : public KComboBox
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCombo)
  Q_PROPERTY(QString selectedItem READ selectedItem WRITE setSelectedItem STORED false)

public:
  explicit KMyMoneyCombo(QWidget *parent = nullptr);
  explicit KMyMoneyCombo(bool rw, QWidget *parent = nullptr);
  virtual ~KMyMoneyCombo();

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
    * @return pointer to KMyMoneyCompletion or derivative.
    */
  KMyMoneyCompletion* completion() const;

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
  QString selectedItem() const;

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

  void setSuppressObjectCreation(bool suppress);

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt);
  void setCurrentText();

  /**
   * Overridden to support our own completion box
   */
  QSize sizeHint() const override;

protected Q_SLOTS:
  virtual void slotItemSelected(const QString& id);

protected:
  /**
    * reimplemented to support our own popup widget
    */
  void mousePressEvent(QMouseEvent *e) override;

  /**
    * reimplemented to support our own popup widget
    */
  void keyPressEvent(QKeyEvent *e) override;

  /**
    * reimplemented to support our own popup widget
    */
  void paintEvent(QPaintEvent *) override;

  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent*) override;

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentTextById(const QString& id);

  /**
    * Overridden for internal reasons, no API change
    */
  void connectNotify(const QMetaMethod & signal) override;

  /**
    * Overridden for internal reasons, no API change
    */
  void disconnectNotify(const QMetaMethod & signal) override;

protected:
  KMyMoneyComboPrivate * const d_ptr;
  KMyMoneyCombo(KMyMoneyComboPrivate &dd, bool rw = false, QWidget *parent = 0);

Q_SIGNALS:
  void itemSelected(const QString& id);
  void objectCreation(bool);
  void createItem(const QString&, QString&);

private:
  Q_DECLARE_PRIVATE(KMyMoneyCombo)
};

#endif
