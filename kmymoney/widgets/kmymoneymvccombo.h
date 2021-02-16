/*
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2010-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYMVCCOMBO_H
#define KMYMONEYMVCCOMBO_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes


/**
  * @author Cristian Onet
  * This class will replace the KMyMoneyCombo class when all widgets will use the MVC
  */
class KMyMoneyMVCComboPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyMVCCombo : public KComboBox
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyMVCCombo)
  Q_PROPERTY(QString selectedItem READ selectedItem WRITE setSelectedItem STORED false)

public:
  explicit KMyMoneyMVCCombo(QWidget* parent = nullptr);
  explicit KMyMoneyMVCCombo(bool editable, QWidget* parent = nullptr);
  virtual ~KMyMoneyMVCCombo();

  /**
    * @sa KLineEdit::setPlaceholderText()
    */
  void setPlaceholderText(const QString& hint) const;

  /**
    * This method returns the id of the first selected item.
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
    * Protect an entry from selection. Protection is controlled by
    * the parameter @p protect.
    *
    * @param id id of item for which to modify the protection
    * @param protect if true, the entry specified by @p accId cannot be
    *                selected. If false, it can be selected.
    *                Defaults to @p true.
    */
  void protectItem(int id, bool protect);

  /**
    * Make the completion match on any substring or only
    * from the start of an entry.
    *
    * @param enabled @a true turns on substring match, @a false turns it off.
    *                The default is @a false.
    */
  void setSubstringSearch(bool enabled);

  /**
    * set setSubstringSearch(enabled) of all children of widget
    *
    * @param widget a valid pointer (not 0)
    */
  static void setSubstringSearchForChildren(QWidget *const widget, bool enabled = false);

  /**
    * Reimplemented for internal reasons, no API change
    */
  void setEditable(bool editable);

protected Q_SLOTS:
  void activated(int index);

protected:
  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent*) override;

  /**
   * check if the current text is contained in the internal list, if not ask the user if want to create a new item.
   */
  virtual void checkCurrentText();

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

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt);
  void setCurrentText();

  void addEntry(const QString& newTxt, const QString& id);

Q_SIGNALS:
  void itemSelected(const QString& id);
  void objectCreation(bool);
  void createItem(const QString&, QString&);
  void lostFocus();

protected:
  KMyMoneyMVCComboPrivate * const d_ptr;
  KMyMoneyMVCCombo(KMyMoneyMVCComboPrivate &dd, QWidget* parent = nullptr);
  KMyMoneyMVCCombo(KMyMoneyMVCComboPrivate &dd, bool editable, QWidget* parent = nullptr);

private:
  Q_DECLARE_PRIVATE(KMyMoneyMVCCombo)
};

#endif
