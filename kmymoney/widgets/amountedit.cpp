/***************************************************************************
                                amountedit.cpp
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

#include "amountedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QStyle>
#include <QToolButton>
#include <QFrame>
#include <QLocale>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycalculator.h"
#include "mymoneymoney.h"
#include "icons/icons.h"

using namespace Icons;

AmountValidator::AmountValidator(QObject * parent) :
    QDoubleValidator(parent)
{
}

AmountValidator::AmountValidator(double bottom, double top, int decimals,
    QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
}

/*
 * The code of the following function is taken from kdeui/knumvalidator.cpp
 * and adjusted to always use the monetary symbols defined in the KDE System Settings
 */
QValidator::State AmountValidator::validate(QString & input, int & _p) const
{
  QString s = input;
  QLocale locale;
  // ok, we have to re-format the number to have:
  // 1. decimalSymbol == '.'
  // 2. negativeSign  == '-'
  // 3. positiveSign  == <empty>
  // 4. thousandsSeparator() == <empty> (we don't check that there
  //    are exactly three decimals between each separator):
  QString dec = locale.decimalPoint(),
              n = locale.negativeSign(),
                  p = locale.positiveSign(),
                      t = locale.groupSeparator();
  // first, delete p's and t's:
  if (!p.isEmpty())
    for (int idx = s.indexOf(p) ; idx >= 0 ; idx = s.indexOf(p, idx))
      s.remove(idx, p.length());


  if (!t.isEmpty())
    for (int idx = s.indexOf(t) ; idx >= 0 ; idx = s.indexOf(t, idx))
      s.remove(idx, t.length());

  // then, replace the d's and n's
  if ((!n.isEmpty() && n.indexOf('.') != -1) ||
      (!dec.isEmpty() && dec.indexOf('-') != -1)) {
    // make sure we don't replace something twice:
    qWarning() << "KDoubleValidator: decimal symbol contains '-' or "
    "negative sign contains '.' -> improve algorithm" << endl;
    return Invalid;
  }

  if (!dec.isEmpty() && dec != ".")
    for (int idx = s.indexOf(dec) ; idx >= 0 ; idx = s.indexOf(dec, idx + 1))
      s.replace(idx, dec.length(), ".");

  if (!n.isEmpty() && n != "-")
    for (int idx = s.indexOf(n) ; idx >= 0 ; idx = s.indexOf(n, idx + 1))
      s.replace(idx, n.length(), "-");

  // TODO: port KF5 (support for paren around negative numbers)
#if 0
  // Take care of monetary parens around the value if selected via
  // the locale settings.
  // If the lead-in or lead-out paren is present, remove it
  // before passing the string to the QDoubleValidator
  if (l->negativeMonetarySignPosition() == KLocale::ParensAround
      || l->positiveMonetarySignPosition() == KLocale::ParensAround) {
    QRegExp regExp("^(\\()?([\\d-\\.]*)(\\))?$");
    if (s.indexOf(regExp) != -1) {
      s = regExp.cap(2);
    }
  }
#endif

  // check for non numeric values (QDoubleValidator allows an 'e', we don't)
  QRegExp nonNumeric("[^\\d-\\.]+");
  if (s.indexOf(nonNumeric) != -1)
    return Invalid;

  // check for minus sign trailing the number
  QRegExp trailingMinus("^([^-]*)\\w*-$");
  if (s.indexOf(trailingMinus) != -1) {
    s = QString("-%1").arg(trailingMinus.cap(1));
  }

  // check for the maximum allowed number of decimal places
  int decPos = s.indexOf('.');
  if (decPos != -1) {
    if (decimals() == 0)
      return Invalid;
    if (((int)(s.length()) - decPos) > decimals())
      return Invalid;
  }

  // If we have just a single minus sign, we are done
  if (s == QString("-"))
    return Acceptable;

  QValidator::State rc = QDoubleValidator::validate(s, _p);
  // TODO: port KF5 (support for paren around negative numbers)
#if 0
  if (rc == Acceptable) {
    // If the numeric value is acceptable, we check if the parens
    // are ok. If only the lead-in is present, the return value
    // is intermediate, if only the lead-out is present then it
    // definitely is invalid. Nevertheless, we check for parens
    // only, if the locale settings have it enabled.
    if (l->negativeMonetarySignPosition() == KLocale::ParensAround
        || l->positiveMonetarySignPosition() == KLocale::ParensAround) {
      int tmp = input.count('(') - input.count(')');
      if (tmp > 0)
        rc = Intermediate;
      else if (tmp < 0)
        rc = Invalid;
    }
  }
#endif
  return rc;
}







