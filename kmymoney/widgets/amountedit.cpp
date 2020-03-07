/*
 * Copyright 2010-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "amountvalidator.h"
#include "kmymoneycalculator.h"
#include "mymoneysecurity.h"
#include "icons/icons.h"

using namespace Icons;

class AmountEditHelper
{
public:
  AmountEditHelper() : q(nullptr) {}
  ~AmountEditHelper() { delete q; }
  AmountEdit *q;
};

Q_GLOBAL_STATIC(AmountEditHelper, s_globalAmountEdit)

AmountEdit* AmountEdit::global()
{
  if (!s_globalAmountEdit()->q) {
    s_globalAmountEdit()->q = new AmountEdit(0, 2);
  }

  return s_globalAmountEdit()->q;
}

class AmountEditPrivate
{
  Q_DISABLE_COPY(AmountEditPrivate)
  Q_DECLARE_PUBLIC(AmountEdit)

public:
  explicit AmountEditPrivate(AmountEdit* qq)
    : q_ptr(qq)
    , m_calculatorFrame(nullptr)
    , m_calculator(nullptr)
    , m_calculatorButton(nullptr)
    , m_currencySymbol(nullptr)
    , m_prec(2)
    , m_allowEmpty(false)
  {
    Q_Q(AmountEdit);
    m_calculatorFrame = new QFrame(q);
    m_calculatorFrame->setWindowFlags(Qt::Popup);

    m_calculatorFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    m_calculatorFrame->setLineWidth(3);

    m_calculator = new KMyMoneyCalculator(m_calculatorFrame);
    m_calculatorFrame->hide();
  }

  void init()
  {
    Q_Q(AmountEdit);
    // Yes, just a simple double validator !
    auto validator = new AmountValidator(q);
    q->setValidator(validator);
    q->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    const auto btnSize = q->sizeHint().height() - 5;

    m_calculatorButton = new QToolButton(q);
    m_calculatorButton->setIcon(Icons::get(Icon::Calculator));
    m_calculatorButton->setCursor(Qt::ArrowCursor);
    m_calculatorButton->setStyleSheet("QToolButton { border: none; padding: 2px}");
    m_calculatorButton->setFixedSize(btnSize, btnSize);
    m_calculatorButton->setFocusPolicy(Qt::ClickFocus);
    m_calculatorButton->show();

    m_currencySymbol = new QLabel(q);
    m_currencySymbol->setCursor(Qt::ArrowCursor);
    m_currencySymbol->setFixedSize(btnSize, btnSize);
    m_currencySymbol->hide();

    updateLineEditSize(true, false);

    q->connect(m_calculatorButton, &QAbstractButton::clicked, q, &AmountEdit::slotCalculatorOpen);

    KSharedConfig::Ptr kconfig = KSharedConfig::openConfig();
    KConfigGroup grp = kconfig->group("General Options");
    if (grp.readEntry("DontShowCalculatorButton", false) == true)
      q->setCalculatorButtonVisible(false);

    q->connect(q, &QLineEdit::textChanged, q, &AmountEdit::theTextChanged);
    q->connect(m_calculator, &KMyMoneyCalculator::signalResultAvailable, q, &AmountEdit::slotCalculatorResult);
    q->connect(m_calculator, &KMyMoneyCalculator::signalQuit, q, &AmountEdit::slotCalculatorClose);
  }

  /**
    * Internal helper function for value() and ensureFractionalPart().
    */
  void ensureFractionalPart(QString& s) const
  {
    s = MyMoneyMoney(s).formatMoney(QString(), m_prec, false);
  }

  /**
    * This method opens the calculator and replays the key
    * event pointed to by @p ev. If @p ev is 0, then no key
    * event is replayed.
    *
    * @param ev pointer to QKeyEvent that started the calculator.
    */
  void calculatorOpen(QKeyEvent* k)
  {
    Q_Q(AmountEdit);
    // do not open the calculator in read-only mode
    if (q->isReadOnly())
      return;

    m_calculator->setInitialValues(q->text(), k);

    auto h = m_calculatorFrame->height();
    auto w = m_calculatorFrame->width();

    // usually, the calculator widget is shown underneath the MoneyEdit widget
    // if it does not fit on the screen, we show it above this widget
    auto p = q->mapToGlobal(QPoint(0, 0));
    if (p.y() + q->height() + h > QApplication::desktop()->height())
      p.setY(p.y() - h);
    else
      p.setY(p.y() + q->height());

    // usually, it is shown left aligned. If it does not fit, we align it
    // to the right edge of the widget
    if (p.x() + w > QApplication::desktop()->width())
      p.setX(p.x() + q->width() - w);

    m_calculatorFrame->show();

    QRect r = m_calculator->geometry();
    r.moveTopLeft(p);
    m_calculatorFrame->setGeometry(r);
    m_calculator->setFocus();
  }

  void updateLineEditSize(bool calculatorVisible, bool currencySymbolVisible)
  {
    Q_Q(AmountEdit);
    int height = q->sizeHint().height();
    int frameWidth = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    const int btnSize = height - 5;
    int factor = 0;
    factor += calculatorVisible ? 1 : 0;
    factor += currencySymbolVisible ? 1 : 0;
    q->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(factor*btnSize - frameWidth));
    q->setMinimumHeight(height);
  }


  AmountEdit*           q_ptr;
  QFrame*               m_calculatorFrame;
  KMyMoneyCalculator*   m_calculator;
  QToolButton*          m_calculatorButton;
  QLabel*               m_currencySymbol;
  int                   m_prec;
  bool                  m_allowEmpty;
  QString               m_previousText; // keep track of what has been typed
  QString               m_text;         // keep track of what was the original value
  /**
   * This holds the number of precision to be used
   * when no other information (e.g. from account)
   * is available.
   *
   * @sa setStandardPrecision()
   */
};

