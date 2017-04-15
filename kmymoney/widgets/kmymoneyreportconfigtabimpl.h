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

#ifndef KMYMONEYREPORTCONFIGTABIMPL_H
#define KMYMONEYREPORTCONFIGTABIMPL_H

#include <QWidget>
#include <QDoubleValidator>

class DateRangeDlg;

namespace Ui
{
class kMyMoneyReportConfigTab1Decl;
class kMyMoneyReportConfigTab2Decl;
class kMyMoneyReportConfigTab3Decl;
class kMyMoneyReportConfigTabChartDecl;
class kMyMoneyReportConfigTabRangeDecl;
}

class kMyMoneyReportConfigTab1Decl : public QWidget
{
public:
  kMyMoneyReportConfigTab1Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab1Decl();
private:
  Ui::kMyMoneyReportConfigTab1Decl* ui;
};

class kMyMoneyReportConfigTab2Decl : public QWidget
{
public:
  kMyMoneyReportConfigTab2Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab2Decl();
private:
  Ui::kMyMoneyReportConfigTab2Decl* ui;
};

class kMyMoneyReportConfigTab3Decl : public QWidget
{
  Q_OBJECT
public:
  kMyMoneyReportConfigTab3Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab3Decl();
private:
  Ui::kMyMoneyReportConfigTab3Decl* ui;
private slots:
  void slotHideTransactionsChanged(bool checked);
};

class kMyMoneyReportConfigTabChartDecl : public QWidget
{
  Q_OBJECT
public:
  kMyMoneyReportConfigTabChartDecl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTabChartDecl();
  Ui::kMyMoneyReportConfigTabChartDecl* ui;
private slots:
  void slotChartTypeChanged(int index);
};

class kMyMoneyReportConfigTabRangeDecl : public QWidget
{
  Q_OBJECT
public:
  kMyMoneyReportConfigTabRangeDecl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTabRangeDecl();
  Ui::kMyMoneyReportConfigTabRangeDecl* ui;
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
#endif /* KMYMONEYREPORTCONFIGTABIMPL_H */

