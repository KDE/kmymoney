/***************************************************************************
                               amountedit.h
                             -------------------
    copyright            : (C) 2016 by Thomas Baumgart <tbaumgart@kde.org>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMOUNTEDIT_H
#define AMOUNTEDIT_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QValidator>
#include <QLineEdit>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysecurity.h"
#include "mymoneymoney.h"

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT AmountValidator : public QDoubleValidator
{
  Q_OBJECT

public:
  /**
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  AmountValidator(QObject * parent);
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p digits after the decimal
    * point.
    */
  AmountValidator(double bottom, double top, int decimals,
                         QObject * parent);
  /**
    * Destructs the validator.
    */
  virtual ~AmountValidator() {}

  /** Overloaded for internal reasons. The API is not affected. */
  virtual QValidator::State validate(QString & input, int & pos) const;
};

/**
  * This class represents a widget to enter monetary values.
  * It has an edit field and a button to select a popup
  * calculator. The result of the calculator (if used) is
  * stored in the edit field.
  *
  * @author Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT AmountEdit : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(bool calculatorButtonVisibility READ isCalculatorButtonVisible WRITE setCalculatorButtonVisible)
  Q_PROPERTY(bool allowEmpty READ isEmptyAllowed WRITE setAllowEmpty)
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
  Q_PROPERTY(MyMoneyMoney value READ value WRITE setValue DESIGNABLE false STORED false USER true)
  Q_PROPERTY(bool valid READ isValid DESIGNABLE false STORED false)

private:
  class Private;
  QScopedPointer<Private> d;

  /**
   * This holds the number of precision to be used
   * when no other information (e.g. from account)
   * is available.
   *
   * @sa setStandardPrecision()
   */
  static int standardPrecision;

private:
  /**
    * Internal helper function for value() and ensureFractionalPart().
    */
  void ensureFractionalPart(QString& txt) const;

protected:
  /**
    * This method ensures that the text version contains a
    * fractional part.
    */
  void ensureFractionalPart();

  /**
    * This method opens the calculator and replays the key
    * event pointed to by @p ev. If @p ev is 0, then no key
    * event is replayed.
    *
    * @param ev pointer to QKeyEvent that started the calculator.
    */
  void calculatorOpen(QKeyEvent* ev);

  /**
    * Helper method for constructors.
    */
  void init();

  /**
   * Overridden to support calculator button.
   */
  virtual void resizeEvent(QResizeEvent* event);

  /**
   * Overridden to support ensureFractionalPart().
   */
  virtual void focusOutEvent(QFocusEvent* event);

  /**
   * Overridden to support calculator button.
   */
  virtual void keyPressEvent(QKeyEvent* event);

protected Q_SLOTS:
  void theTextChanged(const QString & text);
  void slotCalculatorResult();
  void slotCalculatorOpen();
  void slotCalculatorClose();

public:
  explicit AmountEdit(QWidget *parent = 0, const int prec = -2);
  explicit AmountEdit(const MyMoneySecurity& eq, QWidget *parent = 0);
  virtual ~AmountEdit();

  MyMoneyMoney value() const;

  void setValue(const MyMoneyMoney& value);

  bool isValid() const;

  /**
    * This method returns the value of the edit field in "numerator/denominator" format.
    * If you want to get the text of the edit field, use lineedit()->text() instead.
    */
  QString numericalText() const
  {
    return value().toString();
  }

  /**
    * Set the number of fractional digits that should be shown
    *
    * @param prec number of fractional digits.
    *
    * @note should be used prior to calling setText()
    * @sa precision
    */
  void setPrecision(const int prec);

  /**
    * return the number of fractional digits
    * @sa setPrecision
    */
  int precision() const;

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

  bool isEmptyAllowed() const;

  bool isCalculatorButtonVisible() const;

  /**
   * This allows to setup the standard precision (number of decimal places)
   * to be used when no other information is available. @a prec must be in
   * the range of 0..19. If never set, the default precision is 2.
   */
  static void setStandardPrecision(int prec);

public Q_SLOTS:
  void resetText();

  void setText(const QString& txt);

  /**
    * This method allows to show/hide the calculator button of the widget.
    * The parameter @p show controls the behavior. Default is to show the
    * button.
    *
    * @param show if true, button is shown, if false it is hidden
    */
  void setCalculatorButtonVisible(const bool show);

Q_SIGNALS:
  /**
    * This signal is sent, when the focus leaves this widget and
    * the amount has been changed by user during this focus possesion.
    */
  void valueChanged(const QString& text);

  /**
   * This signal is emitted when the contents of the widget
   * changed and was validated. Use this in favor of textChanged()
   * in your application.
   */
  void validatedTextChanged(const QString& text);
};


class KMM_WIDGETS_EXPORT CreditDebitHelper : public QObject
{
  Q_OBJECT
public:
  explicit CreditDebitHelper(QObject* parent, AmountEdit* credit, AmountEdit* debit);
  virtual ~CreditDebitHelper();

  /**
   * Retruns the value of the widget that is filled.
   * A credit is retruned as negative, a debit as positive value.
   */
  MyMoneyMoney value() const;

  /**
   * Loads the widgets with the @a value passed. If
   * @a value is negative it is loaded into the credit
   * widget, otherwise into the debit widget.
   */
  void setValue(const MyMoneyMoney& value);

  /**
   * This method returns true if at least one
   * of the two widgets is filled with text.
   * It returns false if both widgets are empty.
   */
  bool haveValue() const;

Q_SIGNALS:
  void valueChanged();

private Q_SLOTS:
  void creditChanged();
  void debitChanged();

private:
  void widgetChanged(AmountEdit* src, AmountEdit* dst);

private:
  QPointer<AmountEdit>  m_credit;
  QPointer<AmountEdit>  m_debit;
};

#endif
