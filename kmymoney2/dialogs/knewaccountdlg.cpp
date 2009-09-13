/***************************************************************************
                          knewaccountdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <q3header.h>
#include <QToolTip>
#include <QCheckBox>
#include <QTimer>
#include <QTabWidget>
#include <q3buttongroup.h>
#include <QRadioButton>
#include <q3textedit.h>
#include <QLayout>
#include <QTabWidget>
//Added by qt3to4:
#include <QList>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <k3listview.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kled.h>
#include <kdebug.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewaccountdlg.h"

#include <kmymoneyedit.h>
#include <kmymoneydateinput.h>
#include <mymoneyexception.h>
#include <mymoneyfile.h>
#include <kmymoneyaccounttree.h>
#include <kmymoneyglobalsettings.h>
#include <mymoneyreport.h>
#include <kguiutils.h>
#include <kmymoneycombo.h>

#include "kmymoneycurrencyselector.h"
#include "kmymoneyaccountselector.h"

#include "mymoneyexception.h"
#include "mymoneykeyvaluecontainer.h"
#include "knewbankdlg.h"
#include "kmymoneyfile.h"
#include "kmymoneyutils.h"

//#include "kreportchartview.h"
//#include "pivottable.h"

// in KOffice version < 1.5 KDCHART_PROPSET_NORMAL_DATA was a static const
// but in 1.5 this has been changed into a #define'd value. So we have to
// make sure, we use the right one.
#ifndef KDCHART_PROPSET_NORMAL_DATA
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDChartParams::KDCHART_PROPSET_NORMAL_DATA
#else
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDCHART_PROPSET_NORMAL_DATA
#endif

KNewAccountDlg::KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent, const char *name, const QString& title)
  : KNewAccountDlgDecl(parent),
    m_account(account),
    m_bSelectedParentAccount(false),
    m_categoryEditor(categoryEditor),
    m_isEditing(isEditing)
{
  QString columnName = ( (categoryEditor) ? i18n("Categories") : i18n("Accounts") );

  m_qlistviewParentAccounts->setRootIsDecorated(true);
  m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
  m_qlistviewParentAccounts->setSectionHeader(columnName);
  m_qlistviewParentAccounts->setMultiSelection(false);
  m_qlistviewParentAccounts->header()->setResizeEnabled(true);
  m_qlistviewParentAccounts->setColumnWidthMode(0, Q3ListView::Maximum);
  m_qlistviewParentAccounts->setEnabled(false);
  // never show the horizontal scroll bar
  m_qlistviewParentAccounts->setHScrollBarMode(Q3ScrollView::AlwaysOff);

  m_subAccountLabel->setText(i18n("Is a sub account"));

  m_qlistviewParentAccounts->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  accountNameEdit->setText(account.name());
  descriptionEdit->setText(account.description());

  typeCombo->setEnabled(true);
  MyMoneyFile *file = MyMoneyFile::instance();

  // load the price mode combo
  m_priceMode->insertItem(i18nc("default price mode", "<default>"), 0);
  m_priceMode->insertItem(i18n("Price per share"), 1);
  m_priceMode->insertItem(i18n("Total for all shares"), 2);

  int priceMode = 0;
  if(m_account.accountType() == MyMoneyAccount::Investment) {
    m_priceMode->setEnabled(true);
    if(!m_account.value("priceMode").isEmpty())
      priceMode = m_account.value("priceMode").toInt();
  }
  m_priceMode->setCurrentItem(priceMode);

  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  if (categoryEditor)
  {
    // get rid of the tabs that are not used for categories
    QWidget* tab = m_tab->page(m_tab->indexOf(m_institutionTab));
    if(tab)
      m_tab->removePage(tab);
    tab = m_tab->page(m_tab->indexOf(m_limitsTab));
    if(tab)
      m_tab->removePage(tab);

    //m_qlistviewParentAccounts->setEnabled(true);
    startDateEdit->setEnabled(false);
    accountNoEdit->setEnabled(false);

    m_institutionBox->hide();
    m_qcheckboxNoVat->hide();

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Income));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Expense));

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      case MyMoneyAccount::Income:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Income), false);
        break;

      case MyMoneyAccount::Expense:
      default:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Expense), false);
        break;
    }
    m_currency->setEnabled(true);
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
      m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
    }
    m_qcheckboxPreferred->hide();

    m_qcheckboxTax->setChecked(account.value("Tax") == "Yes");
    loadVatAccounts();
  }
  else
  {
    // get rid of the tabs that are not used for accounts
    QWidget* taxtab = m_tab->page(m_tab->indexOf(m_taxTab));
    if (taxtab) {
      if(m_account.isAssetLiability()) {
        m_vatCategory->setText(i18n( "VAT account"));
        m_vatAssignmentFrame->hide();
        m_qcheckboxTax->setChecked(account.value("Tax") == "Yes");
      } else {
        m_tab->removePage(taxtab);
      }
    }

    switch(m_account.accountType()) {
      case MyMoneyAccount::Savings:
      case MyMoneyAccount::Cash:
        haveMinBalance = true;
        break;

      case MyMoneyAccount::Checkings:
        haveMinBalance = true;
        haveMaxCredit = true;
        break;

      case MyMoneyAccount::CreditCard:
        haveMaxCredit = true;
        break;

      default:
        // no limit available, so we might get rid of the tab
        QWidget* tab = m_tab->page(m_tab->indexOf(m_limitsTab));
        if(tab)
          m_tab->removePage(tab);
        // don't try to hide the widgets we just wiped
        // in the next step
        haveMaxCredit = haveMinBalance = true;
        break;
    }

    if(!haveMaxCredit) {
      m_maxCreditLabel->setEnabled(false);
      m_maxCreditLabel->hide();
      m_maxCreditEarlyEdit->hide();
      m_maxCreditAbsoluteEdit->hide();
    }
    if(!haveMinBalance) {
      m_minBalanceLabel->setEnabled(false);
      m_minBalanceLabel->hide();
      m_minBalanceEarlyEdit->hide();
      m_minBalanceAbsoluteEdit->hide();
    }

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Stock));
/*
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CertificateDep));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
*/

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      default:
      case MyMoneyAccount::Checkings:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings), false);
        break;
      case MyMoneyAccount::Savings:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings), false);
        break;
      case MyMoneyAccount::Cash:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash), false);
        break;
      case MyMoneyAccount::CreditCard:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard), false);
        break;
      case MyMoneyAccount::Loan:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan), false);
        break;
      case MyMoneyAccount::Investment:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment), false);
        break;
      case MyMoneyAccount::Asset:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset), false);
        break;
      case MyMoneyAccount::Liability:
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability), false);
        break;
      case MyMoneyAccount::Stock:
        m_institutionBox->hide();
        typeCombo->setCurrentItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Stock), false);
        break;
