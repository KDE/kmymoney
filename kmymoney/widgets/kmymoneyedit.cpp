/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
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

#include "kmymoneyedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QValidator>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneymoneyvalidator.h"
#include "kmymoneylineedit.h"
#include "kmymoneycalculator.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "icons.h"

using namespace Icons;

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

class KMyMoneyEditPrivate
{
  Q_DISABLE_COPY(KMyMoneyEditPrivate)
  Q_DECLARE_PUBLIC(KMyMoneyEdit)

public:
  explicit KMyMoneyEditPrivate(KMyMoneyEdit *qq) :
    q_ptr(qq),
    m_calculator(nullptr),
    m_calculatorFrame(nullptr),
    m_edit(nullptr),
    m_calcButton(nullptr),
    m_resetButton(nullptr),
    m_prec(2),
    allowEmpty(true)
  {
  }

  ~KMyMoneyEditPrivate()
  {
  }

  /**
  * Force geometry update of a hidden widget,
  * see https://stackoverflow.com/a/3996525 for details
  *
  * @parem widget widget to force update
  */
  void forceUpdate(QWidget *widget)
  {
      widget->setAttribute(Qt::WA_DontShowOnScreen);
      widget->show();
      widget->updateGeometry();
      widget->hide();
      widget->setAttribute(Qt::WA_DontShowOnScreen, false);
  }

  void init()
  {
    Q_Q(KMyMoneyEdit);
    QHBoxLayout *editLayout = new QHBoxLayout(q);
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);

    allowEmpty = false;
    m_edit = new KMyMoneyLineEdit(q, true);
    m_edit->installEventFilter(q);
    q->setFocusProxy(m_edit);
    editLayout->addWidget(m_edit);

    // Yes, just a simple double validator !
    KMyMoneyMoneyValidator *validator = new KMyMoneyMoneyValidator(q);
    m_edit->setValidator(validator);
    m_edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_calculatorFrame = new QWidget;
    QVBoxLayout *calculatorFrameVBoxLayout = new QVBoxLayout(m_calculatorFrame);
    calculatorFrameVBoxLayout->setMargin(0);
    m_calculatorFrame->setWindowFlags(Qt::Popup);

    m_calculator = new KMyMoneyCalculator(m_calculatorFrame);
    calculatorFrameVBoxLayout->addWidget(m_calculator);
    forceUpdate(m_calculatorFrame);

    m_calcButton = new QPushButton(Icons::get(Icon::AccessoriesCalculator), QString(), q);
    m_calcButton->setFocusProxy(m_edit);
    editLayout->addWidget(m_calcButton);

    QPixmap pixmap;
    pixmap.loadFromData(resetButtonImage, sizeof(resetButtonImage), "PNG", 0);
    m_resetButton = new QPushButton(pixmap, QString(QString()), q);
    m_resetButton->setEnabled(false);
    m_resetButton->setFocusProxy(m_edit);
    editLayout->addWidget(m_resetButton);

    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    KConfigGroup grp = kconfig->group("General Options");
    if (grp.readEntry("DontShowCalculatorButton", false) == true)
      q->setCalculatorButtonVisible(false);

    q->connect(m_edit, &QLineEdit::textChanged, q, &KMyMoneyEdit::theTextChanged);
    q->connect(m_calculator, &KMyMoneyCalculator::signalResultAvailable, q, &KMyMoneyEdit::slotCalculatorResult);
    q->connect(m_calcButton, &QAbstractButton::clicked, q, &KMyMoneyEdit::slotCalculatorOpen);
    q->connect(m_resetButton, &QAbstractButton::clicked, q, &KMyMoneyEdit::resetText);
  }

  /**
    * This method ensures that the text version contains a
    * fractional part.
    */
  void ensureFractionalPart()
  {
    QString s(m_edit->text());
    ensureFractionalPart(s);
    // by setting the text only when it's different then the one that it is already there
    // we preserve the edit widget's state (like the selection for example) during a
    // call to ensureFractionalPart() that does not change anything
    if (s != m_edit->text())
      m_edit->setText(s);
  }

  /**
    * Internal helper function for value() and ensureFractionalPart().
    */
  void ensureFractionalPart(QString& s) const
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
          for (auto i = 0; i < m_prec; ++i)
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

  /**
    * This method opens the calculator and replays the key
    * event pointed to by @p ev. If @p ev is 0, then no key
    * event is replayed.
    *
    * @param ev pointer to QKeyEvent that started the calculator.
    */
  void calculatorOpen(QKeyEvent* k)
  {
    Q_Q(KMyMoneyEdit);
    m_calculator->setInitialValues(m_edit->text(), k);

    int h = m_calculatorFrame->height();
    int w = m_calculatorFrame->width();

    // usually, the calculator widget is shown underneath the MoneyEdit widget
    // if it does not fit on the screen, we show it above this widget
    QPoint p = q->mapToGlobal(QPoint(0, 0));
    if (p.y() + q->height() + h > QApplication::desktop()->height())
      p.setY(p.y() - h);
    else
      p.setY(p.y() + q->height());

    // usually, it is shown left aligned. If it does not fit, we align it
    // to the right edge of the widget
    if (p.x() + w > QApplication::desktop()->width())
      p.setX(p.x() + q->width() - w);

    QRect r = m_calculator->geometry();
    r.moveTopLeft(p);
    m_calculatorFrame->setGeometry(r);
    m_calculatorFrame->show();
    m_calculator->setFocus();
  }

  KMyMoneyEdit       *q_ptr;
  QString previousText; // keep track of what has been typed
  QString m_text;       // keep track of what was the original value
  KMyMoneyCalculator* m_calculator;
  QWidget*            m_calculatorFrame;
  KMyMoneyLineEdit*   m_edit;
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
};

