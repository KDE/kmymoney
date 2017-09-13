/***************************************************************************
                          kmymoneyedit.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QFrame>
#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <QIcon>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneylineedit.h"
#include "kmymoneycalculator.h"
#include "mymoneymoney.h"
#include <icons.h>

using namespace Icons;

kMyMoneyMoneyValidator::kMyMoneyMoneyValidator(QObject * parent) :
    QDoubleValidator(parent)
{
  setLocale(QLocale::c());
}

kMyMoneyMoneyValidator::kMyMoneyMoneyValidator(double bottom, double top, int decimals,
    QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
  setLocale(QLocale::c());
}

/*
 * The code of the following function is taken from kdeui/knumvalidator.cpp
 * and adjusted to always use the monetary symbols defined in the KDE System Settings
 */
QValidator::State kMyMoneyMoneyValidator::validate(QString & input, int & _p) const
{
  Q_UNUSED(_p)
  QString s = input;
  // TODO: port this to kf5
#if 0
  KLocale * l = KLocale::global();
  // ok, we have to re-format the number to have:
  // 1. decimalSymbol == '.'
  // 2. negativeSign  == '-'
  // 3. positiveSign  == <empty>
  // 4. thousandsSeparator() == <empty> (we don't check that there
  //    are exactly three decimals between each separator):
  QString d = l->monetaryDecimalSymbol(),
              n = l->negativeSign(),
                  p = l->positiveSign(),
                      t = l->monetaryThousandsSeparator();
  // first, delete p's and t's:
  if (!p.isEmpty())
    for (int idx = s.indexOf(p) ; idx >= 0 ; idx = s.indexOf(p, idx))
      s.remove(idx, p.length());


  if (!t.isEmpty())
    for (int idx = s.indexOf(t) ; idx >= 0 ; idx = s.indexOf(t, idx))
      s.remove(idx, t.length());

  // then, replace the d's and n's
  if ((!n.isEmpty() && n.indexOf('.') != -1) ||
      (!d.isEmpty() && d.indexOf('-') != -1)) {
    // make sure we don't replace something twice:
    qWarning() << "KDoubleValidator: decimal symbol contains '-' or "
    "negative sign contains '.' -> improve algorithm" << endl;
    return Invalid;
  }

  if (!d.isEmpty() && d != ".")
    for (int idx = s.indexOf(d) ; idx >= 0 ; idx = s.indexOf(d, idx + 1))
      s.replace(idx, d.length(), ".");

  if (!n.isEmpty() && n != "-")
    for (int idx = s.indexOf(n) ; idx >= 0 ; idx = s.indexOf(n, idx + 1))
      s.replace(idx, n.length(), "-");

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
  return rc;
#else
  return Acceptable;
#endif
}

kMyMoneyEdit::kMyMoneyEdit(QWidget *parent, const int prec)
    : QWidget(parent)
{
  m_prec = prec;
  // TODO: port this to kf5
  if (prec < -1 || prec > 20)
    m_prec = 2;//KLocale::global()->monetaryDecimalPlaces();
  init();
}

kMyMoneyEdit::kMyMoneyEdit(const MyMoneySecurity& sec, QWidget *parent)
    : QWidget(parent)
{
  m_prec = MyMoneyMoney::denomToPrec(sec.smallestAccountFraction());
  init();
}