/*
      case MyMoneyAccount::CertificateDep:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::MoneyMarket:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Currency:
        typeCombo->setCurrentItem(8);
        break;
*/
    }

    if(!m_account.openingDate().isValid())
      m_account.setOpeningDate(QDate::currentDate());

    startDateEdit->setDate(m_account.openingDate());
    accountNoEdit->setText(account.number());
    m_qcheckboxPreferred->setChecked(account.value("PreferredAccount") == "Yes");
    m_qcheckboxNoVat->setChecked(account.value("NoVat") == "Yes");
    loadKVP("iban", ibanEdit);
    loadKVP("minBalanceAbsolute", m_minBalanceAbsoluteEdit);
    loadKVP("minBalanceEarly", m_minBalanceEarlyEdit);
    loadKVP("maxCreditAbsolute", m_maxCreditAbsoluteEdit);
    loadKVP("maxCreditEarly", m_maxCreditEarlyEdit);
    // reverse the sign for display purposes
    if(!m_maxCreditAbsoluteEdit->lineedit()->text().isEmpty())
      m_maxCreditAbsoluteEdit->setValue(m_maxCreditAbsoluteEdit->value()*MyMoneyMoney(-1,1));
    if(!m_maxCreditEarlyEdit->lineedit()->text().isEmpty())
      m_maxCreditEarlyEdit->setValue(m_maxCreditEarlyEdit->value()*MyMoneyMoney(-1,1));
    loadKVP("lastNumberUsed", m_lastCheckNumberUsed);


    // we do not allow to change the account type once an account
    // was created. Same applies to currency if it is referenced.
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
      m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
    }
    if(m_account.isInvest()) {
      typeCombo->setEnabled(false);
      m_qcheckboxPreferred->hide();
      m_currencyText->hide();
      m_currency->hide();
    } else {
      // use the old field and override a possible new value
      if(!MyMoneyMoney(account.value("minimumBalance")).isZero()) {
        m_minBalanceAbsoluteEdit->setValue(MyMoneyMoney(account.value("minimumBalance")));
      }
    }

