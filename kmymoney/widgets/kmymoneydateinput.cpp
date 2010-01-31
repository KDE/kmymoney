/***************************************************************************
                          kmymoneydateinput.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneydateinput.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>
#include <QPoint>
#include <QValidator>
#include <QStyle>
#include <QLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QTimer>
#include <QLabel>
#include <QResizeEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QEvent>
#include <QDateEdit>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kshortcut.h>
#include <kpassivepopup.h>
#include <kdatepicker.h>
#include <kvbox.h>

// ----------------------------------------------------------------------------
// Project Includes

namespace
{
const int DATE_POPUP_TIMEOUT = 1500;
const QDate INVALID_DATE = QDate(1800, 1, 1);
}

bool KMyMoneyDateEdit::event(QEvent* e)
{
  // make sure that we keep the current date setting of a kMyMoneyDateInput object
  // across the QDateEdit::event(FocusOutEvent)
  bool rc;

  kMyMoneyDateInput* p = dynamic_cast<kMyMoneyDateInput*>(parentWidget());
  if (e->type() == QEvent::FocusOut && p) {
    QDate d = p->date();
    rc = QDateEdit::event(e);
    if (d.isValid())
      d = p->date();
    p->loadDate(d);
  } else {
    rc = QDateEdit::event(e);
  }
  return rc;
}

struct kMyMoneyDateInput::Private {
  QDateEdit *m_dateEdit;
  KDatePicker *m_datePicker;
  QDate m_date;
  QDate m_prevDate;
  Qt::AlignmentFlag m_qtalignment;
  KVBox *m_dateFrame;
  KPushButton *m_dateButton;
  KPassivePopup *m_datePopup;
};

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, Qt::AlignmentFlag flags)
    : KHBox(parent), d(new Private)
{
  d->m_qtalignment = flags;
  d->m_date = QDate::currentDate();

  d->m_dateEdit = new KMyMoneyDateEdit(d->m_date, this);
  setFocusProxy(d->m_dateEdit);
  focusWidget()->installEventFilter(this); // To get d->m_dateEdit's FocusIn/Out and some KeyPress events
  d->m_dateEdit->installEventFilter(this); // To get d->m_dateEdit's FocusIn/Out and some KeyPress events

  // we use INVALID_DATE as a special value for multi transaction editing
  d->m_dateEdit->setMinimumDate(INVALID_DATE);
  d->m_dateEdit->setSpecialValueText(QLatin1String(" "));

  d->m_datePopup = new KPassivePopup(d->m_dateEdit);
  d->m_datePopup->setObjectName("datePopup");
  d->m_datePopup->setTimeout(DATE_POPUP_TIMEOUT);
  d->m_datePopup->setView(new QLabel(KGlobal::locale()->formatDate(d->m_date), d->m_datePopup));

  d->m_dateFrame = new KVBox(this);
  d->m_dateFrame->setWindowFlags(Qt::Popup);
  d->m_dateFrame->setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  d->m_dateFrame->setLineWidth(3);
  d->m_dateFrame->hide();

  QString dateFormat = KGlobal::locale()->dateFormatShort().toLower();
  QString order, separator;
  for (int i = 0; i < dateFormat.length(); ++i) {
    // DD.MM.YYYY is %d.%m.%y
    // dD.mM.YYYY is %e.%n.%y
    // SHORTWEEKDAY, dD SHORTMONTH YYYY is %a, %e %b %Y
    if (dateFormat[i] == 'y' || dateFormat[i] == 'm' || dateFormat[i] == 'n' || dateFormat[i] == 'd' || dateFormat[i] == 'e') {
      if (dateFormat[i] == 'n')
        dateFormat[i] = 'm';
      if (dateFormat[i] == 'e')
        dateFormat[i] = 'd';
      order += dateFormat[i];
    } else if (dateFormat[i] != '%' && separator.isEmpty())
      separator = dateFormat[i];
    if (order.length() == 3)
      break;
  }

  // see if we find a known format. If it's unknown, then we use YMD (international)
  if (order == "mdy") {
    d->m_dateEdit->setDisplayFormat(QString("MM%1dd%2yyyy").arg(separator, separator));
  } else if (order == "dmy") {
    d->m_dateEdit->setDisplayFormat(QString("dd%1MM%2yyyy").arg(separator, separator));
  } else if (order == "ydm") {
    d->m_dateEdit->setDisplayFormat(QString("yyyy%1dd%2MM").arg(separator, separator));
  } else {
    d->m_dateEdit->setDisplayFormat(QString("yyyy%1MM%2dd").arg(separator, separator));
  }

  d->m_datePicker = new KDatePicker(d->m_date, d->m_dateFrame);
  // Let the date picker have a close button (Added in 3.1)
  d->m_datePicker->setCloseButton(true);

  // the next line is a try to add an icon to the button
  d->m_dateButton = new KPushButton(KIcon("view-calendar-day"), QString(""), this);
  d->m_dateButton->setMinimumWidth(30);

  connect(d->m_dateButton, SIGNAL(clicked()), SLOT(toggleDatePicker()));
  connect(d->m_dateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChosenRef(const QDate&)));
  connect(d->m_datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(d->m_datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(d->m_datePicker, SIGNAL(dateSelected(QDate)), d->m_dateFrame, SLOT(hide()));
}

void kMyMoneyDateInput::markAsBadDate(bool bad, const QColor& color)
{
  // the next line knows a bit about the internals of QAbstractSpinBox
  QLineEdit* le = d->m_dateEdit->findChild<QLineEdit *>();

  if (le) {
    QPalette palette = this->palette();
    le->setPalette(palette);
    if (bad) {
      palette.setColor(foregroundRole(), color);
      le->setPalette(palette);
    }
  }
}

void kMyMoneyDateInput::showEvent(QShowEvent* event)
{
  // don't forget the standard behaviour  ;-)
  KHBox::showEvent(event);

  // If the widget is shown, the size must be fixed a little later
  // to be appropriate. I saw this in some other places and the only
  // way to solve this problem is to postpone the setup of the size
  // to the time when the widget is on the screen.
  QTimer::singleShot(50, this, SLOT(fixSize()));
}

void kMyMoneyDateInput::fixSize(void)
{
  // According to a hint in the documentation of KDatePicker::sizeHint()
  // 28 pixels should be added in each direction to obtain a better
  // display of the month button. I decided, (22,14) is good
  // enough and save some space on the screen (ipwizard)
  d->m_dateFrame->setFixedSize(d->m_datePicker->sizeHint() + QSize(22, 14));

  d->m_dateEdit->setMinimumWidth(d->m_dateEdit->minimumSizeHint().width() + 6);
}

kMyMoneyDateInput::~kMyMoneyDateInput()
{
  delete d->m_dateFrame;
  delete d->m_datePopup;
  delete d;
}

void kMyMoneyDateInput::toggleDatePicker()
{
  int w = d->m_dateFrame->width();
  int h = d->m_dateFrame->height();

  if (d->m_dateFrame->isVisible()) {
    d->m_dateFrame->hide();
  } else {
    QPoint tmpPoint = mapToGlobal(d->m_dateButton->geometry().bottomRight());

    // usually, the datepicker widget is shown underneath the d->m_dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if (tmpPoint.y() + h > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - h - d->m_dateButton->height());
    }

    if ((d->m_qtalignment == Qt::AlignRight && tmpPoint.x() + w <= QApplication::desktop()->width())
        || (tmpPoint.x() - w < 0)) {
      d->m_dateFrame->setGeometry(tmpPoint.x() - width(), tmpPoint.y(), w, h);
    } else {
      tmpPoint.setX(tmpPoint.x() - w);
      d->m_dateFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    }

    if (d->m_date.isValid()) {
      d->m_datePicker->setDate(d->m_date);
    } else {
      d->m_datePicker->setDate(QDate::currentDate());
    }
    d->m_dateFrame->show();
  }
}


void kMyMoneyDateInput::resizeEvent(QResizeEvent* ev)
{
  d->m_dateButton->setMaximumHeight(ev->size().height());
  d->m_dateButton->setMaximumWidth(ev->size().height());
  d->m_dateEdit->setMaximumHeight(ev->size().height());

  // qDebug("Received resize-event %d,%d", ev->size().width(), ev->size().height());
}


/** Overriding QWidget::keyPressEvent
  *
  * increments/decrements the date upon +/- or Up/Down key input
  * sets the date to current date when the 'T' key is pressed
  */