// converted image from kde3.5.1/share/apps/kdevdesignerpart/pics/designer_resetproperty.png
static const uchar resetButtonImage[] = {
  0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
  0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x84,
  0x76, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4B, 0x47,
  0x44, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xA0,
  0xBD, 0xA7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70,
  0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00,
  0x00, 0x0B, 0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18,
  0x00, 0x00, 0x00, 0x07, 0x74, 0x49, 0x4D, 0x45,
  0x07, 0xD6, 0x06, 0x10, 0x09, 0x36, 0x0C, 0x58,
  0x91, 0x11, 0x7C, 0x00, 0x00, 0x00, 0x64, 0x49,
  0x44, 0x41, 0x54, 0x78, 0xDA, 0x65, 0xC9, 0xA1,
  0x0D, 0x02, 0x41, 0x18, 0x84, 0xD1, 0xF7, 0x5F,
  0x13, 0x04, 0x9A, 0x39, 0x43, 0x68, 0x81, 0x02,
  0x10, 0xB8, 0x13, 0x74, 0x80, 0xC1, 0x21, 0x76,
  0x1D, 0xDD, 0xD0, 0x01, 0x65, 0x10, 0x34, 0x9A,
  0x0C, 0x66, 0x83, 0x61, 0x92, 0x2F, 0x23, 0x5E,
  0x25, 0x01, 0xBD, 0x6A, 0xC6, 0x1D, 0x9B, 0x25,
  0x79, 0xC2, 0x34, 0xE0, 0x30, 0x00, 0x56, 0xBD,
  0x6A, 0x0D, 0xD5, 0x38, 0xE1, 0xEA, 0x7F, 0xE7,
  0x4A, 0xA2, 0x57, 0x1D, 0x71, 0xC1, 0x07, 0xBB,
  0x81, 0x8F, 0x09, 0x96, 0xE4, 0x86, 0x3D, 0xDE,
  0x78, 0x8D, 0x48, 0xF2, 0xAB, 0xB1, 0x1D, 0x9F,
  0xC6, 0xFC, 0x05, 0x46, 0x68, 0x28, 0x6B, 0x58,
  0xEE, 0x72, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x49,
  0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

void kMyMoneyEdit::init()
{
  QHBoxLayout *editLayout = new QHBoxLayout(this);
  editLayout->setSpacing(0);
  editLayout->setContentsMargins(0, 0, 0, 0);

  allowEmpty = false;
  m_edit = new kMyMoneyLineEdit(this, true);
  m_edit->installEventFilter(this);
  setFocusProxy(m_edit);
  editLayout->addWidget(m_edit);

  // Yes, just a simple double validator !
  kMyMoneyMoneyValidator *validator = new kMyMoneyMoneyValidator(this);
  m_edit->setValidator(validator);
  m_edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_calculatorFrame = new QWidget;
  QVBoxLayout *calculatorFrameVBoxLayout = new QVBoxLayout(m_calculatorFrame);
  calculatorFrameVBoxLayout->setMargin(0);
  m_calculatorFrame->setWindowFlags(Qt::Popup);

  m_calculator = new kMyMoneyCalculator(m_calculatorFrame);
  calculatorFrameVBoxLayout->addWidget(m_calculator);
  m_calculatorFrame->hide();

  m_calcButton = new QPushButton(QIcon::fromTheme(g_Icons[Icon::AccessoriesCalculator]), QString(), this);
  m_calcButton->setFocusProxy(m_edit);
  editLayout->addWidget(m_calcButton);

  QPixmap pixmap;
  pixmap.loadFromData(resetButtonImage, sizeof(resetButtonImage), "PNG", 0);
  m_resetButton = new QPushButton(pixmap, QString(""), this);
  m_resetButton->setEnabled(false);
  m_resetButton->setFocusProxy(m_edit);
  editLayout->addWidget(m_resetButton);

  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  KConfigGroup grp = kconfig->group("General Options");
  if (grp.readEntry("DontShowCalculatorButton", false) == true)
    setCalculatorButtonVisible(false);

  connect(m_edit, SIGNAL(textChanged(QString)), this, SLOT(theTextChanged(QString)));
  connect(m_calculator, SIGNAL(signalResultAvailable()), this, SLOT(slotCalculatorResult()));
  connect(m_calcButton, SIGNAL(clicked()), this, SLOT(slotCalculatorOpen()));
  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetText()));
}

void kMyMoneyEdit::setValidator(const QValidator* v)
{
  m_edit->setValidator(v);
}

kMyMoneyEdit::~kMyMoneyEdit()
{
}