//    m_qcheckboxTax->hide(); TODO should only be visible for VAT category/account
  }

  m_currency->setSecurity(file->currency(account.currencyId()));

  // Load the institutions
  // then the accounts
  QString institutionName;

  try
  {
    if (m_isEditing && !account.institutionId().isEmpty())
      institutionName = file->institution(account.institutionId()).name();
    else
      institutionName = QString();
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", qPrintable(e->what()));
    delete e;
  }

  initParentWidget(account.parentAccountId(), account.id());
  if(m_account.isInvest())
    m_qlistviewParentAccounts->setEnabled(false);

  if (!categoryEditor)
    slotLoadInstitutions(institutionName);

  accountNameEdit->setFocus();

  if (!title.isEmpty())
    setWindowTitle(title);

  // load button icons
  KIconLoader* il = KIconLoader::global();
  cancelButton->setGuiItem(KStandardGuiItem::cancel());
  createButton->setGuiItem(KStandardGuiItem::ok());

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qlistviewParentAccounts, SIGNAL(selectionChanged(Q3ListViewItem*)),
    this, SLOT(slotSelectionChanged(Q3ListViewItem*)));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(typeCombo, SIGNAL(activated(const QString&)),
    this, SLOT(slotAccountTypeChanged(const QString&)));

  connect(accountNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));

  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotVatChanged(bool)));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotVatAssignmentChanged(bool)));
  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));
  connect(m_vatAccount, SIGNAL(stateChanged()), this, SLOT(slotCheckFinished()));

  connect(m_minBalanceEarlyEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMinBalanceAbsoluteEdit(const QString&)));
  connect(m_minBalanceAbsoluteEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMinBalanceEarlyEdit(const QString&)));
  connect(m_maxCreditEarlyEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMaxCreditAbsoluteEdit(const QString&)));
  connect(m_maxCreditAbsoluteEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMaxCreditEarlyEdit(const QString&)));

  connect(m_qcomboboxInstitutions, SIGNAL(activated(const QString&)), this, SLOT(slotLoadInstitutions(const QString&)));

  m_vatCategory->setChecked(false);
  m_vatAssignment->setChecked(false);

  // make sure our account does not have an id and no parent assigned
  // and certainly no children in case we create a new account
  if(!m_isEditing) {
    m_account.clearId();
    m_account.setParentAccountId(QString());
    QStringList::ConstIterator it;
    while((it = m_account.accountList().begin()) != m_account.accountList().end())
      m_account.removeAccountId(*it);

    if(m_parentItem == 0) {
      // force loading of initial parent
      m_account.setAccountType(MyMoneyAccount::UnknownAccountType);
      MyMoneyAccount::_accountTypeE type = account.accountType();
      if(type == MyMoneyAccount::UnknownAccountType)
        type = MyMoneyAccount::Checkings;
      slotAccountTypeChanged(KMyMoneyUtils::accountTypeToString(type));
    }
  } else {
    if(!m_account.value("VatRate").isEmpty()) {
      m_vatCategory->setChecked(true);
      m_vatRate->setValue(MyMoneyMoney(m_account.value("VatRate"))*MyMoneyMoney(100,1));
    } else {
      if(!m_account.value("VatAccount").isEmpty()) {
        QString accId = m_account.value("VatAccount").toLatin1();
        try {
          // make sure account exists
          MyMoneyFile::instance()->account(accId);
          m_vatAssignment->setChecked(true);
          m_vatAccount->setSelected(accId);
          m_grossAmount->setChecked(true);
          if(m_account.value("VatAmount") == "Net")
            m_netAmount->setChecked(true);
        } catch(MyMoneyException *e) {
          delete e;
        }
      }
    }
  }
  slotVatChanged(m_vatCategory->isChecked());
  slotVatAssignmentChanged(m_vatAssignment->isChecked());
  slotCheckFinished();

  kMandatoryFieldGroup* requiredFields = new kMandatoryFieldGroup (this);
  requiredFields->setOkButton(createButton); // button to be enabled when all fields present
  requiredFields->add(accountNameEdit);

  // using a timeout is the only way, I got the 'ensureItemVisible'
  // working when creating the dialog. I assume, this
  // has something to do with the delayed update of the display somehow.
  QTimer::singleShot(50, this, SLOT(timerDone()));
}

