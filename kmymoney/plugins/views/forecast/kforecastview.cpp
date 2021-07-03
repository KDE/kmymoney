/*
    SPDX-FileCopyrightText: 2007 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

void KForecastView::createActions(KXMLGUIClient* guiClient)
{
    Q_UNUSED(guiClient)
}

void KForecastView::removeActions()
{
}
