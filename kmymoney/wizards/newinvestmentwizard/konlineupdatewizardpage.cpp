/***************************************************************************
                         konlineupdatewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konlineupdatewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "webpricequote.h"

KOnlineUpdateWizardPage::KOnlineUpdateWizardPage(QWidget *parent)
    : KOnlineUpdateWizardPageDecl(parent)
{
  m_onlineFactor->setValue(MyMoneyMoney::ONE);
  m_onlineFactor->setPrecision(4);

  // make m_onlineSourceCombo sortable
  QSortFilterProxyModel* proxy = new QSortFilterProxyModel(m_onlineSourceCombo);
  proxy->setSourceModel(m_onlineSourceCombo->model());
  proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_onlineSourceCombo->model()->setParent(proxy);
  m_onlineSourceCombo->setModel(proxy);

  // Connect signals-slots
  connect(m_useFinanceQuote, SIGNAL(toggled(bool)), this, SLOT(slotSourceChanged(bool)));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("onlineFactor", m_onlineFactor, "value");
  registerField("onlineSourceCombo", m_onlineSourceCombo, "currentText", SIGNAL(currentIndexChanged(QString)));
  registerField("useFinanceQuote", m_useFinanceQuote);
  connect(m_onlineSourceCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotCheckPage(QString)));
  connect(m_onlineFactor, SIGNAL(textChanged(QString)),
          this, SIGNAL(completeChanged()));

  connect(m_onlineSourceCombo, SIGNAL(activated(QString)),
          this, SIGNAL(completeChanged()));

  connect(m_useFinanceQuote, SIGNAL(toggled(bool)),
          this, SIGNAL(completeChanged()));
}

/**
 * Set the values based on the @param security
 */
void KOnlineUpdateWizardPage::init2(const MyMoneySecurity& security)
{
  int idx = -1;
  if (security.value("kmm-online-quote-system") == "Finance::Quote") {
    FinanceQuoteProcess p;
    m_useFinanceQuote->setChecked(true);
    idx = m_onlineSourceCombo->findText(p.niceName(security.value("kmm-online-source")));
  } else {
    idx = m_onlineSourceCombo->findText(security.value("kmm-online-source"));
  }

  // in case we did not find the entry, we use the empty one
  if (idx == -1)
    idx = m_onlineSourceCombo->findText(QString());
  m_onlineSourceCombo->setCurrentIndex(idx);

  if (!security.value("kmm-online-factor").isEmpty())
    m_onlineFactor->setValue(MyMoneyMoney(security.value("kmm-online-factor")));
}

/**
 * Update the "Next" button
 */
bool KOnlineUpdateWizardPage::isComplete() const
{
  return !(m_onlineFactor->isEnabled()
           && m_onlineFactor->value().isZero());
}

bool KOnlineUpdateWizardPage::isOnlineFactorEnabled() const
{
  return m_onlineFactor->isEnabled();
}

void KOnlineUpdateWizardPage::slotCheckPage(const QString& txt)
{
  m_onlineFactor->setEnabled(!txt.isEmpty());
}

void KOnlineUpdateWizardPage::slotSourceChanged(bool useFQ)
{
  Q_UNUSED(useFQ);
  m_onlineSourceCombo->clear();
  m_onlineSourceCombo->insertItem(0, QString());
  if (useFQ) {
    m_onlineSourceCombo->addItems(WebPriceQuote::quoteSources(WebPriceQuote::FinanceQuote));
  } else {
    m_onlineSourceCombo->addItems(WebPriceQuote::quoteSources());
  }
  m_onlineSourceCombo->model()->sort(0);
}
