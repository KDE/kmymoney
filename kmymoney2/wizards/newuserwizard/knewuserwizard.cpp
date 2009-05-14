/***************************************************************************
                             knewuserwizard.cpp
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <locale.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <k3listview.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kuser.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <kurl.h>
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#include <kmessagebox.h>
#include <k3activelabel.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include <kguiutils.h>
#include <kmymoneyaccounttree.h>
#include <mymoneypayee.h>
#include <mymoneymoney.h>
#include <mymoneyinstitution.h>
#include <mymoneyaccount.h>
#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kaccounttemplateselector.h>

#include "../../kmymoney2.h"
#include "../../kmymoneyglobalsettings.h"

using namespace NewUserWizard;

static int stepCount;

NewUserWizard::Wizard::Wizard(QWidget *parent, const char *name, bool modal, Qt::WFlags flags) :
  KMyMoneyWizard(parent, name, modal, flags),
  m_introPage(0)
{
  bool isFirstTime = KMyMoneyGlobalSettings::firstTimeRun();

  stepCount = 1;

  setTitle(i18n("KMyMoney New File Setup"));
  if(isFirstTime)
    addStep(i18n("Introduction"));
  addStep(i18n("Personal Data"));
  addStep(i18n("Select Currency"));
  addStep(i18n("Select Accounts"));
  addStep(i18n("Set preferences"));
  addStep(i18n("Finish"));

  if(isFirstTime)
    m_introPage = new IntroPage(this);
  m_generalPage = new GeneralPage(this);
  m_currencyPage = new CurrencyPage(this);
  m_accountPage = new AccountPage(this);
  m_categoriesPage = new CategoriesPage(this);
  m_preferencePage = new PreferencePage(this);
  m_filePage = new FilePage(this);

  m_accountPage->m_haveCheckingAccountButton->setChecked(true);
  if(isFirstTime)
    setFirstPage(m_introPage);
  else
    setFirstPage(m_generalPage);

  setHelpContext("firsttime-3");
}

MyMoneyPayee NewUserWizard::Wizard::user(void) const
{
  return m_generalPage->user();
}

QString NewUserWizard::Wizard::url(void) const
{
  return m_filePage->m_dataFileEdit->url();
}

MyMoneyInstitution NewUserWizard::Wizard::institution(void) const
{
  MyMoneyInstitution inst;
  if(m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    if(m_accountPage->m_institutionNameEdit->text().length()) {
      inst.setName(m_accountPage->m_institutionNameEdit->text());
      if(m_accountPage->m_institutionNumberEdit->text().length())
        inst.setSortcode(m_accountPage->m_institutionNumberEdit->text());
    }
  }
  return inst;
}

MyMoneyAccount NewUserWizard::Wizard::account(void) const
{
  MyMoneyAccount acc;
  if(m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    acc.setName(m_accountPage->m_accountNameEdit->text());
    if(m_accountPage->m_accountNumberEdit->text().length())
      acc.setNumber(m_accountPage->m_accountNumberEdit->text());
    acc.setOpeningDate(m_accountPage->m_openingDateEdit->date());
    acc.setCurrencyId(m_baseCurrency.id());
    acc.setAccountType(MyMoneyAccount::Checkings);
  }
  return acc;
}

MyMoneyMoney NewUserWizard::Wizard::openingBalance(void) const
{
  return m_accountPage->m_openingBalanceEdit->value();
}

MyMoneySecurity NewUserWizard::Wizard::baseCurrency(void) const
{
  return m_baseCurrency;
}

Q3ValueList<MyMoneyTemplate> NewUserWizard::Wizard::templates(void) const
{
  return m_categoriesPage->selectedTemplates();
}

IntroPage::IntroPage(Wizard* wizard, const char* name) :
  KIntroPageDecl(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
}

KMyMoneyWizardPage* IntroPage::nextPage(void) const
{
  return m_wizard->m_generalPage;
}

GeneralPage::GeneralPage(Wizard* wizard, const char* name) :
  UserInfo(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
  m_userNameEdit->setFocus();
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self(true);
  connect(ab, SIGNAL(addressBookChanged(AddressBook*)), this, SLOT(slotAddressBookLoaded()));
  connect(m_loadAddressButton, SIGNAL(clicked()), this, SLOT(slotLoadFromKABC()));
  m_loadAddressButton->setEnabled(false);
}

void GeneralPage::slotAddressBookLoaded(void)
{
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  if (!ab)
    return;

  m_loadAddressButton->setEnabled(!ab->whoAmI().isEmpty());
}

void GeneralPage::slotLoadFromKABC(void)
{
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  if (!ab)
    return;

  KABC::Addressee addr = ab->whoAmI();
  if ( addr.isEmpty() ) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard addressbook."), i18n("Addressbook import"));
    return;
  }

  m_userNameEdit->setText( addr.formattedName() );
  m_emailEdit->setText( addr.preferredEmail() );

  KABC::PhoneNumber phone = addr.phoneNumber( KABC::PhoneNumber::Home );
  m_telephoneEdit->setText( phone.number() );

  KABC::Address a = addr.address( KABC::Address::Home );
  QString sep;
  if(!a.country().isEmpty() && !a.region().isEmpty())
    sep = " / ";
  m_countyEdit->setText(QString("%1%2%3").arg(a.country(), sep, a.region()));
  m_postcodeEdit->setText( a.postalCode() );
  m_townEdit->setText( a.locality() );
  m_streetEdit->setText( a.street() );
}

KMyMoneyWizardPage* GeneralPage::nextPage(void) const
{
  return m_wizard->m_currencyPage;
}

CurrencyPage::CurrencyPage(Wizard* wizard, const char* name) :
  Currency(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
  Q3ListViewItem *first = 0;
  Q3ValueList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  Q3ValueList<MyMoneySecurity>::const_iterator it;

  QString localCurrency(localeconv()->int_curr_symbol);
  localCurrency.truncate(3);

  QString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  m_currencyList->clear();
  for(it = list.begin(); it != list.end(); ++it) {
    Q3ListViewItem* p = insertCurrency(*it);
    if((*it).id() == baseCurrency) {
      first = p;
      p->setPixmap(0, QPixmap( locate("icon","hicolor/16x16/apps/kmymoney2.png")));
    } else {
      p->setPixmap(0, empty);
    }
    if(!first && (*it).id() == localCurrency)
      first = p;
  }

  if(first == 0)
    first = m_currencyList->firstChild();
  if(first != 0) {
    m_currencyList->setCurrentItem(first);
    m_currencyList->setSelected(first, true);
    m_currencyList->ensureItemVisible(first);
  }
}

void CurrencyPage::enterPage(void)
{
  m_currencyList->setFocus();
}


KMyMoneyWizardPage* CurrencyPage::nextPage(void) const
{
  m_wizard->m_baseCurrency = MyMoneyFile::instance()->security(selectedCurrency());
  m_wizard->m_accountPage->m_accountCurrencyLabel->setText(m_wizard->m_baseCurrency.tradingSymbol());
  return m_wizard->m_accountPage;
}

AccountPage::AccountPage(Wizard* wizard, const char* name) :
  KAccountPageDecl(wizard, name),
  WizardPage<Wizard>(stepCount, this, wizard, name)       // don't inc. the step count here
{
  m_mandatoryGroup->add(m_accountNameEdit);
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  connect(m_haveCheckingAccountButton, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));
  m_accountNameEdit->setFocus();
  m_openingDateEdit->setDate(QDate(QDate::currentDate().year(),1,1));
}

KMyMoneyWizardPage* AccountPage::nextPage(void) const
{
  return m_wizard->m_categoriesPage;
}

bool AccountPage::isComplete(void) const
{
  return !m_haveCheckingAccountButton->isChecked() || m_mandatoryGroup->isEnabled();
}

CategoriesPage::CategoriesPage(Wizard* wizard, const char* name) :
  Accounts(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
}

KMyMoneyWizardPage* CategoriesPage::nextPage(void) const
{
  return m_wizard->m_preferencePage;
}

Q3ValueList<MyMoneyTemplate> CategoriesPage::selectedTemplates(void) const
{
  return m_templateSelector->selectedTemplates();
}

PreferencePage::PreferencePage(Wizard* wizard, const char* name) :
  KPreferencePageDecl(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
  connect(m_openConfigButton, SIGNAL(clicked()), kmymoney2, SLOT(slotSettings()));
}

KMyMoneyWizardPage* PreferencePage::nextPage(void) const
{
  return m_wizard->m_filePage;
}

FilePage::FilePage(Wizard* wizard, const char* name) :
  KFilePageDecl(wizard),
  WizardPage<Wizard>(stepCount++, this, wizard, name)
{
  m_mandatoryGroup->add(m_dataFileEdit->lineEdit());
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));

  KUser user;
  m_dataFileEdit->setShowLocalProtocol(false);
  m_dataFileEdit->setURL(QString("%1/%2.kmy").arg(QDir::homePath(), user.loginName()));
}

bool FilePage::isComplete(void) const
{
  bool rc = m_mandatoryGroup->isEnabled();
  m_existingFileLabel->hide();
  m_finishLabel->show();
  if(rc) {
    // if a filename is present, check that
    // a) the file does not exist
    // b) the directory does exist
    rc = !KIO::NetAccess::exists(m_dataFileEdit->url(), false, m_wizard);
    if(rc) {
      QRegExp exp("(.*)/(.*)");
      rc = false;
      if(exp.search(m_dataFileEdit->url()) != -1) {
        if(exp.cap(2).length() > 0) {
          rc = KIO::NetAccess::exists(exp.cap(1), true, m_wizard);
        }
      }
    }
    m_existingFileLabel->setHidden(rc);
    m_finishLabel->setShown(rc);
  }
  return rc;
}

#include "knewuserwizard.moc"
