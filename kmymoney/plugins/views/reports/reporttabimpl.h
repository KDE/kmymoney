/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
        2018 by Michael Kiefer <Michael-Kiefer@web.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef REPORTTABIMPL_H
#define REPORTTABIMPL_H

#include <QWidget>
#include <QDoubleValidator>

class DateRangeDlg;

namespace Ui
{
class ReportTabGeneral;
class ReportTabRowColPivot;
class ReportTabRowColQuery;
class ReportTabChart;
class ReportTabRange;
class ReportTabCapitalGain;
class ReportTabPerformance;
}

class ReportTabGeneral : public QWidget
{
  Q_DISABLE_COPY(ReportTabGeneral)

public:
  explicit ReportTabGeneral(QWidget *parent);
  ~ReportTabGeneral();

  Ui::ReportTabGeneral* ui;
};

class ReportTabRowColPivot : public QWidget
{
  Q_DISABLE_COPY(ReportTabRowColPivot)

public:
  explicit ReportTabRowColPivot(QWidget *parent);
  ~ReportTabRowColPivot();

  Ui::ReportTabRowColPivot* ui;
};

class ReportTabRowColQuery : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(ReportTabRowColQuery)

public:
  explicit ReportTabRowColQuery(QWidget *parent);
  ~ReportTabRowColQuery();

  Ui::ReportTabRowColQuery* ui;
private Q_SLOTS:
  void slotHideTransactionsChanged(bool checked);
};

class ReportTabChart : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(ReportTabChart)

public:
  explicit ReportTabChart(QWidget *parent);
  ~ReportTabChart();

  Ui::ReportTabChart* ui;
  void setNegExpenses(bool set);

private Q_SLOTS:
  void slotChartTypeChanged(int index);
};

class ReportTabRange : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(ReportTabRange)

public:
  explicit ReportTabRange(QWidget *parent);
  ~ReportTabRange();

  Ui::ReportTabRange* ui;
  DateRangeDlg *m_dateRange;
  void setRangeLogarythmic(bool set);
private:
  enum EDimension { eRangeStart = 0, eRangeEnd, eMajorTick, eMinorTick};
private Q_SLOTS:
  void slotEditingFinished(EDimension dim);
  void slotEditingFinishedStart();
  void slotEditingFinishedEnd();
  void slotEditingFinishedMajor();
  void slotEditingFinishedMinor();
  void slotYLabelsPrecisionChanged(const int &value);
  void slotDataLockChanged(int index);
};

class ReportTabCapitalGain : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(ReportTabCapitalGain)

public:
  explicit ReportTabCapitalGain(QWidget *parent);
  ~ReportTabCapitalGain();

  Ui::ReportTabCapitalGain* ui;

private Q_SLOTS:
  void slotInvestmentSumChanged(int index);
};

class ReportTabPerformance : public QWidget
{
public:
  explicit ReportTabPerformance(QWidget *parent);
  ~ReportTabPerformance();

  Ui::ReportTabPerformance* ui;
};

class MyDoubleValidator : public QDoubleValidator
{
public:
  explicit MyDoubleValidator(int decimals, QObject * parent = 0);

  QValidator::State validate(QString &s, int &i) const final override;
};
#endif /* REPORTTABIMPL_H */