void kMyMoneyDateInput::keyPressEvent(QKeyEvent * k)
{
  KShortcut today(i18nc("Enter todays date into date input widget", "T"));

  switch (k->key()) {
  case Qt::Key_Equal:
  case Qt::Key_Plus:
    slotDateChosen(d->m_date.addDays(1));
    break;

  case Qt::Key_Minus:
    slotDateChosen(d->m_date.addDays(-1));
    break;

  default:
    if (today.contains(QKeySequence(k->key())) || k->key() == Qt::Key_T) {
      slotDateChosen(QDate::currentDate());
    }
    break;
  }
}

/**
  * This function receives all events that are sent to focusWidget().
  * Some KeyPress events are intercepted and passed to keyPressEvent.
  * Otherwise they would be consumed by QDateEdit.
  */
bool kMyMoneyDateInput::eventFilter(QObject *, QEvent *e)
{
  if (e->type() == QEvent::FocusIn) {
    d->m_datePopup->show(mapToGlobal(QPoint(0, height())));
    // select the date section
    d->m_dateEdit->setSelectedSection(QDateTimeEdit::DaySection);
  } else if (e->type() == QEvent::FocusOut)
    d->m_datePopup->hide();
  else if (e->type() == QEvent::KeyPress) {
    if (QKeyEvent *k = dynamic_cast<QKeyEvent*>(e)) {
      if (k->key() == Qt::Key_Minus) {
        keyPressEvent(k);
        return true;
      }
    }
  }

  return false; // Don't filter the event
}

