/***************************************************************************
                          kforecastview.cpp
                             -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kforecastview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

using namespace reports;
using namespace Icons;

KForecastView::KForecastView(QWidget *parent) :
    KMyMoneyViewBase(*new KForecastViewPrivate(this), parent)
{
}

KForecastView::~KForecastView()
{
}

void KForecastView::slotTabChanged(int index)
{
  Q_D(KForecastView);
  ForecastViewTab tab = static_cast<ForecastViewTab>(index);

  // remember this setting for startup
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KForecastView_LastType", QVariant(tab).toString());

  d->loadForecast(tab);
}

void KForecastView::slotManualForecast()
{
  Q_D(KForecastView);
  d->m_needReload[SummaryView] = true;
  d->m_needReload[ListView] = true;
  d->m_needReload[AdvancedView] = true;
  d->m_needReload[BudgetView] = true;
  d->m_needReload[ChartView] = true;

  if (isVisible())
    slotTabChanged(d->ui->m_tab->currentIndex());
}

void KForecastView::showEvent(QShowEvent* event)
{
  Q_D(KForecastView);
  if (d->m_needLoad) {
    d->init();
    d->loadForecastSettings();
  }
  emit customActionRequested(View::Forecast, eView::Action::AboutToShow);

  slotTabChanged(d->ui->m_tab->currentIndex());

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KForecastView::executeCustomAction(eView::Action action)
{
  Q_D(KForecastView);
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      QTimer::singleShot(0, d->ui->m_forecastButton, SLOT(setFocus()));
      break;

    default:
      break;
  }
}

void KForecastView::refresh()
{
  Q_D(KForecastView);
  d->m_needReload[SummaryView] = true;
  d->m_needReload[ListView] = true;
  d->m_needReload[AdvancedView] = true;
  d->m_needReload[BudgetView] = true;
  d->m_needReload[ChartView] = true;

  if (isVisible()) {
    //refresh settings
    d->loadForecastSettings();
    slotTabChanged(d->ui->m_tab->currentIndex());
  }
}

void KForecastView::itemExpanded(QTreeWidgetItem *item)
{
  Q_D(KForecastView);
  if (!item->parent() || !item->parent()->parent())
    return;
  for (int i = 1; i < item->columnCount(); ++i) {
    d->showAmount(item, i, item->data(i, AmountRole).value<MyMoneyMoney>(), MyMoneyFile::instance()->security(item->data(0, AccountRole).value<MyMoneyAccount>().currencyId()));
  }
}

void KForecastView::itemCollapsed(QTreeWidgetItem *item)
{
  Q_D(KForecastView);
  for (int i = 1; i < item->columnCount(); ++i) {
    d->showAmount(item, i, item->data(i, ValueRole).value<MyMoneyMoney>(), MyMoneyFile::instance()->baseCurrency());
  }
}
