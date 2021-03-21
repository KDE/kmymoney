/*
    SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERSEARCHLINE_H
#define REGISTERSEARCHLINE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyRegister {
class Register;
class RegisterItem;
}

namespace KMyMoneyRegister
{

/**
  * This class makes it easy to add a search line for filtering the items
  * in a register based on simple text.  Inspired by the idea of the kdelibs
  * class KListViewSearchLine.
  *
  * @author Thomas Baumgart
  */
class RegisterSearchLine : public KLineEdit
{
  Q_OBJECT
public:
  /**
    * Constructs a RegisterSearchLine with @a reg being the register to be
    * filtered.
    *
    * If @a reg is null then the widget will be disabled until a register
    * is set with setRegister().
    */
  explicit RegisterSearchLine(QWidget* parent = nullptr, Register* reg = 0);

  /**
    * Destroys the object
    */
  ~RegisterSearchLine();

  /**
    * Sets the KMyMoneyRegister that is filtered by this search line.
    * If @a reg is null then the widget will be disabled.
    *
    * @see KMyMoneyRegister()
    */
  void setRegister(Register* reg);

public Q_SLOTS:
  virtual void updateSearch(const QString& s = QString());

protected Q_SLOTS:
  void queueSearch(const QString& search);
  void activateSearch();
  void slotStatusChanged(int);

private Q_SLOTS:
  void itemAdded(RegisterItem* item) const;
  void registerDestroyed();

private:
  void init(Register* reg);

private:
  class RegisterSearchLinePrivate;
  RegisterSearchLinePrivate* const d;
};

/**
  * Creates a widget containing a RegisterSearchLine, a label with the text
  * "Search" and a button to clear the search. Modelled after KListViewSearchLineWidget.
  *
  * @author Thomas Baumgart
  */
class RegisterSearchLineWidget : public QWidget
{
  Q_OBJECT
public:
  /**
    * Creates a RegisterSearchLineWidget for @a reg with @a parent as the
    * parent and with @a name.
    */
  explicit RegisterSearchLineWidget(Register* reg = 0, QWidget* parent = nullptr);

  /**
    * Destroys the object
    */
  ~RegisterSearchLineWidget();

  /**
    * Returns a pointer to the searchline
    */
  RegisterSearchLine* searchLine() const;

  /**
    * Creates the search line.  This can be useful to reimplement in cases where
    * a RegisterSearchLine subclass is used.
    */
  virtual RegisterSearchLine* createSearchLine(Register* reg);

protected Q_SLOTS:
  /**
    * Creates the widgets inside of the widget.  This is called from the
    * constructor via a single shot timer so that it it guaranteed to run
    * after construction is complete.  This makes it suitable for overriding in
    * subclasses.
    */
  virtual void createWidgets();

private:
  class RegisterSearchLineWidgetPrivate;
  RegisterSearchLineWidgetPrivate* const d;
};

} // namespace

#endif
