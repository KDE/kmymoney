/***************************************************************************
                          kmymoneybriefschedule.h  -  description
                             -------------------
    begin                : Sun Jul 6 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
#ifndef KMYMONEYBRIEFSCHEDULE_H
#define KMYMONEYBRIEFSCHEDULE_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;
class MyMoneySchedule;

template <typename T> class QList;

/**
  *@author Michael Edwardes
  */

class KMyMoneyBriefSchedulePrivate;
class KMyMoneyBriefSchedule : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyBriefSchedule)

public:
  explicit KMyMoneyBriefSchedule(QWidget* parent = nullptr);
  ~KMyMoneyBriefSchedule();
  void setSchedules(QList<MyMoneySchedule> list, const QDate& date);

signals:
  void enterClicked(const MyMoneySchedule&, const QDate&);
  void skipClicked(const MyMoneySchedule&, const QDate&);

protected slots:
  void slotPrevClicked();
  void slotNextClicked();
  void slotEnterClicked();
  void slotSkipClicked();

private:
  KMyMoneyBriefSchedulePrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyBriefSchedule)
};

#endif
