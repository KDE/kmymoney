/***************************************************************************
                          kmymoneyedit.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>

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

#include <QValidator>
#include <QEvent>
#include <QKeyEvent>
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
class QPushButton;



// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneylineedit.h>
#include <mymoneysecurity.h>
#include "kmm_widgets_export.h"

class MyMoneyMoney;
class kMyMoneyCalculator;

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT kMyMoneyMoneyValidator : public QDoubleValidator
{
  Q_OBJECT

public:
  /**
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever QDoubleValidator uses for that) and parent @p
    * parent
    */
  kMyMoneyMoneyValidator(QObject * parent);
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p digits after the decimal
    * point.
    */
  kMyMoneyMoneyValidator(double bottom, double top, int decimals,
                         QObject * parent);
  /**
    * Destructs the validator.
    */
  virtual ~kMyMoneyMoneyValidator() {}

  /** Overloaded for internal reasons. The API is not affected. */
  virtual QValidator::State validate(QString & input, int & pos) const;
};

/**
  * This class represents a widget to enter monetary values.
  * It has an edit field and a button to select a popup
  * calculator. The result of the calculator (if used) is
  * stored in the edit field.
  *
  * @author Michael Edwardes, Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT kMyMoneyEdit : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool calculatorButtonVisibility READ isCalculatorButtonVisible WRITE setCalculatorButtonVisible)
  Q_PROPERTY(bool resetButtonVisibility READ isResetButtonVisible WRITE setResetButtonVisible)
  Q_PROPERTY(bool allowEmpty READ isEmptyAllowed WRITE setAllowEmpty)
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
  Q_PROPERTY(MyMoneyMoney value READ value WRITE setValue DESIGNABLE false STORED false USER true)
  Q_PROPERTY(bool valid READ isValid DESIGNABLE false STORED false)

private:
  QString previousText; // keep track of what has been typed
  QString m_text;       // keep track of what was the original value
  kMyMoneyCalculator* m_calculator;
  QWidget*            m_calculatorFrame;
  kMyMoneyLineEdit*   m_edit;
  QPushButton*        m_calcButton;
  QPushButton*        m_resetButton;
  int                 m_prec;
  bool                allowEmpty;

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

protected slots:
  void theTextChanged(const QString & text);
  void slotCalculatorResult();
  void slotCalculatorOpen();

public:
  explicit kMyMoneyEdit(QWidget *parent = 0, const int prec = -2);
  explicit kMyMoneyEdit(const MyMoneySecurity& eq, QWidget *parent = 0);
  ~kMyMoneyEdit();

  MyMoneyMoney value() const;

  void setValue(const MyMoneyMoney& value);

  bool isValid() const;

  virtual bool eventFilter(QObject * , QEvent *);

  /**
    * This method returns the value of the edit field in "numerator/denominator" format.
    * If you want to get the text of the edit field, use lineedit()->text() instead.
    */
  QString text() const {
    return value().toString();
  };

  void setMinimumWidth(int w) {
    m_edit->setMinimumWidth(w);
  };

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
  int precision() {
    return m_prec;
  };

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

  void setText(const QString& txt) {
    setValue(MyMoneyMoney(txt));
  };

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
};

#endif
