/***************************************************************************
                          kmymoneycalculator.cpp  -  description
                             -------------------
    begin                : Sat Oct 19 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QSignalMapper>
#include <QRegExp>
#include <QGridLayout>
#include <QFrame>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycalculator.h"

kMyMoneyCalculator::kMyMoneyCalculator(QWidget* parent)
  : QFrame(parent)
{
  m_comma = KGlobal::locale()->monetaryDecimalSymbol()[0];
  m_clearOperandOnDigit = false;

  QGridLayout* grid = new QGridLayout(this);

  display = new QLabel(this);
  QPalette palette;
  palette.setColor(display->backgroundRole(), QColor("#BDFFB4"));
  display->setPalette(palette);

  display->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  grid->addWidget(display, 0, 0, 1, 5);

  buttons[0] = new KPushButton("0", this);
  buttons[1] = new KPushButton("1", this);
  buttons[2] = new KPushButton("2", this);
  buttons[3] = new KPushButton("3", this);
  buttons[4] = new KPushButton("4", this);
  buttons[5] = new KPushButton("5", this);
  buttons[6] = new KPushButton("6", this);
  buttons[7] = new KPushButton("7", this);
  buttons[8] = new KPushButton("8", this);
  buttons[9] = new KPushButton("9", this);
  buttons[PLUS] = new KPushButton("+", this);
  buttons[MINUS] = new KPushButton("-", this);
  buttons[STAR] = new KPushButton("X", this);
  buttons[COMMA] = new KPushButton(m_comma, this);
  buttons[EQUAL] = new KPushButton("=", this);
  buttons[SLASH] = new KPushButton("/", this);
  buttons[CLEAR] = new KPushButton("C", this);
  buttons[CLEARALL] = new KPushButton("AC", this);
  buttons[PLUSMINUS] = new KPushButton("+-", this);
  buttons[PERCENT] = new KPushButton("%", this);

  grid->addWidget(buttons[7], 1, 0);
  grid->addWidget(buttons[8], 1, 1);
  grid->addWidget(buttons[9], 1, 2);
  grid->addWidget(buttons[4], 2, 0);
  grid->addWidget(buttons[5], 2, 1);
  grid->addWidget(buttons[6], 2, 2);
  grid->addWidget(buttons[1], 3, 0);
  grid->addWidget(buttons[2], 3, 1);
  grid->addWidget(buttons[3], 3, 2);
  grid->addWidget(buttons[0], 4, 1);

  grid->addWidget(buttons[COMMA], 4, 0);
  grid->addWidget(buttons[PLUS], 3, 3);
  grid->addWidget(buttons[MINUS], 4, 3);
  grid->addWidget(buttons[STAR], 3, 4);
  grid->addWidget(buttons[SLASH], 4, 4);
  grid->addWidget(buttons[EQUAL], 4, 2);
  grid->addWidget(buttons[PLUSMINUS], 2, 3);
  grid->addWidget(buttons[PERCENT], 2, 4);
  grid->addWidget(buttons[CLEAR], 1, 3);
  grid->addWidget(buttons[CLEARALL], 1, 4);

  buttons[EQUAL]->setFocus();

  op1 = 0.0;
  stackedOp = op = 0;
  operand = QString();
  changeDisplay("0");

  // connect the digit signals through a signal mapper
  QSignalMapper* mapper = new QSignalMapper(this);
  for(int i = 0; i < 10; ++i) {
    mapper->setMapping(buttons[i], i);
    connect(buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
  }
  connect(mapper, SIGNAL(mapped(int)), this, SLOT(digitClicked(int)));

  // connect the calculation operations through another mapper
  mapper = new QSignalMapper(this);
  for(int i = PLUS; i <= EQUAL; ++i) {
    mapper->setMapping(buttons[i], i);
    connect(buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
  }
  connect(mapper, SIGNAL(mapped(int)), this, SLOT(calculationClicked(int)));

  // connect all remaining signals
  connect(buttons[COMMA], SIGNAL(clicked()), SLOT(commaClicked()));
  connect(buttons[PLUSMINUS], SIGNAL(clicked()), SLOT(plusminusClicked()));
  connect(buttons[PERCENT], SIGNAL(clicked()), SLOT(percentClicked()));
  connect(buttons[CLEAR], SIGNAL(clicked()), SLOT(clearClicked()));
  connect(buttons[CLEARALL], SIGNAL(clicked()), SLOT(clearAllClicked()));

  for(int i = 0; i < MAX_BUTTONS; ++i) {
    buttons[i]->setMinimumSize(40, 30);
    buttons[i]->setMaximumSize(40, 30);
  }
  int height = 4 * (buttons[0]->minimumHeight()+6) + 15;
  int width = 5 * (buttons[0]->minimumWidth()+6);

  setMinimumSize(width, height);
  setMaximumSize(width, height);
}

kMyMoneyCalculator::~kMyMoneyCalculator()
{
}

void kMyMoneyCalculator::digitClicked(int button)
{
  if(m_clearOperandOnDigit) {
    operand = QString();
    m_clearOperandOnDigit = false;
  }

  operand += QChar(button + 0x30);
  if(operand.length() > 16)
    operand = operand.left(16);
  changeDisplay(operand);
}

void kMyMoneyCalculator::commaClicked(void)
{
  if(operand.length() == 0)
    operand = '0';
  if(operand.contains('.', Qt::CaseInsensitive) == 0)
    operand.append('.');

  if(operand.length() > 16)
    operand = operand.left(16);
  changeDisplay(operand);
}

void kMyMoneyCalculator::plusminusClicked(void)
{
  if(operand.length() == 0 && m_result.length() > 0)
    operand = m_result;

  if(operand.length() > 0) {
    if(operand.indexOf('-') != -1)
      operand.replace('-', QChar());
    else
      operand.prepend('-');
    changeDisplay(operand);
  }
}

void kMyMoneyCalculator::calculationClicked(int button)
{
  if(operand.length() == 0 && op != 0 && button == EQUAL) {
    op = 0;
    m_result = normalizeString(op1);
    changeDisplay(m_result);

  } else if(operand.length() > 0 && op != 0) {
    // perform operation
    double op2 = operand.toDouble();
    bool error = false;

    // if the pending operation is addition and we now do multiplication
    // we just stack op1 and remember the operation in
    if((op == PLUS || op == MINUS) && (button == STAR || button == SLASH)) {
      op0 = op1;
      stackedOp = op;
      op = 0;
    }

    switch(op) {
      case PLUS:
        op2 = op1 + op2;
        break;
      case MINUS:
        op2 = op1 - op2;
        break;
      case STAR:
        op2 = op1 * op2;
        break;
      case SLASH:
        if(op2 == 0.0)
          error = true;
        else
          op2 = op1 / op2;
        break;
    }

    // if we have a pending addition operation, and the next operation is
    // not multiplication, we calculate the stacked operation
    if(stackedOp && button != STAR && button != SLASH) {
      switch(stackedOp) {
        case PLUS:
          op2 = op0 + op2;
          break;
        case MINUS:
          op2 = op0 - op2;
          break;
      }
      stackedOp = 0;
    }

    if(error) {
      op = 0;
      changeDisplay("Error");
      operand = QString();
    } else {
      op1 = op2;
      m_result = normalizeString(op1);
      changeDisplay(m_result);
    }
  } else if(operand.length() > 0 && op == 0) {
    op1 = operand.toDouble();
    m_result = normalizeString(op1);
    changeDisplay(m_result);
  }

  if(button != EQUAL) {
    op = button;
  } else {
    op = 0;
    emit signalResultAvailable();
  }
  operand = QString();
}

QString kMyMoneyCalculator::normalizeString(const double& val)
{
  QString str;
  str.setNum(val, 'f');
  int i = str.length();
  while(i > 1 && str[i-1] == '0') {
    --i;
  }
  // cut off trailing 0's
  str.remove(i, str.length());
  if(str.length() > 0) {
    // possibly remove trailing period
    if(str[str.length()-1] == '.') {
      str.remove(str.length()-1, 1);
    }
  }
  return str;
}

void kMyMoneyCalculator::clearClicked(void)
{
  if(operand.length() > 0) {
    operand = operand.left(operand.length() - 1);
  }
  if(operand.length() == 0)
    changeDisplay("0");
  else
    changeDisplay(operand);
}

void kMyMoneyCalculator::clearAllClicked(void)
{
  operand = QString();
  op = 0;
  changeDisplay("0");
}

void kMyMoneyCalculator::percentClicked(void)
{
  if(op != 0) {
    double op2 = operand.toDouble();
    switch(op) {
      case PLUS:
      case MINUS:
        op2 = (op1 * op2) / 100;
        break;

      case STAR:
      case SLASH:
        op2 /= 100;
        break;
    }
    operand = normalizeString(op2);
    changeDisplay(operand);
  }
}

const QString kMyMoneyCalculator::result(void) const
{
  QString txt = m_result;
  txt.replace(QRegExp("\\."), m_comma);
  if(txt[0] == '-') {
    txt = txt.mid(1); // get rid of the minus sign
    QString mask;
    switch(KGlobal::locale()->negativeMonetarySignPosition()) {
      case KLocale::ParensAround:
        mask = "(%1)";
        break;
      case KLocale::AfterQuantityMoney:
        mask = "%1-";
        break;
      case KLocale::AfterMoney:
      case KLocale::BeforeMoney:
        mask = "%1 -";
        break;
      case KLocale::BeforeQuantityMoney:
        mask = "-%1";
        break;
    }
    txt = QString(mask).arg(txt);
  }
  return txt;
}

void kMyMoneyCalculator::changeDisplay(const QString& str)
{
  QString txt = str;
  txt.replace(QRegExp("\\."), m_comma);
  display->setText("<b>" + txt + "</b>");
}

void kMyMoneyCalculator::keyPressEvent(QKeyEvent* ev)
{
  int button = -1;

  switch(ev->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
      if(m_clearOperandOnDigit) {
        operand = QString();
        m_clearOperandOnDigit = false;
      }
      button = ev->key() - Qt::Key_0;
      break;
    case Qt::Key_Plus:
      button = PLUS;
      break;
    case Qt::Key_Minus:
      button = MINUS;
      break;
    case Qt::Key_Comma:
    case Qt::Key_Period:
      if(m_clearOperandOnDigit) {
        operand = QString();
        m_clearOperandOnDigit = false;
      }
      button = COMMA;
      break;
    case Qt::Key_Slash:
      button = SLASH;
      break;
    case Qt::Key_Backspace:
      button = CLEAR;
      break;
    case Qt::Key_Asterisk:
      button = STAR;
      break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal:
      button = EQUAL;
      break;
    case Qt::Key_Escape:
      button = CLEARALL;
      break;
    case Qt::Key_Percent:
      button = PERCENT;
      break;
    default:
      ev->ignore();
      break;
  }
  if(button != -1)
    buttons[button]->animateClick();

  m_clearOperandOnDigit = false;
}

void kMyMoneyCalculator::setInitialValues(const QString& value, QKeyEvent* ev)
{
  bool negative = false;
  // setup operand
  operand = value;
  operand.replace(QRegExp(QString('\\')+KGlobal::locale()->thousandsSeparator()), QChar());
  operand.replace(QRegExp(QString('\\')+m_comma), ".");
  if(operand.contains('(')) {
    negative = true;
    operand.replace('(', QChar());
    operand.replace(')', QChar());
  }
  if(operand.contains('-')) {
    negative = true;
    operand.replace('-', QChar());
  }
  if(operand.isEmpty())
    operand = '0';
  else if(negative)
    operand = QString("-%1").arg(operand);

  changeDisplay(operand);

  // and operation
  op = 0;
  if(ev)
    keyPressEvent(ev);
  else
    m_clearOperandOnDigit = true;
}

#include "kmymoneycalculator.moc"
