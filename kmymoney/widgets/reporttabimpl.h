/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>

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
}

class ReportTabGeneral : public QWidget
{
public:
  ReportTabGeneral(QWidget *parent);
  virtual ~ReportTabGeneral();
  Ui::ReportTabGeneral* ui;
};

class ReportTabRowColPivot : public QWidget
{
public:
  ReportTabRowColPivot(QWidget *parent);
  virtual ~ReportTabRowColPivot();
  Ui::ReportTabRowColPivot* ui;
};

class ReportTabRowColQuery : public QWidget
{
  Q_OBJECT
public:
  ReportTabRowColQuery(QWidget *parent);
  virtual ~ReportTabRowColQuery();
  Ui::ReportTabRowColQuery* ui;
private slots:
  void slotHideTransactionsChanged(bool checked);
};

class ReportTabChart : public QWidget
{
  Q_OBJECT
public:
  ReportTabChart(QWidget *parent);
  virtual ~ReportTabChart();
  Ui::ReportTabChart* ui;
private slots:
  void slotChartTypeChanged(int index);
};

class ReportTabRange : public QWidget
{
  Q_OBJECT
public:
  ReportTabRange(QWidget *parent);
  virtual ~ReportTabRange();
  Ui::ReportTabRange* ui;
  DateRangeDlg *m_dateRange;
  void setRangeLogarythmic(bool set);
private:
  enum EDimension { eRangeStart = 0, eRangeEnd, eMajorTick, eMinorTick};
private slots:
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
public:
  ReportTabCapitalGain(QWidget *parent);
  virtual ~ReportTabCapitalGain();
  Ui::ReportTabCapitalGain* ui;
};

class MyDoubleValidator : public QDoubleValidator
{
public:
  MyDoubleValidator(int decimals, QObject * parent = 0) :
    QDoubleValidator(0, 0, decimals, parent)
  {
  }

  QValidator::State validate(QString &s, int &i) const
  {
    Q_UNUSED(i);
    if (s.isEmpty() || s == "-") {
      return QValidator::Intermediate;
    }

    QChar decimalPoint = locale().decimalPoint();

    if(s.indexOf(decimalPoint) != -1) {
      int charsAfterPoint = s.length() - s.indexOf(decimalPoint) - 1;

      if (charsAfterPoint > decimals()) {
        return QValidator::Invalid;
      }
    }

    bool ok;
    locale().toDouble(s, &ok);

    if (ok) {
      return QValidator::Acceptable;
    } else {
      return QValidator::Invalid;
    }
  }
};
#endif /* REPORTTABIMPL_H */

