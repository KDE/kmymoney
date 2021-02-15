/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
