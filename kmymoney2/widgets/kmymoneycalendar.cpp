/***************************************************************************
                          kmymoneycalendar.cpp  -  description
                             -------------------
    begin                : Wed Jul 2 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 * Contains code from KDatePicker in kdelibs-3.1.2.
 * Original license message:
 *
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
 ****************************************************************************/

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
#include <q3frame.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qvalidator.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes
#include "kdecompat.h"
#include <kglobal.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kdebug.h>
#include <knotification.h>
#include <kdatetable.h> // for maximum re-use
#include <kmenu.h>

#if KDE_IS_VERSION(3,2,0)
#include <kcalendarsystem.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneycalendar.h"
#include "kmymoneydatetbl.h"

class kMyMoneyCalendar::kMyMoneyCalendarPrivate
{
public:
    kMyMoneyCalendarPrivate()
      : closeButton(0L), selectWeek(0L), userButton1(0), userButton2(0) {}

    QToolButton *closeButton;
    QToolButton *selectWeek;
    QPushButton *userButton1;
    QPushButton *userButton2;
};

kMyMoneyCalendar::kMyMoneyCalendar(QWidget *parent, const char *name ) :
  Q3Frame(parent,name),
  table(0),
  d(new kMyMoneyCalendarPrivate)
{
}

kMyMoneyCalendar::~kMyMoneyCalendar()
{
  delete d;
}

void kMyMoneyCalendar::init( const QDate &dt )
{
  styleControl = new QPushButton(i18n("Select Style"), this);
  yearForward = new QToolButton(this);
  yearBackward = new QToolButton(this);
  monthForward = new QToolButton(this);
  monthBackward = new QToolButton(this);
  selectMonth = new QToolButton(this);
  selectYear = new QToolButton(this);
  line = new KLineEdit(this);
  val = new KDateValidator(this);
  fontsize = 10;

  d->selectWeek = new QToolButton( this );

//  KIconLoader *kiconloader = KIconLoader::global();
  KMenu* kpopupmenuNew = new KMenu(this);
  kpopupmenuNew->insertItem(i18n("Week"), this, SLOT(slotSetStyleWeekly()));
  kpopupmenuNew->insertItem(i18n("Month"), this, SLOT(slotSetStyleMonthly()));
/*  kpopupmenuNew->insertItem(i18n("3 Months"), this, SLOT(slotSetStyleQuarterly())); */
  styleControl->setPopup(kpopupmenuNew);

  QToolTip::add(styleControl, i18n("Choose Style"));
  QToolTip::add(yearForward, i18n("Next year"));
  QToolTip::add(yearBackward, i18n("Previous year"));
  QToolTip::add(monthForward, i18n("Next month"));
  QToolTip::add(monthBackward, i18n("Previous month"));
  QToolTip::add(d->selectWeek, i18n("Select a week"));
  QToolTip::add(selectMonth, i18n("Select a month"));
  QToolTip::add(selectYear, i18n("Select a year"));

  // -----
  setFontSize(10);
  line->setValidator(val);
  line->installEventFilter( this );
  yearForward->setPixmap(BarIcon(QString::fromLatin1("2rightarrow")));
  yearBackward->setPixmap(BarIcon(QString::fromLatin1("2leftarrow")));
  monthForward->setPixmap(BarIcon(QString::fromLatin1("1rightarrow")));
  monthBackward->setPixmap(BarIcon(QString::fromLatin1("1leftarrow")));
  setDate(dt); // set button texts
  connect(table, SIGNAL(dateChanged(QDate)), SLOT(dateChangedSlot(QDate)));
  connect(table, SIGNAL(tableClicked()), SLOT(tableClickedSlot()));
  connect(monthForward, SIGNAL(clicked()), SLOT(monthForwardClicked()));
  connect(monthBackward, SIGNAL(clicked()), SLOT(monthBackwardClicked()));
  connect(yearForward, SIGNAL(clicked()), SLOT(yearForwardClicked()));
  connect(yearBackward, SIGNAL(clicked()), SLOT(yearBackwardClicked()));
  connect(d->selectWeek, SIGNAL(clicked()), SLOT(selectWeekClicked()));
  connect(selectMonth, SIGNAL(clicked()), SLOT(selectMonthClicked()));
  connect(selectYear, SIGNAL(clicked()), SLOT(selectYearClicked()));
  connect(line, SIGNAL(returnPressed()), SLOT(lineEnterPressed()));
  if (table)
    table->setFocus();
}