void KNewAccountDlg::timerDone(void)
{
  if(m_accountItem) m_qlistviewParentAccounts->ensureItemVisible(m_accountItem);
  if(m_parentItem) m_qlistviewParentAccounts->ensureItemVisible(m_parentItem);
  // KNewAccountDlgDecl::resizeEvent(0);
  m_qlistviewParentAccounts->setColumnWidth(m_qlistviewParentAccounts->nameColumn(), m_qlistviewParentAccounts->visibleWidth());
  m_qlistviewParentAccounts->repaintContents(false);
}

void KNewAccountDlg::setOpeningBalance(const MyMoneyMoney& balance)
{
  m_openingBalanceEdit->setValue(balance);
}

void KNewAccountDlg::setOpeningBalanceShown(bool shown)
{
  m_openingBalanceEdit->setVisible(shown);
}

void KNewAccountDlg::okClicked()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty())
  {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
    accountNameEdit->setFocus();
    return;
  }

  MyMoneyAccount parent = parentAccount();
  if (parent.name().length() == 0)
  {
    KMessageBox::error(this, i18n("Please select a parent account."));
    return;
  }

  if (!m_categoryEditor)
  {
    QString institutionNameText = m_qcomboboxInstitutions->currentText();
    if (institutionNameText != i18n("<No Institution>"))
    {
      try
      {
        MyMoneyFile *file = MyMoneyFile::instance();

        QList<MyMoneyInstitution> list = file->institutionList();
        QList<MyMoneyInstitution>::ConstIterator institutionIterator;
        for (institutionIterator = list.constBegin(); institutionIterator != list.constEnd(); ++institutionIterator)
        {
          if ((*institutionIterator).name() == institutionNameText)
            m_account.setInstitutionId((*institutionIterator).id());
        }
      }
      catch (MyMoneyException *e)
      {
        qDebug("Exception in account institution set: %s", qPrintable(e->what()));
        delete e;
      }
    }
    else
    {
      m_account.setInstitutionId(QString());
    }
  }

  m_account.setName(accountNameText);
  m_account.setNumber(accountNoEdit->text());
  storeKVP("iban", ibanEdit);
  storeKVP("minBalanceAbsolute", m_minBalanceAbsoluteEdit);
  storeKVP("minBalanceEarly", m_minBalanceEarlyEdit);

  // the figures for credit line with reversed sign
  if(!m_maxCreditAbsoluteEdit->lineedit()->text().isEmpty())
    m_maxCreditAbsoluteEdit->setValue(m_maxCreditAbsoluteEdit->value()*MyMoneyMoney(-1,1));
  if(!m_maxCreditEarlyEdit->lineedit()->text().isEmpty())
    m_maxCreditEarlyEdit->setValue(m_maxCreditEarlyEdit->value()*MyMoneyMoney(-1,1));
  storeKVP("maxCreditAbsolute", m_maxCreditAbsoluteEdit);
  storeKVP("maxCreditEarly", m_maxCreditEarlyEdit);
  if(!m_maxCreditAbsoluteEdit->lineedit()->text().isEmpty())
    m_maxCreditAbsoluteEdit->setValue(m_maxCreditAbsoluteEdit->value()*MyMoneyMoney(-1,1));
  if(!m_maxCreditEarlyEdit->lineedit()->text().isEmpty())
    m_maxCreditEarlyEdit->setValue(m_maxCreditEarlyEdit->value()*MyMoneyMoney(-1,1));

  storeKVP("lastNumberUsed", m_lastCheckNumberUsed);
  // delete a previous version of the minimumbalance information
  storeKVP("minimumBalance", QString(), QString());

  MyMoneyAccount::accountTypeE acctype;
  if (!m_categoryEditor)
  {
    acctype = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
    // If it's a loan, check if the parent is asset or liability. In
    // case of asset, we change the account type to be AssetLoan
    if(acctype == MyMoneyAccount::Loan
    && parent.accountGroup() == MyMoneyAccount::Asset)
      acctype = MyMoneyAccount::AssetLoan;
  }
  else
  {
    acctype = parent.accountGroup();
    QString newName;
    if(!MyMoneyFile::instance()->isStandardAccount(parent.id())) {
      newName = MyMoneyFile::instance()->accountToCategory(parent.id()) + MyMoneyFile::AccountSeperator;
    }
    newName += accountNameText;
    if(!file->categoryToAccount(newName, acctype).isEmpty()
    && (file->categoryToAccount(newName, acctype) != m_account.id())) {
      KMessageBox::error(this, QString("<qt>")+i18n("A category named <b>%1</b> already exists. You cannot create a second category with the same name.",newName)+QString("</qt>"));
      return;
    }
  }
  m_account.setAccountType(acctype);

  m_account.setDescription(descriptionEdit->text());

  if (!m_categoryEditor)
  {
    m_account.setOpeningDate(startDateEdit->date());
    m_account.setCurrencyId(m_currency->security().id());

    if(m_qcheckboxPreferred->isChecked())
      m_account.setValue("PreferredAccount", "Yes");
    else
      m_account.deletePair("PreferredAccount");
    if(m_qcheckboxNoVat->isChecked())
      m_account.setValue("NoVat", "Yes");
    else
      m_account.deletePair("NoVat");

    if(m_minBalanceAbsoluteEdit->isVisible()) {
      m_account.setValue("minimumBalance", m_minBalanceAbsoluteEdit->value().toString());
    }
  }
  else
  {
    if(KMyMoneyGlobalSettings::hideUnusedCategory() && !m_isEditing) {
      KMessageBox::information(this, i18n("You have selected to suppress the display of unused categories in the KMyMoney configuration dialog. The category you just created will therefore only be shown if it is used. Otherwise, it will be hidden in the accounts/categories view."), i18n("Hidden categories"), "NewHiddenCategory");
    }
  }

  if ( m_qcheckboxTax->isChecked())
    m_account.setValue("Tax", "Yes");
  else
    m_account.deletePair("Tax");

  m_account.deletePair("VatAccount");
  m_account.deletePair("VatAmount");
  m_account.deletePair("VatRate");

  if(m_vatCategory->isChecked()) {
    m_account.setValue("VatRate", (m_vatRate->value().abs() / MyMoneyMoney(100,1)).toString());
  } else {
    if(m_vatAssignment->isChecked() && !m_vatAccount->selectedItems().isEmpty()) {
      m_account.setValue("VatAccount", m_vatAccount->selectedItems().first());
      if(m_netAmount->isChecked())
        m_account.setValue("VatAmount", "Net");
    }
  }

  accept();
}