class AmountEdit::Private
{
public:
  Private(AmountEdit* q)
  : m_q(q)
  , m_allowEmpty(false)
  {
    m_calculatorFrame = new QFrame(m_q);
    m_calculatorFrame->setWindowFlags(Qt::Popup);

    m_calculatorFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    m_calculatorFrame->setLineWidth(3);

    m_calculator = new kMyMoneyCalculator(m_calculatorFrame);
    m_calculatorFrame->hide();
  }

  AmountEdit*           m_q;
  QFrame*               m_calculatorFrame;
  kMyMoneyCalculator*   m_calculator;
  QToolButton*          m_calculatorButton;
  int                   m_prec;
  bool                  m_allowEmpty;
  QString               m_previousText; // keep track of what has been typed
  QString               m_text;         // keep track of what was the original value
};





int AmountEdit::standardPrecision = 2;

AmountEdit::AmountEdit(QWidget *parent, const int prec)
  : QLineEdit(parent)
  , d(new Private(this))
{
  d->m_prec = prec;
  if (prec < -1 || prec > 20) {
    d->m_prec = standardPrecision;
  }
  init();
}

AmountEdit::AmountEdit(const MyMoneySecurity& sec, QWidget *parent)
  : QLineEdit(parent)
  , d(new Private(this))
{
  d->m_prec = MyMoneyMoney::denomToPrec(sec.smallestAccountFraction());
  init();
}

AmountEdit::~AmountEdit()
{
}

void AmountEdit::setStandardPrecision(int prec)
{
  if (prec >= 0 && prec < 20) {
    standardPrecision = prec;
  }
}

void AmountEdit::init()
{
  // Yes, just a simple double validator !
  AmountValidator *validator = new AmountValidator(this);
  setValidator(validator);
  setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  int height = sizeHint().height();
  int btnSize = sizeHint().height() - 5;

  d->m_calculatorButton = new QToolButton(this);
  d->m_calculatorButton->setIcon(QIcon::fromTheme(g_Icons[Icon::AccessoriesCalculator]));
  d->m_calculatorButton->setCursor(Qt::ArrowCursor);
  d->m_calculatorButton->setStyleSheet("QToolButton { border: none; padding: 2px}");
  d->m_calculatorButton->setFixedSize(btnSize, btnSize);
  d->m_calculatorButton->show();

  int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  setStyleSheet(QString("QLineEdit { padding-right: %1px }")
                                              .arg(btnSize - frameWidth));
  setMinimumHeight(height);

  connect(d->m_calculatorButton, SIGNAL(clicked()), this, SLOT(slotCalculatorOpen()));

  KSharedConfig::Ptr kconfig = KSharedConfig::openConfig();
  KConfigGroup grp = kconfig->group("General Options");
  if (grp.readEntry("DontShowCalculatorButton", false) == true)
    setCalculatorButtonVisible(false);

  connect(this, SIGNAL(textChanged(QString)), this, SLOT(theTextChanged(QString)));
  connect(d->m_calculator, SIGNAL(signalResultAvailable()), this, SLOT(slotCalculatorResult()));
  connect(d->m_calculator, SIGNAL(signalQuit()), this, SLOT(slotCalculatorClose()));
}

void AmountEdit::resizeEvent(QResizeEvent* event)
{
  Q_UNUSED(event);
  const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  d->m_calculatorButton->move(width() - d->m_calculatorButton->width() - frameWidth - 2, 2);
}