int KMyMoneyEditPrivate::standardPrecision = 2;

KMyMoneyEdit::KMyMoneyEdit(QWidget *parent, const int prec) :
  QWidget(parent),
  d_ptr(new KMyMoneyEditPrivate(this))
{
  Q_D(KMyMoneyEdit);
  d->m_prec = prec;
  if (prec < -1 || prec > 20)
    d->m_prec = KMyMoneyEditPrivate::standardPrecision;
  d->init();
}

KMyMoneyEdit::KMyMoneyEdit(const MyMoneySecurity& sec, QWidget *parent) :
  QWidget(parent),
  d_ptr(new KMyMoneyEditPrivate(this))
{
  Q_D(KMyMoneyEdit);
  d->m_prec = MyMoneyMoney::denomToPrec(sec.smallestAccountFraction());
  d->init();
}

void KMyMoneyEdit::setStandardPrecision(int prec)
{
  if (prec >= 0 && prec < 20) {
    KMyMoneyEditPrivate::standardPrecision = prec;
  }
}

void KMyMoneyEdit::setValidator(const QValidator* v)
{
  Q_D(KMyMoneyEdit);
  d->m_edit->setValidator(v);
}

KMyMoneyEdit::~KMyMoneyEdit()
{
  Q_D(KMyMoneyEdit);
  delete d;
}

KLineEdit* KMyMoneyEdit::lineedit() const
{
  Q_D(const KMyMoneyEdit);
  return d->m_edit;
}

QString KMyMoneyEdit::text() const
{
  return value().toString();
}

void KMyMoneyEdit::setMinimumWidth(int w)
{
  Q_D(KMyMoneyEdit);
  d->m_edit->setMinimumWidth(w);
}

void KMyMoneyEdit::setPrecision(const int prec)
{
  Q_D(KMyMoneyEdit);
  if (prec >= -1 && prec <= 20) {
    if (prec != d->m_prec) {
      d->m_prec = prec;
      // update current display
      setValue(value());
    }
  }
}

int KMyMoneyEdit::precision() const
{
  Q_D(const KMyMoneyEdit);
  return d->m_prec;
}

bool KMyMoneyEdit::isValid() const
{
  Q_D(const KMyMoneyEdit);
  return !(d->m_edit->text().isEmpty());
}

MyMoneyMoney KMyMoneyEdit::value() const
{
  Q_D(const KMyMoneyEdit);
  auto txt = d->m_edit->text();
  d->ensureFractionalPart(txt);
  MyMoneyMoney money(txt);
  if (d->m_prec != -1)
    money = money.convert(MyMoneyMoney::precToDenom(d->m_prec));
  return money;
}

void KMyMoneyEdit::setValue(const MyMoneyMoney& value)
{
  Q_D(KMyMoneyEdit);
  // load the value into the widget but don't use thousandsSeparators
  auto txt = value.formatMoney(QString(), d->m_prec, false);
  loadText(txt);
}

void KMyMoneyEdit::loadText(const QString& txt)
{
  Q_D(KMyMoneyEdit);
  d->m_edit->setText(txt);
  if (isEnabled() && !txt.isEmpty())
    d->ensureFractionalPart();
  d->m_text = d->m_edit->text();
  d->m_resetButton->setEnabled(false);
}

void KMyMoneyEdit::clearText()
{
  Q_D(KMyMoneyEdit);
  d->m_text.clear();
  d->m_edit->setText(d->m_text);
}

void KMyMoneyEdit::setText(const QString& txt)
{
  setValue(MyMoneyMoney(txt));
}

void KMyMoneyEdit::resetText()
{
  Q_D(KMyMoneyEdit);
  d->m_edit->setText(d->m_text);
  d->m_resetButton->setEnabled(false);
}