void KNewAccountDlg::loadKVP(const QString& key, kMyMoneyEdit* widget)
{
  if(!widget)
    return;

  if(m_account.value(key).isEmpty()) {
    widget->clearText();
  } else {
    widget->setValue(MyMoneyMoney(m_account.value(key)));
  }
}

void KNewAccountDlg::loadKVP(const QString& key, KLineEdit* widget)
{
  if(!widget)
    return;

  widget->setText(m_account.value(key));
}

void KNewAccountDlg::storeKVP(const QString& key, const QString& text, const QString& value)
{
  if(text.isEmpty())
    m_account.deletePair(key);
  else
    m_account.setValue(key, value);
}

void KNewAccountDlg::storeKVP(const QString& key, kMyMoneyEdit* widget)
{
  storeKVP(key, widget->lineedit()->text(), widget->text());
}

void KNewAccountDlg::storeKVP(const QString& key, KLineEdit* widget)
{
  storeKVP(key, widget->text(), widget->text());
}

const MyMoneyAccount& KNewAccountDlg::account(void)
{
  // assign the right currency to the account
  m_account.setCurrencyId(m_currency->security().id());

  // and the price mode
  switch(m_priceMode->currentItem()) {
    case 0:
      m_account.deletePair("priceMode");
      break;
    case 1:
    case 2:
      m_account.setValue("priceMode", QString("%1").arg(m_priceMode->currentItem()));
      break;
  }

  return m_account;
}

const MyMoneyAccount& KNewAccountDlg::parentAccount(void)
{
  if (!m_bSelectedParentAccount)
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    switch (m_account.accountGroup())
    {
      case MyMoneyAccount::Asset:
        m_parentAccount = file->asset();
        break;
      case MyMoneyAccount::Liability:
        m_parentAccount = file->liability();
        break;
      case MyMoneyAccount::Income:
        m_parentAccount = file->income();
        break;
      case MyMoneyAccount::Expense:
        m_parentAccount = file->expense();
        break;
      case MyMoneyAccount::Equity:
        m_parentAccount = file->equity();
        break;
      default:
        qDebug("Seems we have an account that hasn't been mapped to the top five");
        if(m_categoryEditor)
          m_parentAccount = file->income();
        else
          m_parentAccount = file->asset();
    }
  }
  return m_parentAccount;
}

