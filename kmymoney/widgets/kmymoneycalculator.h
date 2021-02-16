/*
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCALCULATOR_H
#define KMYMONEYCALCULATOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

/**
  *@author Thomas Baumgart
  */

/**
  * This class implements a simple electronic calculator with the
  * ability of addition, subtraction, multiplication and division
  * and percentage calculation. Memory locations are not available.
  *
  * The first operand and operation can be loaded from an external
  * source to switch from an edit-widget to the calculator widget
  * without having the user to re-type the data. See setInitialValues()
  * for details.
  */
class KMyMoneyCalculatorPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyCalculator : public QFrame
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCalculator)

public:
  explicit KMyMoneyCalculator(QWidget* parent = nullptr);
  ~KMyMoneyCalculator();

  /**
    * This methods is used to extract the result of the last
    * calculation. The fractional part is separated from the
    * integral part by the character setup using setComma().
    *
    * @return QString representing the result of the
    *         last operation
    */
  QString result() const;

  /**
    * This method is used to set the character to be used
    * as the separator between the integer and fractional part
    * of an operand. Upon creation of the object, m_comma is
    * set to the current locale setting of KDE's decimalSymbol.
    *
    * @param ch QChar representing the character to be used
    */
  void setComma(const QChar ch);

  /**
    * This method is used to preset the first operand and start
    * execution of an operation. This method is currently used
    * by AmountEdit. If @p ev is 0, then no operation will be
    * started.
    *
    * @param value reference to QString representing the operands value
    * @param ev    pointer to QKeyEvent representing the operation's key
    */
  void setInitialValues(const QString& value, QKeyEvent* ev);

Q_SIGNALS:
  /**
    * This signal is emitted, when a new result is available
    */
  void signalResultAvailable();

  /**
    * This signal is emitted, when the user pressed the ESC key
    */
  void signalQuit();

protected:
  void keyPressEvent(QKeyEvent* ev) override;

  /**
    * This method is used to transform a double into a QString
    * and removing any trailing 0's and decimal separators
    *
    * @param val reference to double value to be converted
    * @return QString object containing the converted value
    */
  QString normalizeString(const double& val);

protected Q_SLOTS:
  /**
    * This method appends the digit represented by the parameter
    * to the current operand
    *
    * @param button integer value of the digit to be added in the
    *               range [0..9]
    */
  void digitClicked(int button);

  /**
    * This methods starts the operation contained in the parameter
    *
    * @param button The Qt::Keycode for the button pressed or clicked
    */
  void calculationClicked(int button);

  /**
    * This method appends a period (comma) to initialize the fractional
    * part of an operand. The period is only appended once.
    */
  void commaClicked();

  /**
    * This method reverses the sign of the current operand
    */
  void plusminusClicked();

  /**
    * This method clears the current operand
    */
  void clearClicked();

  /**
    * This method clears all registers
    */
  void clearAllClicked();

  /**
    * This method executes the percent operation
    */
  void percentClicked();

  /**
    * This method updates the display of the calculator with
    * the text passed as argument
    *
    * @param str reference to QString containing the new display contents
    */
  void changeDisplay(const QString& str);

private:
  KMyMoneyCalculatorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyCalculator)
};

#endif
