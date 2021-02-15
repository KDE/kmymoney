/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
        2018 by Michael Kiefer <Michael-Kiefer@web.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
  bool m_logYaxis;
  /**
   * Update data range start and data range end text validators
   * and re-validate the contents of those text fields against the updated validator.
   * If re-validation fails, arbitrary default values will be set depending on vertical axis type.
   * This fucntion should be called when vertical axis type or labels precision changed.
   */
  void updateDataRangeValidators(const int& precision);
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

class MyLogarithmicDoubleValidator : public QDoubleValidator
{
public:
  explicit MyLogarithmicDoubleValidator(const int decimals, const qreal defaultValue, QObject *parent = nullptr);

  QValidator::State validate(QString &s, int &i) const final Q_DECL_OVERRIDE;
  void fixup(QString &input) const final Q_DECL_OVERRIDE;
private:
  QString m_defaultText;
};
#endif /* REPORTTABIMPL_H */