void AmountEdit::setReadOnly(bool ro)
{
  Q_D(AmountEdit);
  d->m_calculatorButton->setEnabled(!ro);
  QLineEdit::setReadOnly(ro);
}

AmountEdit::AmountEdit(QWidget *parent, const int prec) :
  QLineEdit(parent),
  d_ptr(new AmountEditPrivate(this))
{
  Q_D(AmountEdit);
  d->m_prec = prec;
  if (prec < -1 || prec > 20) {
    d->m_prec = AmountEdit::global()->standardPrecision();
  }
  d->init();
}

AmountEdit::AmountEdit(const MyMoneySecurity& sec, QWidget *parent) :
  QLineEdit(parent),
  d_ptr(new AmountEditPrivate(this))
{
  Q_D(AmountEdit);
  d->m_prec = MyMoneyMoney::denomToPrec(sec.smallestAccountFraction());
  d->init();
}

AmountEdit::~AmountEdit()
{
  Q_D(AmountEdit);
  delete d;
}

void AmountEdit::setStandardPrecision(int prec)
{
  if (prec >= 0 && prec < 20) {
    global()->d_ptr->m_prec = prec;
  }
}

int AmountEdit::standardPrecision()
{
  return global()->d_ptr->m_prec;
}


void AmountEdit::resizeEvent(QResizeEvent* event)
{
  Q_D(AmountEdit);
  Q_UNUSED(event);
  const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  d->m_calculatorButton->move(width() - d->m_calculatorButton->width() - frameWidth - 2, 2);
  d->m_currencySymbol->move(width() - d->m_calculatorButton->width() - d->m_currencySymbol->width() - frameWidth - 2, 2);
}

void AmountEdit::focusOutEvent(QFocusEvent* event)
{
  Q_D(AmountEdit);
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
  Q_D(AmountEdit);
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
        d->calculatorOpen(event);
        break;

    default:
      // in case all text is selected and the user presses the decimal point
      // we fill the widget with the leading "0". The outcome of this will be
      // that the widget then contains "0.".
      if ((event->key() == QLocale().decimalPoint()) && (selectedText() == text())) {
        QLineEdit::setText(QLatin1String("0"));
      }
      QLineEdit::keyPressEvent(event);
      break;
  }
}


void AmountEdit::setPrecision(const int prec)
{
  Q_D(AmountEdit);
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
  Q_D(const AmountEdit);
  return d->m_prec;
}

bool AmountEdit::isValid() const
{
  return !(text().isEmpty());
}

QString AmountEdit::numericalText() const
{
  return value().toString();
}

MyMoneyMoney AmountEdit::value() const
{
  Q_D(const AmountEdit);
  MyMoneyMoney money(text());
  if (d->m_prec != -1)
    money = money.convert(MyMoneyMoney::precToDenom(d->m_prec));
  return money;
}

void AmountEdit::setValue(const MyMoneyMoney& value)
{
  Q_D(AmountEdit);
  // load the value into the widget but don't use thousandsSeparators
  setText(value.formatMoney(QString(), d->m_prec, false));
}

void AmountEdit::setText(const QString& txt)
{
  Q_D(AmountEdit);
  d->m_text = txt;
  if (isEnabled() && !txt.isEmpty())
    d->ensureFractionalPart(d->m_text);
  QLineEdit::setText(d->m_text);
#if 0
  m_resetButton->setEnabled(false);
#endif
}

void AmountEdit::resetText()
{
#if 0
  Q_D(AmountEdit);
  setText(d->m_text);
  m_resetButton->setEnabled(false);
#endif
}

void AmountEdit::theTextChanged(const QString & theText)
{
  Q_D(AmountEdit);
  QLocale locale;
  QString dec = locale.groupSeparator();
  QString l_text = theText;
  QString nsign, psign;
  nsign = locale.negativeSign();
  psign = locale.positiveSign();

  auto i = 0;
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


void AmountEdit::slotCalculatorOpen()
{
  Q_D(AmountEdit);
  d->calculatorOpen(0);
}

void AmountEdit::slotCalculatorClose()
{
  Q_D(AmountEdit);
  if (d->m_calculator != 0) {
    d->m_calculatorFrame->hide();
  }
}

void AmountEdit::slotCalculatorResult()
{
  Q_D(AmountEdit);
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
  Q_D(AmountEdit);
  d->m_calculatorButton->setVisible(show);
  d->updateLineEditSize(show, d->m_currencySymbol->isVisible());
}

void AmountEdit::setAllowEmpty(bool allowed)
{
  Q_D(AmountEdit);
  d->m_allowEmpty = allowed;
}

bool AmountEdit::isEmptyAllowed() const
{
  Q_D(const AmountEdit);
  return d->m_allowEmpty;
}

bool AmountEdit::isCalculatorButtonVisible() const
{
  Q_D(const AmountEdit);
  return d->m_calculatorButton->isVisible();
}

void AmountEdit::ensureFractionalPart()
{
  Q_D(AmountEdit);
  QString s(text());
  d->ensureFractionalPart(s);
  // by setting the text only when it's different then the one that it is already there
  // we preserve the edit widget's state (like the selection for example) during a
  // call to ensureFractionalPart() that does not change anything
  if (s != text())
    QLineEdit::setText(s);
}

void AmountEdit::showCurrencySymbol(const QString& symbol)
{
  Q_D(AmountEdit);
  d->m_currencySymbol->setText(symbol);
  d->m_currencySymbol->setHidden(symbol.isEmpty());
  d->updateLineEditSize(d->m_calculator->isVisible(), !symbol.isEmpty());
}