void KNewAccountDlg::initParentWidget(QString parentId, const QString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();
  MyMoneyAccount expenseAccount = file->expense();
  MyMoneyAccount incomeAccount = file->income();
  MyMoneyAccount equityAccount = file->equity();

  m_parentItem = 0;
  m_accountItem = 0;

  // Determine the parent account
  try
  {
    m_parentAccount = file->account(parentId);
  }
  catch (MyMoneyException *e)
  {
    m_bSelectedParentAccount = false;
    m_parentAccount = MyMoneyAccount();
    if(m_account.accountType() != MyMoneyAccount::UnknownAccountType) {
      parentAccount();
      parentId = m_parentAccount.id();
    }
    delete e;
  }
  m_bSelectedParentAccount = true;

  // extract the account type from the combo box
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE groupType;
  type = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
  groupType = MyMoneyAccount::accountGroup(type);

  m_qlistviewParentAccounts->clear();

  // Now scan all 4 account roots to load the list and mark the parent
  try
  {
    if (!m_categoryEditor)
    {
      if(groupType == MyMoneyAccount::Asset || type == MyMoneyAccount::Loan) {
        // Asset
        KMyMoneyAccountTreeBaseItem *assetTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts, assetAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = assetAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == assetAccount.id())
          m_parentItem = assetTopLevelAccount;

        assetTopLevelAccount->setOpen(true);

        for ( QStringList::ConstIterator it = assetAccount.accountList().begin();
              it != assetAccount.accountList().end();
              ++it )
        {
          MyMoneyAccount acc = file->account(*it);
          if(acc.isClosed())
            continue;

          KMyMoneyAccountTreeBaseItem *accountItem = new KMyMoneyAccountTreeItem(assetTopLevelAccount, acc);

          if(parentId == acc.id()) {
            m_parentItem = accountItem;
          } else if(accountId == acc.id()) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QStringList subAccounts = acc.accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, acc.id());
          }
        }
      }

      if(groupType == MyMoneyAccount::Liability) {
        // Liability
        KMyMoneyAccountTreeBaseItem *liabilityTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts, liabilityAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = liabilityAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == liabilityAccount.id())
          m_parentItem = liabilityTopLevelAccount;

        liabilityTopLevelAccount->setOpen(true);

        for ( QStringList::ConstIterator it = liabilityAccount.accountList().begin();
              it != liabilityAccount.accountList().end();
              ++it )
        {
          MyMoneyAccount acc = file->account(*it);
          if(acc.isClosed())
            continue;

          KMyMoneyAccountTreeBaseItem *accountItem = new KMyMoneyAccountTreeItem(liabilityTopLevelAccount, acc);

          if(parentId == acc.id()) {
            m_parentItem = accountItem;
          } else if(accountId == acc.id()) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QStringList subAccounts = acc.accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, acc.id());
          }
        }
      }
    }
    else
    {
      if(groupType == MyMoneyAccount::Income) {
        // Income
        KMyMoneyAccountTreeBaseItem *incomeTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts,
                          incomeAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = incomeAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == incomeAccount.id())
          m_parentItem = incomeTopLevelAccount;

        incomeTopLevelAccount->setOpen(true);

        for ( QStringList::ConstIterator it = incomeAccount.accountList().begin();
              it != incomeAccount.accountList().end();
              ++it )
        {
          KMyMoneyAccountTreeBaseItem *accountItem = new KMyMoneyAccountTreeItem(incomeTopLevelAccount,
              file->account(*it));

          QString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }

      if(groupType == MyMoneyAccount::Expense) {
        // Expense
        KMyMoneyAccountTreeBaseItem *expenseTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts,
                          expenseAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = expenseAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == expenseAccount.id())
          m_parentItem = expenseTopLevelAccount;

        expenseTopLevelAccount->setOpen(true);

        for ( QStringList::ConstIterator it = expenseAccount.accountList().begin();
              it != expenseAccount.accountList().end();
              ++it )
        {
          KMyMoneyAccountTreeBaseItem *accountItem = new KMyMoneyAccountTreeItem(expenseTopLevelAccount,
              file->account(*it));

          QString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", qPrintable(e->what()));
    delete e;
  }

  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  if (m_parentItem)
  {
    m_subAccountLabel->setText(i18n("Is a sub account of %1",m_parentAccount.name()));
    m_parentItem->setOpen(true);
    m_qlistviewParentAccounts->setSelected(m_parentItem, true);
  }

  m_qlistviewParentAccounts->setEnabled(true);
}