void KMyMoneyEdit::theTextChanged(const QString & theText)
{
  Q_D(KMyMoneyEdit);
  QString txt = QLocale().decimalPoint();
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
  psign = QString();
#endif
  auto i = 0;
  if (isEnabled()) {
    QValidator::State state =  d->m_edit->validator()->validate(l_text, i);
    if (state == QValidator::Intermediate) {
      if (l_text.length() == 1) {
        if (l_text != txt && l_text != nsign && l_text != psign)
          state = QValidator::Invalid;
      }
    }
    if (state == QValidator::Invalid)
      d->m_edit->setText(d->previousText);
    else {
      d->previousText = l_text;
      emit textChanged(d->m_edit->text());
      d->m_resetButton->setEnabled(true);
    }
  }
}

bool KMyMoneyEdit::eventFilter(QObject * /* o */ , QEvent *event)
{
  Q_D(KMyMoneyEdit);
  auto rc = false;

  // we want to catch some keys that are usually handled by
  // the base class (e.g. '+', '-', etc.)
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *>(event);

    rc = true;
    switch (k->key()) {
      case Qt::Key_Plus:
      case Qt::Key_Minus:
        if (d->m_edit->hasSelectedText()) {
          d->m_edit->cut();
        }
        if (d->m_edit->text().length() == 0) {
          rc = false;
          break;
        }
        // in case of '-' we do not enter the calculator when
        // the current position is the beginning and there is
        // no '-' sign at the first position.
        if (k->key() == Qt::Key_Minus) {
          if (d->m_edit->cursorPosition() == 0 && d->m_edit->text()[0] != '-') {
            rc = false;
            break;
          }
        }
        // intentional fall through

      case Qt::Key_Slash:
      case Qt::Key_Asterisk:
      case Qt::Key_Percent:
        if (d->m_edit->hasSelectedText()) {
          // remove the selected text
          d->m_edit->cut();
        }
        d->calculatorOpen(k);
        break;

      default:
        rc = false;
        break;
    }

  } else if (event->type() == QEvent::FocusOut) {
    if (!d->m_edit->text().isEmpty() || !d->allowEmpty)
      d->ensureFractionalPart();

    if (MyMoneyMoney(d->m_edit->text()) != MyMoneyMoney(d->m_text)
        && !d->m_calculator->isVisible()) {
      emit valueChanged(d->m_edit->text());
    }
    d->m_text = d->m_edit->text();
  }
  return rc;
}

void KMyMoneyEdit::slotCalculatorOpen()
{
  Q_D(KMyMoneyEdit);
  d->calculatorOpen(0);
}

void KMyMoneyEdit::slotCalculatorResult()
{
  Q_D(KMyMoneyEdit);
  if (d->m_calculator != 0) {
    d->m_calculatorFrame->hide();
    d->m_edit->setText(d->m_calculator->result());
    d->ensureFractionalPart();
    emit valueChanged(d->m_edit->text());
    d->m_text = d->m_edit->text();
  }
}

QWidget* KMyMoneyEdit::focusWidget() const
{
  Q_D(const KMyMoneyEdit);
  QWidget* w = d->m_edit;
  while (w->focusProxy())
    w = w->focusProxy();
  return w;
}

void KMyMoneyEdit::setCalculatorButtonVisible(const bool show)
{
  Q_D(KMyMoneyEdit);
  d->m_calcButton->setVisible(show);
}

void KMyMoneyEdit::setResetButtonVisible(const bool show)
{
  Q_D(KMyMoneyEdit);
  d->m_resetButton->setVisible(show);
}

void KMyMoneyEdit::setAllowEmpty(bool allowed)
{
  Q_D(KMyMoneyEdit);
  d->allowEmpty = allowed;
}

bool KMyMoneyEdit::isCalculatorButtonVisible() const
{
  Q_D(const KMyMoneyEdit);
  return d->m_calcButton->isVisible();
}

bool KMyMoneyEdit::isResetButtonVisible() const
{
  Q_D(const KMyMoneyEdit);
  return d->m_resetButton->isVisible();
}

bool KMyMoneyEdit::isEmptyAllowed() const
{
  Q_D(const KMyMoneyEdit);
  return d->allowEmpty;
}

void KMyMoneyEdit::setPlaceholderText(const QString& hint) const
{
  Q_D(const KMyMoneyEdit);
  if (d->m_edit)
    d->m_edit->setPlaceholderText(hint);
}

bool KMyMoneyEdit::isReadOnly() const
{
  Q_D(const KMyMoneyEdit);
  if (d->m_edit)
    return d->m_edit->isReadOnly();
  return false;
}

void KMyMoneyEdit::setReadOnly(bool readOnly)
{
  Q_D(KMyMoneyEdit);
  // we use the QLineEdit::setReadOnly() method directly to avoid
  // changing the background between readonly and read/write mode
  // as it is done by the KLineEdit code.
  if (d->m_edit)
    d->m_edit->QLineEdit::setReadOnly(readOnly); //krazy:exclude=qclasses
}