void AmountEdit::focusOutEvent(QFocusEvent* event)
{
  QLineEdit::focusOutEvent(event);

  // make sure we have a zero value in case the current text
  // is empty but this is not allowed
  if (text().isEmpty() && !d->m_allowEmpty) {
    QLineEdit::setText(QLatin1String("0"));
  }

  // make sure we have a fractional part
  if (!text().isEmpty())
    ensureFractionalPart();

  // in case the widget contains a different value we emit
  // the valueChanged signal
  if (MyMoneyMoney(text()) != MyMoneyMoney(d->m_text)) {
    emit valueChanged(text());
  }
}

void AmountEdit::keyPressEvent(QKeyEvent* event)
{
  switch(event->key()) {
    case Qt::Key_Plus:
    case Qt::Key_Minus:
        if (hasSelectedText()) {
          cut();
        }
        if (text().length() == 0) {
          QLineEdit::keyPressEvent(event);
          break;
        }
        // in case of '-' we do not enter the calculator when
        // the current position is the beginning and there is
        // no '-' sign at the first position.
        if (event->key() == Qt::Key_Minus) {
          if (cursorPosition() == 0 && text()[0] != '-') {
            QLineEdit::keyPressEvent(event);
            break;
          }
        }
        // intentional fall through

      case Qt::Key_Slash:
      case Qt::Key_Asterisk:
      case Qt::Key_Percent:
        if (hasSelectedText()) {
          // remove the selected text
          cut();
        }
        calculatorOpen(event);
        break;

    default:
      QLineEdit::keyPressEvent(event);
      break;
  }
}


void AmountEdit::setPrecision(const int prec)
{
  if (prec >= -1 && prec <= 20) {
    if (prec != d->m_prec) {
      d->m_prec = prec;
      // update current display
      setValue(value());
    }
  }
}

int AmountEdit::precision() const
{
  return d->m_prec;
}


bool AmountEdit::isValid() const
{
  return !(text().isEmpty());
}

MyMoneyMoney AmountEdit::value() const
{
  MyMoneyMoney money(text());
  if (d->m_prec != -1)
    money = money.convert(MyMoneyMoney::precToDenom(d->m_prec));
  return money;
}

void AmountEdit::setValue(const MyMoneyMoney& value)
{
  // load the value into the widget but don't use thousandsSeparators
  setText(value.formatMoney("", d->m_prec, false));
}

void AmountEdit::setText(const QString& txt)
{
  d->m_text = txt;
  if (isEnabled() && !txt.isEmpty())
    ensureFractionalPart(d->m_text);
  QLineEdit::setText(d->m_text);
#if 0
  m_resetButton->setEnabled(false);
#endif
}

void AmountEdit::resetText()
{
#if 0
  setText(d->m_text);
  m_resetButton->setEnabled(false);
#endif
}

void AmountEdit::theTextChanged(const QString & theText)
{
  QLocale locale;
  QString dec = locale.groupSeparator();
  QString l_text = theText;
  QString nsign, psign;
  nsign = locale.negativeSign();
  psign = locale.positiveSign();

  int i = 0;
  if (isEnabled()) {
    QValidator::State state =  validator()->validate(l_text, i);
    if (state == QValidator::Intermediate) {
      if (l_text.length() == 1) {
        if (l_text != dec && l_text != nsign && l_text != psign)
          state = QValidator::Invalid;
      }
    }
    if (state == QValidator::Invalid)
      QLineEdit::setText(d->m_previousText);
    else {
      d->m_previousText = l_text;
      emit validatedTextChanged(text());
    }
  }
}

void AmountEdit::ensureFractionalPart()
{
  QString s(text());
  ensureFractionalPart(s);
  // by setting the text only when it's different then the one that it is already there
  // we preserve the edit widget's state (like the selection for example) during a
  // call to ensureFractionalPart() that does not change anything
  if (s != text())
    QLineEdit::setText(s);
}

void AmountEdit::ensureFractionalPart(QString& s) const
{
  s = MyMoneyMoney(s).formatMoney("", d->m_prec, false);
}


void AmountEdit::slotCalculatorOpen()
{
  calculatorOpen(0);
}

