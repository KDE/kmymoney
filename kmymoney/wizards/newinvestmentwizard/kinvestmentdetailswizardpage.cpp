/***************************************************************************
                         kinvestmentdetailswizardpage  -  description
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

#include "kinvestmentdetailswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

KInvestmentDetailsWizardPage::KInvestmentDetailsWizardPage(QWidget *parent)
    : KInvestmentDetailsWizardPageDecl(parent)
{
  m_fraction->setPrecision(0);
  m_fraction->setValue(MyMoneyMoney(100, 1));
  kMyMoneyMoneyValidator* fractionValidator = new kMyMoneyMoneyValidator(1, 100000, 0, this);
  m_fraction->setValidator(fractionValidator);

  // load the price mode combo
  m_priceMode->insertItem(i18nc("default price mode", "(default)"), 0);
  m_priceMode->insertItem(i18n("Finance", "Price per share"), 1);
  m_priceMode->insertItem(i18n("Finance", "Total for all shares"), 2);

  // load the widget with the available currencies
  m_tradingCurrencyEdit->update(QString());

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("investmentName", m_investmentName);
  connect(m_investmentName, SIGNAL(textChanged(QString)),
          this, SIGNAL(completeChanged()));

  registerField("investmentIdentification", m_investmentIdentification);
  connect(m_investmentIdentification, SIGNAL(textChanged(QString)),
          this, SIGNAL(completeChanged()));

  registerField("investmentSymbol", m_investmentSymbol);
  connect(m_investmentSymbol, SIGNAL(textChanged(QString)),
          this, SIGNAL(completeChanged()));

  registerField("tradingCurrencyEdit", m_tradingCurrencyEdit, "security");

  registerField("tradingMarket", m_tradingMarket, "currentText", SIGNAL(currentIndexChanged(QString)));

  registerField("fraction", m_fraction, "value", SIGNAL(textChanged()));
  connect(m_fraction, SIGNAL(textChanged(QString)),
          this, SIGNAL(completeChanged()));
}

/**
 * Set the values based on the @param security
 */
void KInvestmentDetailsWizardPage::init2(const MyMoneySecurity& security)
{
  MyMoneySecurity tradingCurrency = MyMoneyFile::instance()->currency(security.tradingCurrency());
  m_investmentSymbol->setText(security.tradingSymbol());
  m_tradingMarket->setCurrentIndex(m_tradingMarket->findText(security.tradingMarket(), Qt::MatchExactly));
  m_fraction->setValue(MyMoneyMoney(security.smallestAccountFraction(), 1));
  m_tradingCurrencyEdit->setSecurity(tradingCurrency);

  m_investmentIdentification->setText(security.value("kmm-security-id"));
}

/**
 * Update the "Next" button
 */
bool KInvestmentDetailsWizardPage::isComplete() const
{
  return (!m_investmentName->text().isEmpty()
          && !m_investmentSymbol->text().isEmpty()
          && !m_fraction->value().isZero());
}

int KInvestmentDetailsWizardPage::priceMode() const
{
  return m_priceMode->currentItem();
}

void KInvestmentDetailsWizardPage::setCurrentPriceMode(int mode)
{
  m_priceMode->setCurrentItem(mode);
}

void KInvestmentDetailsWizardPage::loadName(const QString& name)
{
  m_investmentName->loadText(name);
}

void KInvestmentDetailsWizardPage::setName(const QString& name)
{
  m_investmentName->setText(name);
}

void KInvestmentDetailsWizardPage::setPriceModeEnabled(bool enabled)
{
  m_priceMode->setEnabled(enabled);
}

void KInvestmentDetailsWizardPage::setupInvestmentSymbol()
{
  m_investmentSymbol->setFocus();
  connect(m_investmentSymbol, SIGNAL(lineChanged(QString)), this, SIGNAL(checkForExistingSymbol(QString)));
}
