/***************************************************************************
                         kmymoneyaccounttreeforecast.cpp
                            -------------------
   begin                : Fri Aug 01 2008
   copyright            : (C) 2008 by Alvaro Soliverez
   email                : asoliverez@gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyaccounttreeforecast.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoney.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyforecast.h"
#include "reportaccount.h"

using namespace reports;

KMyMoneyAccountTreeForecast::KMyMoneyAccountTreeForecast(QWidget* parent) :
  KMyMoneyAccountTreeBase(parent)
{
  setResizeMode(Q3ListView::NoColumn);
}

void KMyMoneyAccountTreeForecast::showAccount( void )
{
  addColumn(i18n("Account"));
}

void KMyMoneyAccountTreeForecast::clearColumns( void )
{
  clear();
  while(columns() > 0) {
    removeColumn(0);
  }
}

void KMyMoneyAccountTreeForecast::showSummary(MyMoneyForecast& forecast)
{
  int daysToBeginDay;

  //add cycle interval columns
  addColumn(i18nc("Today's forecast", "Current"), -1);

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if(QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }
  for(int i = 0; ((i*forecast.accountsCycle())+daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int intervalDays = ((i*forecast.accountsCycle())+daysToBeginDay);
    QString columnName =  i18np("1 day", "%1 days", intervalDays);
    addColumn(columnName, -1);
  }

  //add variation columns
  addColumn(i18n("Total variation"), -1);

  //align columns
  for(int i = 1; i < columns(); ++i) {
    setColumnAlignment(i, Qt::AlignRight);
  }

}

void KMyMoneyAccountTreeForecast::showDetailed(MyMoneyForecast& forecast)
{
  //add cycle interval columns
  addColumn(i18nc("Today's forecast", "Current"), -1);

  for(int i = 1; i <= forecast.forecastDays(); ++i) {
    QDate forecastDate = QDate::currentDate().addDays(i);
    QString columnName =  KGlobal::locale()->formatDate(forecastDate, KLocale::LongDate);
    addColumn(columnName, -1);
  }

  //add variation columns
  addColumn(i18n("Total variation"), -1);

  //align columns
  for(int i = 1; i < columns(); ++i) {
    setColumnAlignment(i, Qt::AlignRight);
  }
}

void KMyMoneyAccountTreeForecast::showAdvanced(MyMoneyForecast& forecast)
{
  int daysToBeginDay;
  
  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if(QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }

  //add columns
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = addColumn(i18n("Min Bal %1",i), -1);
    setColumnAlignment(col, Qt::AlignRight);
    addColumn(i18n("Min Date %1",i), -1);
  }
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = addColumn(i18n("Max Bal %1",i), -1);
    setColumnAlignment(col, Qt::AlignRight);
    addColumn(i18n("Max Date %1",i), -1);
  }
  int col = addColumn(i18nc("Average balance", "Average"), -1);
  setColumnAlignment(col, Qt::AlignRight);
}

void KMyMoneyAccountTreeForecast::showBudget(MyMoneyForecast& forecast)
{
  QDate forecastStartDate = forecast.forecastStartDate();
  QDate forecastEndDate = forecast.forecastEndDate();

  //add cycle interval columns
  QDate f_date = forecastStartDate;
  for(; f_date <= forecastEndDate; f_date = f_date.addMonths(1)) {
    QString columnName =  QDate::longMonthName(f_date.month());
    addColumn(columnName, -1);
  }
  //add total column
  addColumn(i18nc("Total balance", "Total"), -1);


  //align columns
  for(int i = 1; i < columns(); ++i) {
    setColumnAlignment(i, Qt::AlignRight);
  }
}

void KMyMoneyAccountTreeForecast::slotSelectObject(const Q3ListViewItem* i)
{
  emit selectObject(MyMoneyInstitution());
  emit selectObject(MyMoneyAccount());

  const KMyMoneyAccountTreeBaseItem* item = dynamic_cast<const KMyMoneyAccountTreeBaseItem*>(i);
  if(item) {
    emit openObject(item->itemObject());
  }
}

KMyMoneyAccountTreeForecastItem::KMyMoneyAccountTreeForecastItem(K3ListView *parent, const MyMoneyAccount& account, const MyMoneyForecast  &forecast, const MyMoneySecurity& security, const QString& name) :
  KMyMoneyAccountTreeBaseItem(parent, account, security, name),
  m_forecast(forecast)
{
  updateAccount(true);
  
}

KMyMoneyAccountTreeForecastItem::KMyMoneyAccountTreeForecastItem(KMyMoneyAccountTreeForecastItem *parent, const MyMoneyAccount& account, const MyMoneyForecast& forecast, const QList<MyMoneyPrice>& price, const MyMoneySecurity& security, const EForecastViewType forecastType) :
    KMyMoneyAccountTreeBaseItem(parent, account, price, security),
    m_forecast(forecast),
    m_forecastType(forecastType)
{
  //setForecastViewType(forecastViewType);
  updateAccount(true);
  switch(forecastViewType())
  {
    case eSummary:
      updateSummary();
      break;
    case eDetailed:
      updateDetailed();
      break;
    case eBudget:
      updateBudget();
      break;
    default:
      break;
  }
}


KMyMoneyAccountTreeForecastItem::~KMyMoneyAccountTreeForecastItem()
{
}

void KMyMoneyAccountTreeForecastItem::setForecast(const MyMoneyForecast& forecast)
{
  m_forecast = forecast;
  updateAccount();
}

void KMyMoneyAccountTreeForecastItem::updateSummary()
{
  MyMoneyMoney amountMM;
  int it_c = 1; // iterator for the columns of the listview
  MyMoneyFile* file = MyMoneyFile::instance();
  int daysToBeginDay;

  if(QDate::currentDate() < m_forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(m_forecast.beginForecastDate());
  } else {
    daysToBeginDay = m_forecast.accountsCycle();
  }
  
  MyMoneySecurity currency;
  if(m_account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(m_account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(m_account.currencyId());
  }


    //add current balance column
  QDate summaryDate = QDate::currentDate();
  amountMM = m_forecast.forecastBalance(m_account, summaryDate);

  //calculate the balance in base currency for the total row
  setAmount(it_c, amountMM);
  setValue(it_c, amountMM, summaryDate);
  showAmount(it_c, amountMM, currency);
  it_c++;

    //iterate through all other columns
  for(QDate summaryDate = QDate::currentDate().addDays(daysToBeginDay); summaryDate <= m_forecast.forecastEndDate();summaryDate = summaryDate.addDays(m_forecast.accountsCycle()), ++it_c) {
    amountMM = m_forecast.forecastBalance(m_account, summaryDate);

    //calculate the balance in base currency for the total row
    setAmount(it_c, amountMM);
    setValue(it_c, amountMM, summaryDate);
    showAmount(it_c, amountMM, currency);
  }
  //calculate and add variation per cycle
  setNegative(m_forecast.accountTotalVariation(m_account).isNegative());
  setAmount(it_c, m_forecast.accountTotalVariation(m_account));
  setValue(it_c, m_forecast.accountTotalVariation(m_account), m_forecast.forecastEndDate());
  showAmount(it_c, m_forecast.accountTotalVariation(m_account), currency);
}

void KMyMoneyAccountTreeForecastItem::updateDetailed()
{
  QString amount;
  QString vAmount;
  MyMoneyMoney vAmountMM;
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneySecurity currency;
  if(m_account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(m_account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(m_account.currencyId());
  }

  int it_c = 1; // iterator for the columns of the listview

  for(QDate forecastDate = QDate::currentDate(); forecastDate <= m_forecast.forecastEndDate(); ++it_c, forecastDate = forecastDate.addDays(1)) {
    MyMoneyMoney amountMM = m_forecast.forecastBalance(m_account, forecastDate);

    //calculate the balance in base currency for the total row
    setAmount(it_c, amountMM);
    setValue(it_c, amountMM, forecastDate);
    showAmount(it_c, amountMM, currency);
  }

    //calculate and add variation per cycle
  vAmountMM = m_forecast.accountTotalVariation(m_account);
  setAmount(it_c, vAmountMM);
  setValue(it_c, vAmountMM, m_forecast.forecastEndDate());
  showAmount(it_c, vAmountMM, currency);
}

void KMyMoneyAccountTreeForecastItem::updateBudget()
{
  MyMoneySecurity currency;
  MyMoneyMoney tAmountMM;

  MyMoneyFile* file = MyMoneyFile::instance();
  int it_c = 1; // iterator for the columns of the listview
  QDate forecastDate = m_forecast.forecastStartDate();

  if(m_account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(m_account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(m_account.currencyId());
  }

    //iterate columns
  for(; forecastDate <= m_forecast.forecastEndDate(); forecastDate = forecastDate.addMonths(1), ++it_c) {
    MyMoneyMoney amountMM;
    amountMM = m_forecast.forecastBalance(m_account,forecastDate);
    if(m_account.accountType() == MyMoneyAccount::Expense)
      amountMM = -amountMM;

    tAmountMM += amountMM;
    setAmount(it_c, amountMM);
    setValue(it_c, amountMM, forecastDate);
    showAmount(it_c, amountMM, currency);
  }

  //set total column
  setAmount(it_c, tAmountMM);
  setValue(it_c, tAmountMM, m_forecast.forecastEndDate());
  showAmount(it_c, tAmountMM, currency);
}

MyMoneyMoney KMyMoneyAccountTreeForecastItem::balance() const
{
  return MyMoneyMoney();
}

void KMyMoneyAccountTreeForecastItem::showAmount(int column, const MyMoneyMoney amount, const MyMoneySecurity security)
{
  setText(column, amount.formatMoney(m_account, security), amount.isNegative() );
}

void KMyMoneyAccountTreeForecastItem::adjustParentValue(int column, const MyMoneyMoney& value)
{
  m_values[column] += value;
  m_values[column] = m_values[column].convert(listView()->baseCurrency().smallestAccountFraction());

  // if the entry has no children,
  // or it is the top entry
  // or it is currently not open
  // we need to display the value of it
  KMyMoneyAccountTreeForecast* lv = dynamic_cast<KMyMoneyAccountTreeForecast*>(listView());
  if(!lv)
    return;
  if(!firstChild() || !parent() || (!isOpen() && firstChild())
    || depth() == 1 ) {
    if(firstChild())
      setText(column, " ");

    showAmount(column, m_values[column], listView()->baseCurrency());
  }

  // now make sure, the upstream accounts also get notified about the value change
  KMyMoneyAccountTreeForecastItem* p = dynamic_cast<KMyMoneyAccountTreeForecastItem*>(parent());
  if(p != 0) {
    p->adjustParentValue(column, value);
  }
}

void KMyMoneyAccountTreeForecastItem::setValue(int column, MyMoneyMoney amount, QDate forecastDate)
{
  KMyMoneyAccountTreeForecastItem* p = dynamic_cast<KMyMoneyAccountTreeForecastItem*>(parent());

  //calculate the balance in base currency for the total row
  if(m_account.currencyId() != listView()->baseCurrency().id()) {
#ifndef KMM_DESIGNER
    ReportAccount repAcc = ReportAccount(m_account.id());
    MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(forecastDate);
    MyMoneyMoney baseAmountMM = amount * curPrice;
    m_values[column] = baseAmountMM.convert(listView()->baseCurrency().smallestAccountFraction());
#endif
    if(p != 0) {
      p->adjustParentValue(column, m_values[column]);
    }
  } else {
    m_values[column] += amount;
    if(p != 0) {
      p->adjustParentValue(column, amount);
    }
  }
}

void KMyMoneyAccountTreeForecastItem::setAmount(int column, MyMoneyMoney amount)
{
  m_amounts[column] = amount;
}

void KMyMoneyAccountTreeForecastItem::setOpen(bool open)
{
  if (open == isOpen())
    return;
  KMyMoneyAccountTreeBaseItem::setOpen(open);

  if(!open)
  {
    for(int i = 1; i < listView()->columns(); ++i)
    {
      showAmount(i, m_values[i], listView()->baseCurrency());
    }
  } else if (depth() > 1) {
    for(int i = 1; i < listView()->columns(); ++i)
    {
      showAmount(i, m_amounts[i], m_security);
    }
  }
}

#include "kmymoneyaccounttreeforecast.moc"