void kMyMoneyDateInput::slotDateChosenRef(const QDate& date)
{
  if (date.isValid()) {
    emit dateChanged(date);
    d->m_date = date;

    QLabel *lbl = static_cast<QLabel*>(d->m_datePopup->view());
    lbl->setText(KGlobal::locale()->formatDate(date));
    lbl->adjustSize();
    if (d->m_datePopup->isVisible() || hasFocus())
      d->m_datePopup->show(mapToGlobal(QPoint(0, height()))); // Repaint
  }
}

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  if (date.isValid()) {
    // the next line implies a call to slotDateChosenRef() above
    d->m_dateEdit->setDate(date);
  } else {
    d->m_dateEdit->setDate(INVALID_DATE);
  }
}

QDate kMyMoneyDateInput::date(void) const
{
  QDate rc = d->m_dateEdit->date();
  if(rc == INVALID_DATE)
    rc = QDate();
  return rc;
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

void kMyMoneyDateInput::loadDate(const QDate& date)
{
  d->m_date = d->m_prevDate = date;

  blockSignals(true);
  slotDateChosen(date);
  blockSignals(false);
}

void kMyMoneyDateInput::resetDate(void)
{
  setDate(d->m_prevDate);
}

QWidget* kMyMoneyDateInput::focusWidget(void) const
{
  QWidget* w = d->m_dateEdit;
  while (w->focusProxy())
    w = w->focusProxy();
  return w;
}
/*
void kMyMoneyDateInput::setRange(const QDate & min, const QDate & max)
{
  d->m_dateEdit->setDateRange(min, max);
}
*/
#include "kmymoneydateinput.moc"
