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

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QBitmap>
#include <QCheckBox>
#include <QPushButton>
#include <QDir>
#include <QLabel>
#include <QList>
#include <QFileInfo>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
#include <ktextedit.h>
#include <kuser.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include <kguiutils.h>
#include <mymoneypayee.h>
#include <mymoneymoney.h>
#include <mymoneyinstitution.h>
#include <mymoneyaccount.h>
#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kaccounttemplateselector.h>
#include "kmymoneyglobalsettings.h"
#include <icons/icons.h>

using namespace Icons;

namespace NewUserWizard
{

static int stepCount;

Wizard::Wizard(QWidget *parent, bool modal, Qt::WindowFlags flags) :
    KMyMoneyWizard(parent, modal, flags),
    m_introPage(0)
{
  bool isFirstTime = KMyMoneyGlobalSettings::firstTimeRun();

  stepCount = 1;

  setTitle(i18n("KMyMoney New File Setup"));
  if (isFirstTime)
    addStep(i18nc("New file wizard introduction", "Introduction"));
  addStep(i18n("Personal Data"));
  addStep(i18n("Select Currency"));
  addStep(i18n("Select Accounts"));
  addStep(i18n("Set preferences"));
  addStep(i18nc("Finish the wizard", "Finish"));

  if (isFirstTime)
    m_introPage = new IntroPage(this);
  m_generalPage = new GeneralPage(this);
  m_currencyPage = new CurrencyPage(this);
  m_accountPage = new AccountPage(this);
  m_categoriesPage = new CategoriesPage(this);
  m_preferencePage = new PreferencePage(this);
  m_filePage = new FilePage(this);

  m_accountPage->m_haveCheckingAccountButton->setChecked(true);
  if (isFirstTime)
    setFirstPage(m_introPage);
  else
    setFirstPage(m_generalPage);

  setHelpContext("firsttime-3");
}

MyMoneyPayee Wizard::user() const
{
  return m_generalPage->user();
}

QUrl Wizard::url() const
{
  return m_filePage->m_dataFileEdit->url();
}

MyMoneyInstitution Wizard::institution() const
{
  MyMoneyInstitution inst;
  if (m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    if (m_accountPage->m_institutionNameEdit->text().length()) {
      inst.setName(m_accountPage->m_institutionNameEdit->text());
      if (m_accountPage->m_institutionNumberEdit->text().length())
        inst.setSortcode(m_accountPage->m_institutionNumberEdit->text());
    }
  }
  return inst;
}

MyMoneyAccount Wizard::account() const
{
  MyMoneyAccount acc;
  if (m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    acc.setName(m_accountPage->m_accountNameEdit->text());
    if (m_accountPage->m_accountNumberEdit->text().length())
      acc.setNumber(m_accountPage->m_accountNumberEdit->text());
    acc.setOpeningDate(m_accountPage->m_openingDateEdit->date());
    acc.setCurrencyId(m_baseCurrency.id());
    acc.setAccountType(MyMoneyAccount::Checkings);
  }
  return acc;
}

MyMoneyMoney Wizard::openingBalance() const
{
  return m_accountPage->m_openingBalanceEdit->value();
}

MyMoneySecurity Wizard::baseCurrency() const
{
  return m_baseCurrency;
}

QList<MyMoneyTemplate> Wizard::templates() const
{
  return m_categoriesPage->selectedTemplates();
}

bool Wizard::startSettingsAfterFinished() const
{
  return m_preferencePage->m_openConfigAfterFinished->checkState() == Qt::Checked;
}

IntroPage::IntroPage(Wizard* wizard) :
    KIntroPageDecl(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard)
{
}

KMyMoneyWizardPage* IntroPage::nextPage() const
{
  return m_wizard->m_generalPage;
}

GeneralPage::GeneralPage(Wizard* wizard) :
    UserInfo(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard),
    m_contact(new MyMoneyContact(this))
{
  m_userNameEdit->setFocus();

  m_loadAddressButton->setEnabled(m_contact->ownerExists());
  connect(m_loadAddressButton, SIGNAL(clicked()), this, SLOT(slotLoadFromAddressBook()));
}

void GeneralPage::slotLoadFromAddressBook()
{
  m_userNameEdit->setText(m_contact->ownerFullName());
  m_emailEdit->setText(m_contact->ownerEmail());
  if (m_emailEdit->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
    return;
  }
  m_loadAddressButton->setEnabled(false);
  connect(m_contact, SIGNAL(contactFetched(ContactData)), this, SLOT(slotContactFetched(ContactData)));
  m_contact->fetchContact(m_emailEdit->text());
}

void GeneralPage::slotContactFetched(const ContactData &identity)
{
  m_loadAddressButton->setEnabled(true);
  if (identity.email.isEmpty())
    return;
  m_telephoneEdit->setText(identity.phoneNumber);
  QString sep;
  if (!identity.country.isEmpty() && !identity.region.isEmpty())
    sep = " / ";
  m_countyEdit->setText(QString("%1%2%3").arg(identity.country, sep, identity.region));
  m_postcodeEdit->setText(identity.postalCode);
  m_townEdit->setText(identity.locality);
  m_streetEdit->setText(identity.street);
}

KMyMoneyWizardPage* GeneralPage::nextPage() const
{
  return m_wizard->m_currencyPage;
}

CurrencyPage::CurrencyPage(Wizard* wizard) :
    Currency(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard)
{
  QTreeWidgetItem *first = 0;

  QList<MyMoneySecurity> list = MyMoneyFile::instance()->availableCurrencyList();
  QList<MyMoneySecurity>::const_iterator it;

  QString localCurrency(QLocale().currencySymbol(QLocale::CurrencyIsoCode));
  QString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();


  m_currencyList->clear();
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem* p = insertCurrency(*it);
    if ((*it).id() == baseCurrency) {
      first = p;
      QIcon icon = QIcon::fromTheme(g_Icons[Icon::ViewBankAccount]);
      p->setIcon(0, icon);
    } else {
      p->setIcon(0, QIcon());
    }
    if (!first && (*it).id() == localCurrency)
      first = p;
  }