void KNewAccountDlg::showSubAccounts(QStringList accounts, KMyMoneyAccountTreeBaseItem *parentItem,
                                     const QString& parentId, const QString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  for ( QStringList::ConstIterator it = accounts.constBegin(); it != accounts.constEnd(); ++it )
  {
    KMyMoneyAccountTreeBaseItem *accountItem  = new KMyMoneyAccountTreeItem(parentItem,
          file->account(*it));

    QString id = file->account(*it).id();
    if(parentId == id) {
      m_parentItem = accountItem;
    } else if(accountId == id) {
      if(m_isEditing)
        accountItem->setSelectable(false);
      m_accountItem = accountItem;
    }

    QStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, parentId, accountId);
    }
  }
}

void KNewAccountDlg::resizeEvent(QResizeEvent* e)
{
  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  // call base class resizeEvent()
  KNewAccountDlgDecl::resizeEvent(e);
}

void KNewAccountDlg::slotSelectionChanged(Q3ListViewItem *item)
{
  KMyMoneyAccountTreeBaseItem *accountItem = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(item);
  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    //qDebug("Selected account id: %s", accountItem->accountID().data());
    m_parentAccount = file->account(accountItem->id());
    m_subAccountLabel->setText(i18n("Is a sub account of %1",m_parentAccount.name()));
    if(m_qlistviewParentAccounts->isEnabled()) {
      m_bSelectedParentAccount = true;
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("This shouldn't happen! : %s", qPrintable(e->what()));
    delete e;
  }
}

void KNewAccountDlg::loadVatAccounts(void)
{
  QList<MyMoneyAccount> list;
  MyMoneyFile::instance()->accountList(list);
  QList<MyMoneyAccount>::Iterator it;
  QStringList loadListExpense;
  QStringList loadListIncome;
  QStringList loadListAsset;
  QStringList loadListLiability;
  for(it = list.begin(); it != list.end(); ++it) {
    if(!(*it).value("VatRate").isEmpty()) {
      if((*it).accountType() == MyMoneyAccount::Expense)
        loadListExpense += (*it).id();
      else if((*it).accountType() == MyMoneyAccount::Income)
        loadListIncome += (*it).id();
      else if((*it).accountType() == MyMoneyAccount::Asset)
        loadListAsset += (*it).id();
      else if((*it).accountType() == MyMoneyAccount::Liability)
        loadListLiability += (*it).id();
    }
  }
  AccountSet vatSet;
  if(!loadListAsset.isEmpty())
    vatSet.load(m_vatAccount, i18n("Asset"), loadListAsset, true);
  if(!loadListLiability.isEmpty())
    vatSet.load(m_vatAccount, i18n("Liability"), loadListLiability, false);
  if(!loadListIncome.isEmpty())
    vatSet.load(m_vatAccount, i18n("Income"), loadListIncome, false);
  if(!loadListExpense.isEmpty())
    vatSet.load(m_vatAccount, i18n("Expense"), loadListExpense, false);
}

void KNewAccountDlg::slotLoadInstitutions(const QString& name)
{
  m_qcomboboxInstitutions->clear();
  QString bic;
  // Are we forcing the user to use institutions?
  m_qcomboboxInstitutions->insertItem(i18n("<No Institution>"));
  m_bicValue->setText(" ");
  ibanEdit->setEnabled(false);
  accountNoEdit->setEnabled(false);
  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    QList<MyMoneyInstitution> list = file->institutionList();
    QList<MyMoneyInstitution>::ConstIterator institutionIterator;
    for (institutionIterator = list.constBegin(); institutionIterator != list.constEnd(); ++institutionIterator)
    {
      if ((*institutionIterator).name() == name) {
        ibanEdit->setEnabled(true);
        accountNoEdit->setEnabled(true);
        m_bicValue->setText((*institutionIterator).value("bic"));
      }
      m_qcomboboxInstitutions->insertItem((*institutionIterator).name());
    }

    m_qcomboboxInstitutions->setCurrentItem(name, false);
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in institution load: %s", qPrintable(e->what()));
    delete e;
  }
}

