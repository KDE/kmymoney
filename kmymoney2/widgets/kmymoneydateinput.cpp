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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpoint.h>
#include <qvalidator.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3Frame>
#include <QKeyEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes
#include "kdecompat.h"
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kshortcut.h>
#include <kpassivepopup.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydateinput.h"

namespace {
  const int DATE_POPUP_TIMEOUT = 1500;
}

bool KMyMoneyDateEdit::event(QEvent* e)
{
  // make sure that we keep the current date setting of a kMyMoneyDateInput object
  // across the QDateEdit::event(FocusOutEvent)
  bool rc;

  kMyMoneyDateInput* p = dynamic_cast<kMyMoneyDateInput*>(parentWidget());
  if(e->type() == QEvent::FocusOut && p) {
    QDate d = p->date();
    rc = Q3DateEdit::event(e);
    if(d.isValid())
      d = p->date();
    p->loadDate(d);
  } else {
    rc = Q3DateEdit::event(e);
  }
  return rc;
}

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, const char *name, Qt::AlignmentFlags flags)
 : Q3HBox(parent,name)
{
  m_qtalignment = flags;
  m_date = QDate::currentDate();

  dateEdit = new KMyMoneyDateEdit(m_date, this, "dateEdit");
  setFocusProxy(dateEdit);
  focusWidget()->installEventFilter(this); // To get dateEdit's FocusIn/Out and some KeyPress events
  dateEdit->installEventFilter(this); // To get dateEdit's FocusIn/Out and some KeyPress events

  m_datePopup = new KPassivePopup(dateEdit, "datePopup");
  m_datePopup->setTimeout(DATE_POPUP_TIMEOUT);
  m_datePopup->setView(new QLabel(KGlobal::locale()->formatDate(m_date), m_datePopup, "datePopupLabel"));

  m_dateFrame = new Q3VBox(this, 0, WType_Popup);
  m_dateFrame->setFrameStyle(Q3Frame::PopupPanel | Q3Frame::Raised);
  m_dateFrame->setLineWidth(3);
  m_dateFrame->hide();

  QString dateFormat = KGlobal::locale()->dateFormatShort().toLower();
  QString order, separator;
  for(unsigned i = 0; i < dateFormat.length(); ++i) {
    // DD.MM.YYYY is %d.%m.%y
    // dD.mM.YYYY is %e.%n.%y
    // SHORTWEEKDAY, dD SHORTMONTH YYYY is %a, %e %b %Y
    if(dateFormat[i] == 'y' || dateFormat[i] == 'm' || dateFormat[i] == 'n' || dateFormat[i] == 'd' || dateFormat[i] == 'e') {
      if(dateFormat[i] == 'n')
        dateFormat[i] = 'm';
      if(dateFormat[i] == 'e')
        dateFormat[i] = 'd';
      order += dateFormat[i];
    } else if(dateFormat[i] != '%' && separator.isEmpty())
      separator = dateFormat[i];
    if(order.length() == 3)
      break;
  }

  // see if we find a known format. If it's unknown, then we use YMD (international)
  // set m_focusDatePart to the day position (0-2)
  if(order == "mdy") {
    dateEdit->setOrder(Q3DateEdit::MDY);
    m_focusDatePart = 1;
  } else if(order == "dmy") {
    dateEdit->setOrder(Q3DateEdit::DMY);
    m_focusDatePart = 0;
  } else if(order == "ydm") {
    dateEdit->setOrder(Q3DateEdit::YDM);
    m_focusDatePart = 1;
  } else {
    dateEdit->setOrder(Q3DateEdit::YMD);
    m_focusDatePart = 2;
    separator = '-';
  }
  dateEdit->setSeparator(separator);

  m_datePicker = new KDatePicker(m_dateFrame, m_date);
#if KDE_IS_VERSION(3,1,0)
  // Let the date picker have a close button (Added in 3.1)
  m_datePicker->setCloseButton(true);
#endif

  // the next line is a try to add an icon to the button
  m_dateButton = new KPushButton(QIcon(QPixmap(KIconLoader::global()->iconPath("date", -KIconLoader::SizeSmall))), QString(""), this);
  m_dateButton->setMinimumWidth(30);

  connect(m_dateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));
  connect(dateEdit, SIGNAL(valueChanged(const QDate&)), this, SLOT(slotDateChosenRef(const QDate&)));
  connect(m_datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(m_datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(m_datePicker, SIGNAL(dateSelected(QDate)), m_dateFrame, SLOT(hide()));
}

void kMyMoneyDateInput::show(void)
{
  // don't forget the standard behaviour  ;-)
  Q3HBox::show();

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
  m_dateFrame->setFixedSize(m_datePicker->sizeHint() + QSize(22, 14));

  dateEdit->setMinimumWidth(dateEdit->minimumSizeHint().width() + 6);
}

kMyMoneyDateInput::~kMyMoneyDateInput()
{
  delete m_dateFrame;
  delete m_datePopup;
}

void kMyMoneyDateInput::toggleDatePicker()
{
  int w = m_dateFrame->width();
  int h = m_dateFrame->height();

  if(m_dateFrame->isVisible())
  {
    m_dateFrame->hide();
  }
  else
  {
    QPoint tmpPoint = mapToGlobal(m_dateButton->geometry().bottomRight());

    // usually, the datepicker widget is shown underneath the dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if(tmpPoint.y() + h > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - h - m_dateButton->height());
    }

    if((m_qtalignment == Qt::AlignRight && tmpPoint.x()+w <= QApplication::desktop()->width())
    || (tmpPoint.x()-w < 0)  )
    {
      m_dateFrame->setGeometry(tmpPoint.x()-width(), tmpPoint.y(), w, h);
    }
    else
    {
      tmpPoint.setX(tmpPoint.x() - w);
      m_dateFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    }

    if(m_date.isValid())
    {
      m_datePicker->setDate(m_date);
    }
    else
    {
      m_datePicker->setDate(QDate::currentDate());
    }
    m_dateFrame->show();
  }
}