void AmountEdit::calculatorOpen(QKeyEvent* k)
{
  d->m_calculator->setInitialValues(text(), k);

  int h = d->m_calculatorFrame->height();
  int w = d->m_calculatorFrame->width();

  // usually, the calculator widget is shown underneath the MoneyEdit widget
  // if it does not fit on the screen, we show it above this widget
  QPoint p = mapToGlobal(QPoint(0, 0));
  if (p.y() + height() + h > QApplication::desktop()->height())
    p.setY(p.y() - h);
  else
    p.setY(p.y() + height());

  // usually, it is shown left aligned. If it does not fit, we align it
  // to the right edge of the widget
  if (p.x() + w > QApplication::desktop()->width())
    p.setX(p.x() + width() - w);

  QRect r = d->m_calculator->geometry();
  r.moveTopLeft(p);
  d->m_calculatorFrame->setGeometry(r);
  d->m_calculatorFrame->show();
  d->m_calculator->setFocus();
}

void AmountEdit::slotCalculatorClose()
{
  if (d->m_calculator != 0) {
    d->m_calculatorFrame->hide();
  }
}

void AmountEdit::slotCalculatorResult()
{
  slotCalculatorClose();
  if (d->m_calculator != 0) {
    setText(d->m_calculator->result());
    ensureFractionalPart();
#if 0
    // I am not sure if getting a result from the calculator
    // is a good event to emit a value changed signal. We
    // should do this only on focusOutEvent()
    emit valueChanged(text());
    d->m_text = text();
#endif
  }
}

void AmountEdit::setCalculatorButtonVisible(const bool show)
{
  d->m_calculatorButton->setVisible(show);
}

void AmountEdit::setAllowEmpty(bool allowed)
{
  d->m_allowEmpty = allowed;
}

bool AmountEdit::isEmptyAllowed() const
{
  return d->m_allowEmpty;
}

bool AmountEdit::isCalculatorButtonVisible() const
{
  return d->m_calculatorButton->isVisible();
}






CreditDebitHelper::CreditDebitHelper(QObject* parent, AmountEdit* credit, AmountEdit* debit)
  : QObject(parent)
  , m_credit(credit)
  , m_debit(debit)
{
  connect(m_credit, SIGNAL(valueChanged(QString)), this, SLOT(creditChanged()));
  connect(m_debit, SIGNAL(valueChanged(QString)), this, SLOT(debitChanged()));
}

CreditDebitHelper::~CreditDebitHelper()
{
}

void CreditDebitHelper::creditChanged()
{
  widgetChanged(m_credit, m_debit);
}

void CreditDebitHelper::debitChanged()
{
  widgetChanged(m_debit, m_credit);
}

void CreditDebitHelper::widgetChanged(AmountEdit* src, AmountEdit* dst)
{
  // make sure the objects exist
  if(!src || !dst) {
    return;
  }

  // in case both are filled with text, the src wins
  if(!src->text().isEmpty() && !dst->text().isEmpty()) {
    dst->clear();
  }

  // in case the source is negative, we negate the value
  // and load it into destination.
  if(src->value().isNegative()) {
    dst->setValue(-(src->value()));
    src->clear();
  }
  emit valueChanged();
}

bool CreditDebitHelper::haveValue() const
{
  return (!m_credit->text().isEmpty()) || (!m_debit->text().isEmpty());
}

MyMoneyMoney CreditDebitHelper::value() const
{
  MyMoneyMoney value;
  if(m_credit && m_debit) {
    if(!m_credit->text().isEmpty()) {
      value = -m_credit->value();
    } else {
      value = m_debit->value();
    }
  } else {
    qWarning() << "CreditDebitHelper::value() called with no objects attached. Zero returned.";
  }
  return value;
}

void CreditDebitHelper::setValue(const MyMoneyMoney& value)
{
  if(m_credit && m_debit) {
    if(value.isNegative()) {
      m_credit->setValue(-value);
      m_debit->clear();
    } else {
      m_debit->setValue(value);
      m_credit->clear();
    }
  } else {
    qWarning() << "CreditDebitHelper::setValue() called with no objects attached. Skipped.";
  }
}