bool
kMyMoneyCalendar::eventFilter(QObject *o, QEvent *e )
{
   if ( e->type() == QEvent::KeyPress ) {
      QKeyEvent *k = (QKeyEvent *)e;

      if ( (k->key() == Qt::Key_PageUp) ||
           (k->key() == Qt::Key_PageDown)  ||
           (k->key() == Qt::Key_Up)    ||
           (k->key() == Qt::Key_Down) )
       {
          QApplication::sendEvent( table, e );
          table->setFocus();
          return TRUE; // eat event
       }
   }
   return Q3Frame::eventFilter( o, e );
}

void
kMyMoneyCalendar::resizeEvent(QResizeEvent*)
{
    QWidget *buttons[] = {
      styleControl,
      d->userButton1,
      d->userButton2,
      yearBackward,
      monthBackward,
      selectMonth,
      selectYear,
      monthForward,
      yearForward,
      d->closeButton
    };
    const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
    QSize sizes[NoOfButtons];
    int buttonHeight=0;
    int count;
    int w;
    int x=0;
    // ----- calculate button row height:
    for(count=0; count<NoOfButtons; ++count) {
        if ( buttons[count] ) { // closeButton may be 0L
            sizes[count]=buttons[count]->sizeHint();
            buttonHeight=qMax(buttonHeight, sizes[count].height());
        }
        else
            sizes[count] = QSize(0,0); // closeButton
    }

    // ----- calculate size of the month button:
    for(count=0; count<NoOfButtons; ++count) {
  if(buttons[count]==selectMonth) {
      QSize metricBound = style().sizeFromContents(QStyle::CT_ToolButton, selectMonth, maxMonthRect);
      sizes[count].setWidth(qMax(metricBound.width(), maxMonthRect.width()+2*QApplication::style().pixelMetric(QStyle::PM_ButtonMargin)));
  }
    }
    // ----- place the buttons:
    // Put the style button and user buttons to the left and the rest to the right
    x = 0;
    int noUserButtons=2;
    buttons[0]->setGeometry(x, 0, sizes[0].width(), buttonHeight);
    x += sizes[0].width();
    for (count=1; count<=noUserButtons; ++count)
    {
      if (buttons[count])
      {
        buttons[count]->setGeometry(x, 0, sizes[count].width(), buttonHeight);
        x += sizes[count].width();
      }
    }

    x = width();
    for(count=(1+noUserButtons); count<NoOfButtons; ++count)
    {
      w=sizes[count].width();
      x -= w;
    }

    for(count=(1+noUserButtons); count<NoOfButtons; ++count)
    {
      w=sizes[count].width();
      if ( buttons[count] )
        buttons[count]->setGeometry(x, 0, w, buttonHeight);
      x+=w;
    }

    // ----- place the line edit for direct input:
    sizes[0]=line->sizeHint();
    int week_width=d->selectWeek->fontMetrics().width(i18n("Week XX"))+((d->closeButton != 0L) ? 50 : 20);
    line->setGeometry(0, height()-sizes[0].height(), width()-week_width, sizes[0].height());
    d->selectWeek->setGeometry(width()-week_width, height()-sizes[0].height(), week_width, sizes[0].height());
    // ----- adjust the table:
    table->setGeometry(0, buttonHeight, width(),
    height()-buttonHeight-sizes[0].height());

    table->setFocus();
}

