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

#include "kmymoneylineedit.h"
#include "kmymoneyedit.h"
#include "mymoneysecurity.h"
#include "mymoneyfile.h"
#include "ktoolinvocation.h"
#include "kmymoneycurrencyselector.h"
#include <alkimia/alkfinancequoteprocess.h>
#include "kmymoneyutils.h"

KNewInvestmentWizard::KNewInvestmentWizard(QWidget *parent) :
    KNewInvestmentWizardDecl(parent)
{
  init1();
  m_onlineUpdatePage->slotCheckPage(QString());

  m_investmentDetailsPage->setupInvestmentSymbol();

  connect(m_investmentDetailsPage, SIGNAL(checkForExistingSymbol(QString)), this, SLOT(slotCheckForExistingSymbol(QString)));
}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneyAccount& acc, QWidget *parent) :
    KNewInvestmentWizardDecl(parent),
    m_account(acc)
{
  setWindowTitle(i18n("Investment detail wizard"));
  init1();

  // load the widgets with the data
  setName(m_account.name());
  m_security = MyMoneyFile::instance()->security(m_account.currencyId());

  init2();

  int priceMode = 0;
  if (!m_account.value("priceMode").isEmpty())
    priceMode = m_account.value("priceMode").toInt();
  m_investmentDetailsPage->setCurrentPriceMode(priceMode);

}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneySecurity& security, QWidget *parent) :
    KNewInvestmentWizardDecl(parent),
    m_security(security)
{
  setWindowTitle(i18n("Security detail wizard"));
  init1();
  m_createAccount = false;

  // load the widgets with the data
  setName(security.name());

  init2();

  // no chance to change the price mode here
  m_investmentDetailsPage->setCurrentPriceMode(0);
  m_investmentDetailsPage->setPriceModeEnabled(false);
}

void KNewInvestmentWizard::init1()
{
  m_onlineUpdatePage->slotSourceChanged(false);

  // make sure, the back button does not clear fields
  setOption(QWizard::IndependentPages, true);

  // enable the help button
  setOption(HaveHelpButton, true);
  connect(this, SIGNAL(helpRequested()), this, SLOT(slotHelp()));

  m_createAccount = true;

  // Update label in case of edit
  if (!m_account.id().isEmpty()) {
    m_investmentTypePage->setIntroLabelText(i18n("This wizard allows you to modify the selected investment."));
  }
  if (!m_security.id().isEmpty()) {
    m_investmentTypePage->setIntroLabelText(i18n("This wizard allows you to modify the selected security."));
  }

  KMyMoneyUtils::updateWizardButtons(this);
}

void KNewInvestmentWizard::init2()
{
  m_investmentTypePage->init2(m_security);
  m_investmentDetailsPage->init2(m_security);
  m_onlineUpdatePage->init2(m_security);
  m_onlineUpdatePage->slotCheckPage(m_security.value("kmm-online-source"));
}

KNewInvestmentWizard::~KNewInvestmentWizard()
{
}

void KNewInvestmentWizard::setName(const QString& name)
{
  m_investmentDetailsPage->setName(name);
}

void KNewInvestmentWizard::slotCheckForExistingSymbol(const QString& symbol)
{
  Q_UNUSED(symbol);

  if (field("investmentName").toString().isEmpty()) {
    QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
    MyMoneySecurity::eSECURITYTYPE type = KMyMoneyUtils::stringToSecurity(field("securityType").toString());

    foreach (const MyMoneySecurity& it_s, list) {
      if (it_s.securityType() == type
          && it_s.tradingSymbol() == field("investmentSymbol").toString()) {
        m_security = MyMoneySecurity();
        if (KMessageBox::questionYesNo(this, i18n("The selected symbol is already on file. Do you want to reuse the existing security?"), i18n("Security found")) == KMessageBox::Yes) {
          m_security = it_s;
          init2();
          m_investmentDetailsPage->loadName(m_security.name());
        }
        break;
      }
    }
  }
}

void KNewInvestmentWizard::slotHelp()
{
  KToolInvocation::invokeHelp("details.investments.newinvestmentwizard");
}

void KNewInvestmentWizard::createObjects(const QString& parentId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
  QList<MyMoneySecurity>::ConstIterator it;

  MyMoneySecurity::eSECURITYTYPE type = KMyMoneyUtils::stringToSecurity(field("securityType").toString());
  MyMoneyFileTransaction ft;
  try {
    // update all relevant attributes only, if we create a stock
    // account and the security is unknown or we modifiy the security
    MyMoneySecurity newSecurity(m_security);
    newSecurity.setName(field("investmentName").toString());
    newSecurity.setTradingSymbol(field("investmentSymbol").toString());
    newSecurity.setTradingMarket(field("tradingMarket").toString());
    newSecurity.setSmallestAccountFraction(field("fraction").value<MyMoneyMoney>().formatMoney("", 0, false).toUInt());
    newSecurity.setTradingCurrency(field("tradingCurrencyEdit").value<MyMoneySecurity>().id());
    newSecurity.setSecurityType(type);
    newSecurity.deletePair("kmm-online-source");
    newSecurity.deletePair("kmm-online-quote-system");
    newSecurity.deletePair("kmm-online-factor");
    newSecurity.deletePair("kmm-security-id");

    if (!field("onlineSourceCombo").toString().isEmpty()) {
      if (field("useFinanceQuote").toBool()) {
        AlkFinanceQuoteProcess p;
        newSecurity.setValue("kmm-online-quote-system", "Finance::Quote");
        newSecurity.setValue("kmm-online-source", p.crypticName(field("onlineSourceCombo").toString()));
      } else {
        newSecurity.setValue("kmm-online-source", field("onlineSourceCombo").toString());
      }
    }
    if (m_onlineUpdatePage->isOnlineFactorEnabled() && (field("onlineFactor").value<MyMoneyMoney>() != MyMoneyMoney::ONE))
      newSecurity.setValue("kmm-online-factor", field("onlineFactor").value<MyMoneyMoney>().toString());
    if (!field("investmentIdentification").toString().isEmpty())
      newSecurity.setValue("kmm-security-id", field("investmentIdentification").toString());

    if (m_security.id().isEmpty() || newSecurity != m_security) {
      m_security = newSecurity;

      // add or update it
      if (m_security.id().isEmpty()) {
        file->addSecurity(m_security);
      } else {
        file->modifySecurity(m_security);
      }
    }

    if (m_createAccount) {
      // now that the security exists, we can add the account to store it
      m_account.setName(field("investmentName").toString());
      if (m_account.accountType() == MyMoneyAccount::UnknownAccountType)
        m_account.setAccountType(MyMoneyAccount::Stock);

      m_account.setCurrencyId(m_security.id());
      switch (m_investmentDetailsPage->priceMode()) {
        case 0:
          m_account.deletePair("priceMode");
          break;
        case 1:
        case 2:
          m_account.setValue("priceMode", QString("%1").arg(m_investmentDetailsPage->priceMode()));
          break;
      }

      if (m_account.id().isEmpty()) {
        MyMoneyAccount parent = file->account(parentId);
        file->addAccount(m_account, parent);
      } else
        file->modifyAccount(m_account);
    }
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(nullptr, i18n("Unable to create all objects for the investment"), QString("%1 caugt in %2:%3").arg(e.what()).arg(e.file()).arg(e.line()));
  }
}
