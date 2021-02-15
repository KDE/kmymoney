/*
    SPDX-FileCopyrightText: 2006-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCATEGORY_H
#define KMYMONEYCATEGORY_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycombo.h"

class QPushButton;
class KMyMoneyAccountSelector;

/**
  * This class implements a text based account/category selector.
  * When initially used, the widget has the functionality of a KComboBox object.
  * Whenever a key is pressed, the set of loaded accounts is searched for
  * accounts which match the currently entered text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * an account is selected, the selection box is closed. Other key-strokes are
  * directed to the parent object to manipulate the text.  The visible contents of
  * the selection box is updated with every key-stroke.
  *
  * This object is a replacement of the KMyMoneyCategory object and should be used
  * for new code.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyCategoryPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyCategory : public KMyMoneyCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCategory)

public:
  /**
    * Standard constructor for the account selection object.
    *
    * If parameter @a splitButton is @a true, the widget
    * will construct a surrounding QFrame and reparent itself to be a child of this
    * QFrame. It also adds a QPushButton with the "Split" icon to the right of the
    * input field. In this case it is important not to use the pointer to this widget
    * but it's parent when placing the object in a QLayout, QTable or some such. The
    * parent widget (the QFrame in this case) can be extracted with the parentWidget()
    * method.
    *
    * Reparenting is handled by the object transparently for both cases.
    *
    * Standard usage example (no split button):
    *
    * @code
    * KMyMoneyCategory* category = new KMyMoneyCategory;
    * category->reparent(newParent);
    * layout->addWidget(category);
    * table->setCellWidget(category);
    * @endcode
    *
    * Enhanced usage example (with split button):
    *
    * @code
    * KMyMoneyCategory* category = new KMyMoneyCategory(0, true);
    * category->reparent(newParent);
    * layout->addWidget(category->parentWidget());
    * table->setCellWidget(category->parentWidget());
    * @endcode
    */
  explicit KMyMoneyCategory(bool splitButton = false, QWidget* parent = nullptr);
  ~KMyMoneyCategory() override;

  /**
    * This member returns a pointer to the completion object.
    *
    * @return pointer to completion's selector object
    */
  KMyMoneyAccountSelector* selector() const;

  /**
    * This member returns a pointer to the split button. In case the @a splitButton parameter
    * of the constructor was @a false, this method prints a warning to stderr and returns 0.
    */
  QPushButton* splitButton() const;

  /**
    * Reimplemented for internal reasons. No API change
    */
  virtual void reparent(QWidget *parent, Qt::WindowFlags, const QPoint &, bool showIt = false);

  /**
    * Reimplemented for internal reasons. No API change.
    */
  virtual void setPalette(const QPalette& palette);

  /**
    * Force the text field to show the text for split transaction.
    */
  void setSplitTransaction();

  /**
    * Check if the text field contains the text for a split transaction
    */
  bool isSplitTransaction() const;

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt);
  void setCurrentText();

  bool eventFilter(QObject *o, QEvent *ev) override;

protected:
  /**
    * Reimplemented to support protected category text ("split transactions")
    *
    * @sa focusIn()
    */
  void focusInEvent(QFocusEvent* ev) override;

  /**
    * Reimplemented to support protected category text ("split transactions")
    */
  void focusOutEvent(QFocusEvent* ev) override;

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  void setCurrentTextById(const QString& id)  override;

public Q_SLOTS:
  void slotItemSelected(const QString& id) override;

Q_SIGNALS:
  /**
    * Signal to inform other objects that this object has reached focus.
    * Used for e.g. to open the split dialog when the focus reaches this
    * object and it contains the text 'Split transaction'.
    *
    * @sa focusInEvent()
    */
  void focusIn();

private:
  Q_DECLARE_PRIVATE(KMyMoneyCategory)
};


class KMM_BASE_WIDGETS_EXPORT KMyMoneySecurity : public KMyMoneyCategory
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneySecurity)

public:
  explicit KMyMoneySecurity(QWidget* parent = nullptr);
  ~KMyMoneySecurity() override;

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt);
  void setCurrentText();

protected:
  /**
    * set the widgets text area based on the item with the given @a id.
    */
  void setCurrentTextById(const QString& id) override;
};

#endif