KLineEdit* kMyMoneyEdit::lineedit() const
{
  return m_edit;
}

void kMyMoneyEdit::setPrecision(const int prec)
{
  if (prec >= -1 && prec <= 20) {
    if (prec != m_prec) {
      m_prec = prec;
      // update current display
      setValue(value());
    }
  }
}

bool kMyMoneyEdit::isValid() const
{
  return !(m_edit->text().isEmpty());
}

MyMoneyMoney kMyMoneyEdit::value() const
{
  QString txt = m_edit->text();
  ensureFractionalPart(txt);
  MyMoneyMoney money(txt);
  if (m_prec != -1)
    money = money.convert(MyMoneyMoney::precToDenom(m_prec));
  return money;
}

void kMyMoneyEdit::setValue(const MyMoneyMoney& value)
{
  // load the value into the widget but don't use thousandsSeparators
  QString txt = value.formatMoney("", m_prec, false);
  loadText(txt);
}

void kMyMoneyEdit::loadText(const QString& txt)
{
  m_edit->setText(txt);
  if (isEnabled() && !txt.isEmpty())
    ensureFractionalPart();
  m_text = m_edit->text();
  m_resetButton->setEnabled(false);
}

void kMyMoneyEdit::clearText()
{
  m_text.clear();
  m_edit->setText(m_text);
}

void kMyMoneyEdit::resetText()
{
  m_edit->setText(m_text);
  m_resetButton->setEnabled(false);
}

void kMyMoneyEdit::theTextChanged(const QString & theText)
{
  QString d = QLocale().decimalPoint();
  QString l_text = theText;
  QString nsign, psign;
#if 0
  KLocale * l = KLocale::global();
  if (l->negativeMonetarySignPosition() == KLocale::ParensAround
      || l->positiveMonetarySignPosition() == KLocale::ParensAround) {
    nsign = psign = '(';
  } else {
    nsign = l->negativeSign();
    psign = l->positiveSign();
  }
#else
  nsign = "-";
  psign = "";
#endif
  int i = 0;
  if (isEnabled()) {
    QValidator::State state =  m_edit->validator()->validate(l_text, i);
    if (state == QValidator::Intermediate) {
      if (l_text.length() == 1) {
        if (l_text != d && l_text != nsign && l_text != psign)
          state = QValidator::Invalid;
      }
    }
    if (state == QValidator::Invalid)
      m_edit->setText(previousText);
    else {
      previousText = l_text;
      emit textChanged(m_edit->text());
      m_resetButton->setEnabled(true);
    }
  }
}

void kMyMoneyEdit::ensureFractionalPart()
{
  QString s(m_edit->text());
  ensureFractionalPart(s);
  // by setting the text only when it's different then the one that it is already there
  // we preserve the edit widget's state (like the selection for example) during a
  // call to ensureFractionalPart() that does not change anything
  if (s != m_edit->text())
    m_edit->setText(s);
}

void kMyMoneyEdit::ensureFractionalPart(QString& s) const
{
  QString decimalSymbol = QLocale().decimalPoint();
  if (decimalSymbol.isEmpty())
    decimalSymbol = '.';

  // If text contains no 'monetaryDecimalSymbol' then add it
  // followed by the required number of 0s
  if (!s.isEmpty()) {
    if (m_prec > 0) {
      if (!s.contains(decimalSymbol)) {
        s += decimalSymbol;
        for (int i = 0; i < m_prec; i++)
          s += '0';
      }
    } else if (m_prec == 0) {
      while (s.contains(decimalSymbol)) {
        int pos = s.lastIndexOf(decimalSymbol);
        if (pos != -1) {
          s.truncate(pos);
        }
      }
    } else if (s.contains(decimalSymbol)) { // m_prec == -1 && fraction
      // no trailing zeroes
      while (s.endsWith('0')) {
        s.truncate(s.length() - 1);
      }
      // no trailing decimalSymbol
      if (s.endsWith(decimalSymbol))
        s.truncate(s.length() - 1);
    }
  }
}

