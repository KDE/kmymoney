/***************************************************************************
                          kmymoneyedit.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
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

#ifndef KMYMONEYEDIT_H
#define KMYMONEYEDIT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDoubleValidator>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "kmm_widgets_export.h"

class QWidget;
class KLineEdit;
class MyMoneySecurity;

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT KMyMoneyMoneyValidator : public QDoubleValidator
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyMoneyValidator)

public:
  /**
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  explicit KMyMoneyMoneyValidator(QObject * parent);
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p digits after the decimal
    * point.
    */
  explicit KMyMoneyMoneyValidator(double bottom, double top, int decimals,
                                  QObject * parent);
  /**
    * Destructs the validator.
    */
  ~KMyMoneyMoneyValidator();

  /** Overloaded for internal reasons. The API is not affected. */
  QValidator::State validate(QString & input, int & pos) const override;
};

/**
  * This class represents a widget to enter monetary values.
  * It has an edit field and a button to select a popup
  * calculator. The result of the calculator (if used) is
  * stored in the edit field.
  *
  * @author Michael Edwardes, Thomas Baumgart
  */
class KMyMoneyEditPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyEdit : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyEdit)
  Q_PROPERTY(bool calculatorButtonVisibility READ isCalculatorButtonVisible WRITE setCalculatorButtonVisible)
  Q_PROPERTY(bool resetButtonVisibility READ isResetButtonVisible WRITE setResetButtonVisible)
  Q_PROPERTY(bool allowEmpty READ isEmptyAllowed WRITE setAllowEmpty)
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
  Q_PROPERTY(MyMoneyMoney value READ value WRITE setValue DESIGNABLE false STORED false USER true)
  Q_PROPERTY(bool valid READ isValid DESIGNABLE false STORED false)

public:
  explicit KMyMoneyEdit(QWidget* parent = nullptr, const int prec = -2);
  explicit KMyMoneyEdit(const MyMoneySecurity& eq, QWidget* parent = nullptr);
  ~KMyMoneyEdit();

  MyMoneyMoney value() const;

  void setValue(const MyMoneyMoney& value);

  bool isValid() const;

  virtual bool eventFilter(QObject *watched, QEvent *event) override;

  /**
    * This method returns the value of the edit field in "numerator/denominator" format.
    * If you want to get the text of the edit field, use lineedit()->text() instead.
    */
  QString text() const;
  void setMinimumWidth(int w);

  /**
    * Set the number of fractional digits that should be shown
    *
    * @param prec number of fractional digits.
    *
    * @note should be used prior to calling loadText()
    * @sa precision
    */
  void setPrecision(const int prec);

  /**
    * return the number of fractional digits
    * @sa setPrecision
    */
  int precision() const;

  QWidget* focusWidget() const;

  /**
    * This method allows to modify the behavior of the widget
    * such that it accepts an empty value (all blank) or not.
    * The default is to not accept an emtpy input and to
    * convert an empty field into 0.00 upon loss of focus.
    *
    * @param allowed if @a true, empty input is allowed, if @a false
    *                emtpy input will be converted to 0.00
    */
  void setAllowEmpty(bool allowed = true);

  /** Overloaded for internal reasons. The API is not affected. */
  void setValidator(const QValidator* v);

  bool isCalculatorButtonVisible() const;

  bool isResetButtonVisible() const;

  bool isEmptyAllowed() const;

  KLineEdit* lineedit() const;

  void setPlaceholderText(const QString& hint) const;

  bool isReadOnly() const;

  /**
   * This allows to setup the standard precision (number of decimal places)
   * to be used when no other information is available. @a prec must be in
   * the range of 0..19. If never set, the default precision is 2.
   */
  static void setStandardPrecision(int prec);

public slots:
  void loadText(const QString& text);
  void resetText();
  void clearText();

  void setText(const QString& txt);

  /**
    * This method allows to show/hide the calculator button of the widget.
    * The parameter @p show controls the behavior. Default is to show the
    * button.
    *
    * @param show if true, button is shown, if false it is hidden
    */
  void setCalculatorButtonVisible(const bool show);

  void setResetButtonVisible(const bool show);

  void setReadOnly(bool readOnly);

signals: // Signals
  /**
    * This signal is sent, when the focus leaves this widget and
    * the amount has been changed by user during this session.
    */
  void valueChanged(const QString& text);

  void textChanged(const QString& text);

protected slots:
  void theTextChanged(const QString & text);
  void slotCalculatorResult();
  void slotCalculatorOpen();

private:
  KMyMoneyEditPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyEdit)
};

#endif