void KNewAccountDlg::slotNewClicked()
{
  MyMoneyInstitution institution;

  QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution, this);
  if (dlg->exec())
  {
    MyMoneyFileTransaction ft;
    try
    {
      MyMoneyFile *file = MyMoneyFile::instance();

      institution = dlg->institution();
      file->addInstitution(institution);
      ft.commit();
      slotLoadInstitutions(institution.name());
    }
    catch (MyMoneyException *e)
    {
      delete e;
      KMessageBox::information(this, i18n("Cannot add institution"));
    }
  }
  delete dlg;
}

void KNewAccountDlg::slotAccountTypeChanged(const QString& typeStr)
{
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE oldType;
  MyMoneyFile* file = MyMoneyFile::instance();

  type = KMyMoneyUtils::stringToAccountType(typeStr);
  try {
    oldType = m_account.accountType();
    if(oldType != type) {
      QString parentId;
      switch(MyMoneyAccount::accountGroup(type)) {
        case MyMoneyAccount::Asset:
          parentId = file->asset().id();
          break;
        case MyMoneyAccount::Liability:
          parentId = file->liability().id();
          break;
        case MyMoneyAccount::Expense:
          parentId = file->expense().id();
          break;
        case MyMoneyAccount::Income:
          parentId = file->income().id();
          break;
        default:
          qWarning("Unknown account group in KNewAccountDlg::slotAccountTypeChanged()");
          break;
      }
      initParentWidget(parentId, QString());
      m_account.setAccountType(type);
    }
  } catch(MyMoneyException *e) {
    delete e;
    qWarning("Unexpected exception in KNewAccountDlg::slotAccountTypeChanged()");
  }
}

void KNewAccountDlg::slotCheckFinished(void)
{
  bool showButton = true;

  if(accountNameEdit->text().length() == 0) {
    showButton = false;
  }

  if(m_vatCategory->isChecked() && m_vatRate->value() <= MyMoneyMoney(0)) {
    showButton = false;
  } else {
    if(m_vatAssignment->isChecked() && m_vatAccount->selectedItems().isEmpty())
      showButton = false;
  }
  createButton->setEnabled(showButton);
}

void KNewAccountDlg::slotVatChanged(bool state)
{
  if(state) {
    m_vatCategoryFrame->show();
    m_vatAssignmentFrame->hide();
  } else {
    m_vatCategoryFrame->hide();
    if(!m_account.isAssetLiability()) {
        m_vatAssignmentFrame->show();
    }
  }
}

void KNewAccountDlg::slotVatAssignmentChanged(bool state)
{
  m_vatAccount->setEnabled(state);
  m_amountGroup->setEnabled(state);
}

void KNewAccountDlg::adjustEditWidgets(kMyMoneyEdit* dst, kMyMoneyEdit* src, char mode, int corr)
{
  MyMoneyMoney factor(corr, 1);
  if(m_account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  switch(mode) {
    case '<':
      if(src->value()*factor < dst->value()*factor)
        dst->setValue(src->value());
      break;

    case '>':
      if(src->value()*factor > dst->value()*factor)
        dst->setValue(src->value());
      break;
  }
}

void KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit(const QString&)
{
  adjustEditWidgets(m_minBalanceAbsoluteEdit, m_minBalanceEarlyEdit, '<', -1);
}

void KNewAccountDlg::slotAdjustMinBalanceEarlyEdit(const QString&)
{
  adjustEditWidgets(m_minBalanceEarlyEdit, m_minBalanceAbsoluteEdit, '>', -1);
}

void KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit(const QString&)
{
  adjustEditWidgets(m_maxCreditAbsoluteEdit, m_maxCreditEarlyEdit, '<', 1);
}

void KNewAccountDlg::slotAdjustMaxCreditEarlyEdit(const QString&)
{
  adjustEditWidgets(m_maxCreditEarlyEdit, m_maxCreditAbsoluteEdit, '>', 1);
}

void KNewAccountDlg::addTab(QWidget* w, const QString& name)
{
  if(w) {
    w->reparent(m_tab, QPoint(0,0));
    m_tab->addTab(w, name);
  }
}


#include "knewaccountdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
