/*
 * Copyright 2000-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2003-2012  Thomas Baumgart <tbaumgart@kde.org>
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

Q_SIGNALS:
  void enterClicked(const MyMoneySchedule&, const QDate&);
  void skipClicked(const MyMoneySchedule&, const QDate&);

protected Q_SLOTS:
  void slotPrevClicked();
  void slotNextClicked();
  void slotEnterClicked();
  void slotSkipClicked();

private:
  KMyMoneyBriefSchedulePrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyBriefSchedule)
};

#endif