void
kMyMoneyCalendar::dateChangedSlot(QDate date)
{
    kDebug() << "kMyMoneyCalendar::dateChangedSlot: date changed (" << date.year() << "/" << date.month() << "/" << date.day() << ").";
    line->setText(KGlobal::locale()->formatDate(date, true));
    d->selectWeek->setText(i18n("Week %1").arg(weekOfYear(date)));
    selectMonth->setText(MONTH_NAME(date.month(), date.year(), false));
    selectYear->setText(date.toString("yyyy"));
    emit(dateChanged(date));
}

void
kMyMoneyCalendar::tableClickedSlot()
{
  kDebug() << "kMyMoneyCalendar::tableClickedSlot: table clicked.";
  emit(dateSelected(table->getDate()));
  emit(tableClicked());
}

const QDate&
kMyMoneyCalendar::getDate() const
{
  return table->getDate();
}

const QDate &
kMyMoneyCalendar::date() const
{
    return table->getDate();
}

bool
kMyMoneyCalendar::setDate(const QDate& date)
{
  if (!table)
    return true;  // hack

    if(date.isValid()) {
  QString temp;
  // -----
  table->setDate(date);
  d->selectWeek->setText(i18n("Week %1").arg(weekOfYear(date)));
  selectMonth->setText(MONTH_NAME(date.month(), date.year(), false));
  temp.setNum(date.year());
  selectYear->setText(temp);
  line->setText(KGlobal::locale()->formatDate(date, true));
  return true;
    } else {
  kDebug() << "kMyMoneyCalendar::setDate: refusing to set invalid date.";
  return false;
    }
}

void
kMyMoneyCalendar::monthForwardClicked()
{
    setDate( table->getDate().addMonths(1) );
}

void
kMyMoneyCalendar::monthBackwardClicked()
{
    setDate( table->getDate().addMonths(-1) );
}

void
kMyMoneyCalendar::yearForwardClicked()
{
    setDate( table->getDate().addYears(1) );
}

void
kMyMoneyCalendar::yearBackwardClicked()
{
    setDate( table->getDate().addYears(-1) );
}

void
kMyMoneyCalendar::selectWeekClicked()
{
#if KDE_VERSION >= 310 && KDE_VERSION <= 314
  int week;
  KPopupFrame* popup = new KPopupFrame(this);
  KDateInternalWeekSelector* picker = new KDateInternalWeekSelector(/*fontsize, */popup);
  // -----
  picker->resize(picker->sizeHint());
  popup->setMainWidget(picker);
  connect(picker, SIGNAL(closeMe(int)), popup, SLOT(close(int)));
  picker->setFocus();
  if(popup->exec(d->selectWeek->mapToGlobal(QPoint(0, d->selectWeek->height()))))
    {
      QDate date;
      int year;
      // -----
      week=picker->getWeek();
      date=table->getDate();
      year=date.year();
      // ----- find the first selectable day in this week (hacky solution :)
      date.setYMD(year, 1, 1);
      while (weekOfYear(date)>50)
          date=date.addDays(1);
      while (weekOfYear(date)<week && (week!=53 || (week==53 &&
            (weekOfYear(date)!=52 || weekOfYear(date.addDays(1))!=1))))
          date=date.addDays(1);
      if (week==53 && weekOfYear(date)==52)
          while (weekOfYear(date.addDays(-1))==52)
              date=date.addDays(-1);
      // ----- set this date
      setDate(date);
    } else {
         KNotifyClient::beep();
    }
  delete popup;
#endif
}

