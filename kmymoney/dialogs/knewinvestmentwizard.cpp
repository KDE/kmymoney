/***************************************************************************
                         knewinvestmentwizard  -  description
                            -------------------
   begin                : Sat Dec 4 2004
   copyright            : (C) 2004 by Thomas Baumgart
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

#include "knewinvestmentwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QLabel>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneylineedit.h>
#include <kmymoneyedit.h>
#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include <ktoolinvocation.h>
#include "kmymoneycurrencyselector.h"
#include "webpricequote.h"
#include "kmymoneyutils.h"

KNewInvestmentWizard::KNewInvestmentWizard( QWidget *parent ) :
  KNewInvestmentWizardDecl( parent )
{
  init1();
  slotCheckPage(QString());

  m_investmentSymbol->setFocus();
  connect(m_investmentSymbol, SIGNAL(lineChanged(const QString&)), this, SLOT(slotCheckForExistingSymbol(const QString&)));
}

KNewInvestmentWizard::KNewInvestmentWizard( const MyMoneyAccount& acc, QWidget *parent ) :
  KNewInvestmentWizardDecl( parent ),
  m_account(acc)
{
  setWindowTitle(i18n("Investment detail wizard"));
  init1();

  // load the widgets with the data
  setName(m_account.name());
  m_security = MyMoneyFile::instance()->security(m_account.currencyId());

  init2();

  int priceMode = 0;
  if(!m_account.value("priceMode").isEmpty())
    priceMode = m_account.value("priceMode").toInt();
  m_priceMode->setCurrentItem(priceMode);

}

KNewInvestmentWizard::KNewInvestmentWizard( const MyMoneySecurity& security, QWidget *parent) :
  KNewInvestmentWizardDecl( parent ),
  m_security(security)
{
  setWindowTitle(i18n("Security detail wizard"));
  init1();
  m_createAccount = false;

  // load the widgets with the data
  setName(security.name());

  init2();

  // no chance to change the price mode here
  m_priceMode->setCurrentItem(0);
  m_priceMode->setEnabled(false);
}

void KNewInvestmentWizard::init1(void)
{
  m_onlineSourceCombo->addItems( WebPriceQuote::quoteSources() );

  m_onlineFactor->setValue(MyMoneyMoney(1,1));
  m_onlineFactor->setPrecision(4);

  m_fraction->setPrecision(0);
  m_fraction->setValue(MyMoneyMoney(100, 1));
  kMyMoneyMoneyValidator* fractionValidator = new kMyMoneyMoneyValidator(1, 100000, 0, this);
  m_fraction->setValidator(fractionValidator);

  // load the price mode combo
  m_priceMode->insertItem(i18nc("default price mode", "<default>"), 0);
  m_priceMode->insertItem(i18n("Price per share"), 1);
  m_priceMode->insertItem(i18n("Total for all shares"), 2);

  // load the widget with the available currencies
  m_tradingCurrencyEdit->update(QString());

  connect(helpButton(),SIGNAL(clicked()), this, SLOT(slotHelp(void)));
  connect(m_investmentName, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_investmentSymbol, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_fraction, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_investmentIdentification, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_onlineFactor, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_onlineSourceCombo, SIGNAL(activated(const QString&)), this, SLOT(slotCheckPage(const QString&)));
  connect(m_useFinanceQuote, SIGNAL(toggled(bool)), this, SLOT(slotSourceChanged(bool)));

  m_createAccount = true;

  // Update label in case of edit
  if(!m_account.id().isEmpty()) {
    m_introLabel->setText(i18n("This wizard allows you to modify the selected investment."));
  }
  if(!m_security.id().isEmpty()) {
    m_introLabel->setText(i18n("This wizard allows you to modify the selected security."));
  }


}

void KNewInvestmentWizard::init2(void)
{
  MyMoneySecurity tradingCurrency = MyMoneyFile::instance()->currency(m_security.tradingCurrency());
  m_investmentSymbol->setText(m_security.tradingSymbol());
  m_tradingMarket->setItemText(m_tradingMarket->currentIndex(), m_security.tradingMarket());
  m_fraction->setValue(MyMoneyMoney(m_security.smallestAccountFraction(), 1));
  m_tradingCurrencyEdit->setSecurity(tradingCurrency);
  if (m_security.value("kmm-online-quote-system") == "Finance::Quote") {
    FinanceQuoteProcess p;
    m_useFinanceQuote->setChecked(true);
    m_onlineSourceCombo->setItemText(m_onlineSourceCombo->currentIndex(), p.niceName(m_security.value("kmm-online-source")));
  } else {
    m_onlineSourceCombo->setItemText(m_onlineSourceCombo->currentIndex(), m_security.value("kmm-online-source"));
  }
  if(!m_security.value("kmm-online-factor").isEmpty())
    m_onlineFactor->setValue(MyMoneyMoney(m_security.value("kmm-online-factor")));
  m_investmentIdentification->setText(m_security.value("kmm-security-id"));
  m_securityType->setItemText(m_securityType->currentIndex(), KMyMoneyUtils::securityTypeToString(m_security.securityType()));

  slotCheckPage(m_security.value("kmm-online-source"));
}

KNewInvestmentWizard::~KNewInvestmentWizard()
{
}

void KNewInvestmentWizard::setName(const QString& name)
{
  m_investmentName->setText(name);
}

void KNewInvestmentWizard::next(void)
{
  KNewInvestmentWizardDecl::next();
  slotCheckPage();
}

void KNewInvestmentWizard::slotCheckForExistingSymbol(const QString& symbol)
{
  Q_UNUSED(symbol);
  if(m_investmentName->text().isEmpty()) {
    QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
    QList<MyMoneySecurity>::const_iterator it_s;
    MyMoneySecurity::eSECURITYTYPE type = KMyMoneyUtils::stringToSecurity(m_securityType->currentText());

    for(it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
      if((*it_s).securityType() == type
      && (*it_s).tradingSymbol() == m_investmentSymbol->text()) {
        m_security = MyMoneySecurity();
        if(KMessageBox::questionYesNo(this, i18n("The selected symbol is already on file. Do you want to reuse the existing security?"), i18n("Security found")) == KMessageBox::Yes) {
          m_security = *it_s;
          init2();
          m_investmentName->loadText(m_security.name());
        }
        break;
      }
    }
  }
}

void KNewInvestmentWizard::slotSourceChanged(bool useFQ)
{
  m_onlineSourceCombo->clear();
  m_onlineSourceCombo->insertItem(0, QString());
  if (useFQ) {
    m_onlineSourceCombo->addItems( WebPriceQuote::quoteSources( WebPriceQuote::FinanceQuote ) );
  } else {
    m_onlineSourceCombo->addItems( WebPriceQuote::quoteSources() );
  }
}

void KNewInvestmentWizard::slotCheckPage(const QString& txt)
{
  m_onlineFactor->setEnabled(!txt.isEmpty());
}

void KNewInvestmentWizard::slotCheckPage(void)
{
  if(currentPage() == m_investmentDetailsPage) {
    setNextEnabled(m_investmentDetailsPage, false);
    if(m_investmentName->text().length() > 0
    && m_investmentSymbol->text().length() > 0
    && !m_fraction->value().isZero()
    ) {
      setNextEnabled(m_investmentDetailsPage, true);
    }
  } else if(currentPage() == m_onlineUpdatePage) {
    setFinishEnabled(m_onlineUpdatePage, true);
    if(m_onlineFactor->isEnabled() && m_onlineFactor->value().isZero())
      setFinishEnabled(m_onlineUpdatePage, false);
  }
}

void KNewInvestmentWizard::slotHelp(void)
{
  KToolInvocation::invokeHelp("details.investments.newinvestmentwizard");
}

void KNewInvestmentWizard::createObjects(const QString& parentId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
  QList<MyMoneySecurity>::ConstIterator it;

  MyMoneySecurity::eSECURITYTYPE type = KMyMoneyUtils::stringToSecurity(m_securityType->currentText());
  MyMoneyFileTransaction ft;
  try {
    // update all relevant attributes only, if we create a stock
    // account and the security is unknown or we modifiy the security
    MyMoneySecurity newSecurity(m_security);
    newSecurity.setName(m_investmentName->text());
    newSecurity.setTradingSymbol(m_investmentSymbol->text());
    newSecurity.setTradingMarket(m_tradingMarket->currentText());
    newSecurity.setSmallestAccountFraction(m_fraction->value());
    newSecurity.setTradingCurrency(m_tradingCurrencyEdit->security().id());
    newSecurity.setSecurityType(type);
    newSecurity.deletePair("kmm-online-source");
    newSecurity.deletePair("kmm-online-quote-system");
    newSecurity.deletePair("kmm-online-factor");
    newSecurity.deletePair("kmm-security-id");

    if(!m_onlineSourceCombo->currentText().isEmpty()) {
      if (m_useFinanceQuote->isChecked()) {
        FinanceQuoteProcess p;
        newSecurity.setValue("kmm-online-quote-system", "Finance::Quote");
        newSecurity.setValue("kmm-online-source", p.crypticName(m_onlineSourceCombo->currentText()));
      }else{
        newSecurity.setValue("kmm-online-source", m_onlineSourceCombo->currentText());
      }
    }
    if(m_onlineFactor->isEnabled() && (m_onlineFactor->value() != MyMoneyMoney(1,1)))
      newSecurity.setValue("kmm-online-factor", m_onlineFactor->value().toString());
    if(!m_investmentIdentification->text().isEmpty())
      newSecurity.setValue("kmm-security-id", m_investmentIdentification->text());

    if(m_security.id().isEmpty() || newSecurity != m_security) {
      m_security = newSecurity;

      // add or update it
      if(m_security.id().isEmpty()) {
        file->addSecurity(m_security);
      } else {
        file->modifySecurity(m_security);
      }
    }

    if(m_createAccount) {
      // now that the security exists, we can add the account to store it
      m_account.setName(m_investmentName->text());
      if(m_account.accountType() == MyMoneyAccount::UnknownAccountType)
        m_account.setAccountType(MyMoneyAccount::Stock);

      m_account.setCurrencyId(m_security.id());
      switch(m_priceMode->currentItem()) {
        case 0:
          m_account.deletePair("priceMode");
          break;
        case 1:
        case 2:
          m_account.setValue("priceMode", QString("%1").arg(m_priceMode->currentItem()));
          break;
      }

      if(m_account.id().isEmpty()) {
        MyMoneyAccount parent = file->account(parentId);
        file->addAccount(m_account, parent);
      } else
        file->modifyAccount(m_account);
    }
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::detailedSorry(0, i18n("Unable to create all objects for the investment"), QString("%1 caugt in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
}

#include "knewinvestmentwizard.moc"