  QTreeWidgetItemIterator itemsIt = QTreeWidgetItemIterator(m_currencyList, QTreeWidgetItemIterator::All);

  if (first == 0)
    first = *itemsIt;
  if (first != 0) {
    m_currencyList->setCurrentItem(first);
    m_currencyList->setItemSelected(first, true);
    m_currencyList->scrollToItem(first, QTreeView::PositionAtTop);
  }
}

void CurrencyPage::enterPage()
{
  m_currencyList->setFocus();
}

KMyMoneyWizardPage* CurrencyPage::nextPage() const
{
  QString selCur = selectedCurrency();
  QList<MyMoneySecurity> currencies = MyMoneyFile::instance()->availableCurrencyList();
  foreach (auto currency, currencies) {
    if (selCur == currency.id()) {
      m_wizard->m_baseCurrency = currency;
      break;
    }
  }
  m_wizard->m_accountPage->m_accountCurrencyLabel->setText(m_wizard->m_baseCurrency.tradingSymbol());
  return m_wizard->m_accountPage;
}

AccountPage::AccountPage(Wizard* wizard) :
    KAccountPageDecl(wizard),
    WizardPage<Wizard>(stepCount, this, wizard)       // don't inc. the step count here
{
  m_mandatoryGroup->add(m_accountNameEdit);
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  connect(m_haveCheckingAccountButton, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));
  m_accountNameEdit->setFocus();
  m_openingDateEdit->setDate(QDate(QDate::currentDate().year(), 1, 1));
}

KMyMoneyWizardPage* AccountPage::nextPage() const
{
  return m_wizard->m_categoriesPage;
}

bool AccountPage::isComplete() const
{
  return !m_haveCheckingAccountButton->isChecked() || m_mandatoryGroup->isEnabled();
}

CategoriesPage::CategoriesPage(Wizard* wizard) :
    Accounts(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard)
{
}

KMyMoneyWizardPage* CategoriesPage::nextPage() const
{
  return m_wizard->m_preferencePage;
}

QList<MyMoneyTemplate> CategoriesPage::selectedTemplates() const
{
  return m_templateSelector->selectedTemplates();
}

PreferencePage::PreferencePage(Wizard* wizard) :
    KPreferencePageDecl(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard)
{
}

KMyMoneyWizardPage* PreferencePage::nextPage() const
{
  return m_wizard->m_filePage;
}

FilePage::FilePage(Wizard* wizard) :
    KFilePageDecl(wizard),
    WizardPage<Wizard>(stepCount++, this, wizard)
{
  m_mandatoryGroup->add(m_dataFileEdit->lineEdit());
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));

  KUser user;
  QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  if (folder.isEmpty() || !QDir().exists(folder))
    folder = QDir::homePath();
  m_dataFileEdit->setStartDir(QUrl::fromLocalFile(folder));
  m_dataFileEdit->setUrl(QUrl::fromLocalFile(folder + QLatin1Char('/') + user.loginName() + QLatin1String(".kmy")));
  m_dataFileEdit->setFilter(i18n("*.kmy *.xml|KMyMoney files\n*|All files"));
  m_dataFileEdit->setMode(KFile::File);
}

bool FilePage::isComplete() const
{
  //! @todo Allow to overwrite files
  bool rc = m_mandatoryGroup->isEnabled();
  m_existingFileLabel->hide();
  m_finishLabel->show();
  if (rc) {
    // if a filename is present, check that
    // a) the file does not exist
    // b) the directory does exist
    // c) the file is stored locally (because we cannot check previous conditions if it is not)
    const QUrl fullPath = m_dataFileEdit->url();
    QFileInfo directory{fullPath.adjusted(QUrl::RemoveFilename).toLocalFile()};
    qDebug() << "Selected fileptah: " << fullPath << " " << directory.absoluteFilePath() << " dir: " << directory.isDir();
    rc = false;
    if (!fullPath.isValid() || !fullPath.isLocalFile()) {
      m_dataFileEdit->setToolTip(i18n("The path has to be valid and cannot be on a remote location."));
    } else if (QFileInfo::exists(fullPath.toLocalFile())) {
      m_dataFileEdit->setToolTip(i18n("The file exists already. Please create a new file."));
    } else if (!directory.isDir()) {
      m_dataFileEdit->setToolTip(i18n("The destination directory does not exist or cannot be written to."));
    } else {
      m_dataFileEdit->setToolTip("");
      rc = true;
    }

    m_existingFileLabel->setHidden(rc);
    m_finishLabel->setVisible(rc);
  }
  return rc;
}
}