void
kMyMoneyCalendar::selectMonthClicked()
{
#if KDE_VERSION >= 310 && KDE_VERSION <= 314
  int month;
  KPopupFrame* popup = new KPopupFrame(this);
  KDateInternalMonthPicker* picker = new KDateInternalMonthPicker(/*fontsize, */popup);
  // -----
  picker->resize(picker->sizeHint());
  popup->setMainWidget(picker);
  picker->setFocus();
  connect(picker, SIGNAL(closeMe(int)), popup, SLOT(close(int)));
  if(popup->exec(selectMonth->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      month=picker->getResult();
      date=table->getDate();
      day=date.day();
      // ----- construct a valid date in this month:
      date.setYMD(date.year(), month, 1);
      date.setYMD(date.year(), month, qMin(day, date.daysInMonth()));
      // ----- set this month
      setDate(date);
    } else {
      KNotifyClient::beep();
    }
  delete popup;
#endif
}

void
kMyMoneyCalendar::selectYearClicked()
{
#if KDE_VERSION >= 310 && KDE_VERSION <= 314
  int year;
  KPopupFrame* popup = new KPopupFrame(this);
  KDateInternalYearSelector* picker = new KDateInternalYearSelector(fontsize, popup);
  // -----
  picker->resize(picker->sizeHint());
  popup->setMainWidget(picker);
  connect(picker, SIGNAL(closeMe(int)), popup, SLOT(close(int)));
  picker->setFocus();
  if(popup->exec(selectYear->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      year=picker->getYear();
      date=table->getDate();
      day=date.day();
      // ----- construct a valid date in this month:
      date.setYMD(year, date.month(), 1);
      date.setYMD(year, date.month(), qMin(day, date.daysInMonth()));
      // ----- set this month
      setDate(date);
    } else {
      KNotifyClient::beep();
    }
  delete popup;
#endif
}

void
kMyMoneyCalendar::setEnabled(bool enable)
{
  QWidget *widgets[]= {
    styleControl, yearForward, yearBackward, monthForward, monthBackward,
    selectMonth, selectYear,
    line, table, d->selectWeek, d->userButton1, d->userButton2 };
  const int Size=sizeof(widgets)/sizeof(widgets[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      if (widgets[count])
        widgets[count]->setEnabled(enable);
    }
}

void
kMyMoneyCalendar::lineEnterPressed()
{
  QDate temp;
  // -----
  if(val->date(line->text(), temp)==QValidator::Acceptable)
    {
  kDebug() << "kMyMoneyCalendar::lineEnterPressed: valid date entered.";
  emit(dateEntered(temp));
  setDate(temp);
    } else {
      KNotifyClient::beep();
      kDebug() << "kMyMoneyCalendar::lineEnterPressed: invalid date entered.";
    }
}

QSize
kMyMoneyCalendar::sizeHint() const
{
  QSize tableSize=table->sizeHint();
  QWidget *buttons[]={
    styleControl,
    yearBackward,
    monthBackward,
    selectMonth,
    selectYear,
    monthForward,
    yearForward,
    d->closeButton,
    d->userButton1,
    d->userButton2
  };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  QSize sizes[NoOfButtons];
  int cx=0, cy=0, count;
  // ----- store the size hints:
  for(count=0; count<NoOfButtons; ++count)
    {
      if ( buttons[count] )
          sizes[count]=buttons[count]->sizeHint();
      else
          sizes[count] = QSize(0,0);

      if(buttons[count]==selectMonth)
  {
    QSize metricBound = style().sizeFromContents(QStyle::CT_ToolButton, selectMonth, maxMonthRect);
    cx+=qMax(metricBound.width(), maxMonthRect.width()+2*QApplication::style().pixelMetric(QStyle::PM_ButtonMargin));
  } else {
    cx+=sizes[count].width();
  }
      cy=qMax(sizes[count].height(), cy);
    }
  // ----- calculate width hint:
  cx=qMax(cx, tableSize.width()); // line edit ignored
  // ----- calculate height hint:
  cy+=tableSize.height()+line->sizeHint().height();
  return QSize(cx, cy);
}

void
kMyMoneyCalendar::setFontSize(int s)
{
  if (table)
  {
    QWidget *buttons[]= {
      // styleControl
      // yearBackward,
      // monthBackward,
      selectMonth,
      selectYear,
      // monthForward,
      // yearForward
    };
    const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
    int count;
    QFont font;
    QRect r;
    // -----
    fontsize=s;
    for(count=0; count<NoOfButtons; ++count)
      {
        font=buttons[count]->font();
        font.setPointSize(s);
        buttons[count]->setFont(font);
      }
    QFontMetrics metrics(selectMonth->fontMetrics());
    for(int i=1; i <= 12; ++i)
      { // maxMonthRect is used by sizeHint()
        r=metrics.boundingRect(MONTH_NAME(i, 2000, false));
        maxMonthRect.setWidth(qMax(r.width(), maxMonthRect.width()));
        maxMonthRect.setHeight(qMax(r.height(),  maxMonthRect.height()));
      }
    table->setFontSize(s);
  }
}

void
kMyMoneyCalendar::setCloseButton( bool enable )
{
    if ( enable == (d->closeButton != 0L) )
        return;

    if ( enable ) {
        d->closeButton = new QToolButton( this );
        QToolTip::add(d->closeButton, i18n("Close"));
        d->closeButton->setPixmap( SmallIcon("remove") );
        connect( d->closeButton, SIGNAL( clicked() ),
                 topLevelWidget(), SLOT( close() ) );
    }
    else {
        delete d->closeButton;
        d->closeButton = 0L;
    }

    updateGeometry();
}

bool kMyMoneyCalendar::hasCloseButton() const
{
    return (d->closeButton != 0L);
}

int kMyMoneyCalendar::weekOfYear(QDate date)
{
    // Calculate ISO 8601 week number (taken from glibc/Gnumeric)
    int year, week, wday, jan1wday, nextjan1wday;
    QDate jan1date, nextjan1date;

    year=date.year();
    wday=date.dayOfWeek();

    jan1date=QDate(year,1,1);
    jan1wday=jan1date.dayOfWeek();

    week = (date.dayOfYear()-1 + jan1wday-1)/7 + ((jan1wday-1) == 0 ? 1 : 0);

    /* Does date belong to last week of previous year? */
    if ((week == 0) && (jan1wday > 4 /*THURSDAY*/)) {
        QDate tmpdate=QDate(year-1,12,31);
        return weekOfYear(tmpdate);
    }

    if ((jan1wday <= 4 /*THURSDAY*/) && (jan1wday > 1 /*MONDAY*/))
        week++;

    if (week == 53) {
        nextjan1date=QDate(year+1, 1, 1);
        nextjan1wday = nextjan1date.dayOfWeek();
        if (nextjan1wday <= 4 /*THURSDAY*/)
            week = 1;
    }

    return week;
}

void kMyMoneyCalendar::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

void kMyMoneyCalendar::slotSetStyleWeekly()
{
  setType(kMyMoneyDateTbl::WEEKLY);
}

void kMyMoneyCalendar::slotSetStyleMonthly()
{
  setType(kMyMoneyDateTbl::MONTHLY);
}

void kMyMoneyCalendar::slotSetStyleQuarterly()
{
  setType(kMyMoneyDateTbl::QUARTERLY);
}

void kMyMoneyCalendar::setUserButton1(bool enable, QPushButton* pb)
{
    if ( enable == (d->userButton1 != 0L) )
        return;

    if ( enable ) {
        d->userButton1 = pb;
    }
    else {
        delete d->userButton1;
        d->userButton1 = 0L;
    }

    updateGeometry();
}

void kMyMoneyCalendar::setUserButton2(bool enable, QPushButton* pb)
{
    if ( enable == (d->userButton2 != 0L) )
        return;

    if ( enable ) {
        d->userButton2 = pb;
    }
    else {
        delete d->userButton2;
        d->userButton2 = 0L;
    }

    updateGeometry();
}

#include "kmymoneycalendar.moc"