void kMyMoneyDateInput::resizeEvent(QResizeEvent* ev)
{
  m_dateButton->setMaximumHeight(ev->size().height());
  m_dateButton->setMaximumWidth(ev->size().height());
  dateEdit->setMaximumHeight(ev->size().height());

  // qDebug("Received resize-event %d,%d", ev->size().width(), ev->size().height());
}


/** Overriding QWidget::keyPressEvent
  *
  * increments/decrements the date upon +/- or Up/Down key input
  * sets the date to current date when the 'T' key is pressed
  */
void kMyMoneyDateInput::keyPressEvent(QKeyEvent * k)
{
  KShortcut today(i18n("Enter todays date into date input widget", "T"));

  switch(k->key()) {
    case Key_Equal:
    case Key_Plus:
      slotDateChosen(m_date.addDays(1));
      break;

    case Key_Minus:
      slotDateChosen(m_date.addDays(-1));
      break;

    default:
      if(today.contains(KKey(k)) || k->key() == Key_T) {
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
    m_datePopup->show();
    // The cast to the base class is needed since setFocusSection
    // is protected in QDateEdit. This causes some logic in
    // QDateEdit::setFocusSection not to be executed but this does
    // not hurt here, because the widget just receives focus.
    static_cast<Q3DateTimeEditBase *>(dateEdit)->setFocusSection(m_focusDatePart);
  }
  else if (e->type() == QEvent::FocusOut)
    m_datePopup->hide();
  else if (e->type() == QEvent::KeyPress) {
    if (QKeyEvent *k = dynamic_cast<QKeyEvent*>(e)) {
      if (k->key() == Key_Minus) {
        keyPressEvent(k);
        return true;
      }
    }
  }

  return false; // Don't filter the event
}

void kMyMoneyDateInput::slotDateChosenRef(const QDate& date)
{
  if(date.isValid()) {
    emit dateChanged(date);
    m_date = date;

    QLabel *lbl = static_cast<QLabel*>(m_datePopup->view());
    lbl->setText(KGlobal::locale()->formatDate(date));
    lbl->adjustSize();
    if(m_datePopup->isVisible() || hasFocus())
      m_datePopup->show(); // Repaint
  }
}

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  if(date.isValid()) {
    // the next line implies a call to slotDateChosenRef() above
    dateEdit->setDate(date);
  }
}

QDate kMyMoneyDateInput::date(void) const
{
  return dateEdit->date();
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

void kMyMoneyDateInput::loadDate(const QDate& date)
{
  m_date = m_prevDate = date;

  blockSignals(true);
  dateEdit->setDate(date);
  m_date = date;
  blockSignals(false);
}

void kMyMoneyDateInput::resetDate(void)
{
  setDate(m_prevDate);
}

QWidget* kMyMoneyDateInput::focusWidget(void) const
{
  QWidget* w = dateEdit;
  while(w->focusProxy())
    w = w->focusProxy();
  return w;
}

#include "kmymoneydateinput.moc"