bool kMyMoneyEdit::eventFilter(QObject * /* o */ , QEvent *e)
{
  bool rc = false;

  // we want to catch some keys that are usually handled by
  // the base class (e.g. '+', '-', etc.)
  if (e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *>(e);

    rc = true;
    switch (k->key()) {
      case Qt::Key_Plus:
      case Qt::Key_Minus:
        if (m_edit->hasSelectedText()) {
          m_edit->cut();
        }
        if (m_edit->text().length() == 0) {
          rc = false;
          break;
        }
        // in case of '-' we do not enter the calculator when
        // the current position is the beginning and there is
        // no '-' sign at the first position.
        if (k->key() == Qt::Key_Minus) {
          if (m_edit->cursorPosition() == 0 && m_edit->text()[0] != '-') {
            rc = false;
            break;
          }
        }
        // intentional fall through

      case Qt::Key_Slash:
      case Qt::Key_Asterisk:
      case Qt::Key_Percent:
        if (m_edit->hasSelectedText()) {
          // remove the selected text
          m_edit->cut();
        }
        calculatorOpen(k);
        break;

      default:
        rc = false;
        break;
    }

  } else if (e->type() == QEvent::FocusOut) {
    if (!m_edit->text().isEmpty() || !allowEmpty)
      ensureFractionalPart();

    if (MyMoneyMoney(m_edit->text()) != MyMoneyMoney(m_text)
        && !m_calculator->isVisible()) {
      emit valueChanged(m_edit->text());
    }
    m_text = m_edit->text();
  }
  return rc;
}

void kMyMoneyEdit::slotCalculatorOpen()
{
  calculatorOpen(0);
}

void kMyMoneyEdit::calculatorOpen(QKeyEvent* k)
{
  m_calculator->setInitialValues(m_edit->text(), k);

  int h = m_calculatorFrame->height();
  int w = m_calculatorFrame->width();

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

  QRect r = m_calculator->geometry();
  r.moveTopLeft(p);
  m_calculatorFrame->setGeometry(r);
  m_calculatorFrame->show();
  m_calculator->setFocus();
}

void kMyMoneyEdit::slotCalculatorResult()
{
  QString result;
  if (m_calculator != 0) {
    m_calculatorFrame->hide();
    m_edit->setText(m_calculator->result());
    ensureFractionalPart();
    emit valueChanged(m_edit->text());
    m_text = m_edit->text();
  }
}

QWidget* kMyMoneyEdit::focusWidget() const
{
  QWidget* w = m_edit;
  while (w->focusProxy())
    w = w->focusProxy();
  return w;
}

void kMyMoneyEdit::setCalculatorButtonVisible(const bool show)
{
  m_calcButton->setVisible(show);
}

void kMyMoneyEdit::setResetButtonVisible(const bool show)
{
  m_resetButton->setVisible(show);
}

void kMyMoneyEdit::setAllowEmpty(bool allowed)
{
  allowEmpty = allowed;
}

bool kMyMoneyEdit::isCalculatorButtonVisible() const
{
  return m_calcButton->isVisible();
}

bool kMyMoneyEdit::isResetButtonVisible() const
{
  return m_resetButton->isVisible();
}

bool kMyMoneyEdit::isEmptyAllowed() const
{
  return allowEmpty;
}

void kMyMoneyEdit::setPlaceholderText(const QString& hint) const
{
  if (m_edit)
    m_edit->setPlaceholderText(hint);
}

bool kMyMoneyEdit::isReadOnly() const
{
  if (m_edit)
    return m_edit->isReadOnly();
  return false;
}

void kMyMoneyEdit::setReadOnly(bool readOnly)
{
  // we use the QLineEdit::setReadOnly() method directly to avoid
  // changing the background between readonly and read/write mode
  // as it is done by the KLineEdit code.
  if (m_edit)
    m_edit->QLineEdit::setReadOnly(readOnly); //krazy:exclude=qclasses
}
