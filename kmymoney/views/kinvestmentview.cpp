/***************************************************************************
                          kinvestmentview.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
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

#include "kinvestmentview.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <KToggleAction>
#include <KMessageBox>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <mymoneyutils.h>
#include <mymoneysecurity.h>
#include <mymoneytransaction.h>
#include <mymoneyinvesttransaction.h>
#include <mymoneyaccount.h>
#include <kmymoneyglobalsettings.h>
#include <kmymoneyaccountcombo.h>
#include <kmymoneycurrencyselector.h>
#include <knewinvestmentwizard.h>
#include "kmymoney.h"
#include "models.h"

/**
  * This class is only needed to implement proper sorting.
  */
class InvestmentItem : public QTreeWidgetItem
{
public:
  InvestmentItem(QTreeWidget *view) : QTreeWidgetItem(view) {}
  virtual bool operator<(const QTreeWidgetItem &other) const;
};

bool InvestmentItem::operator<(const QTreeWidgetItem &other) const
{
  const int sortColumn = treeWidget()->sortColumn();
  if (sortColumn > eInvestmentSymbolColumn) {
    // these columns have a MyMoneyMoney value in the Qt::UserRole role
    const MyMoneyMoney &money = data(sortColumn, Qt::UserRole).value<MyMoneyMoney>();
    const MyMoneyMoney &otherMoney = other.data(sortColumn, Qt::UserRole).value<MyMoneyMoney>();
    return money < otherMoney;
  }
  return QTreeWidgetItem::operator<(other);
}

class KInvestmentView::Private
{
public:
  Private() :
      m_newAccountLoaded(false),
      m_recursion(false),
      m_precision(2),
      m_filterProxyModel(0) {}

  MyMoneyAccount    m_account;
  bool              m_needReload[MaxViewTabs];
  bool              m_newAccountLoaded;
  bool              m_recursion;
  int               m_precision;
  AccountNamesFilterProxyModel *m_filterProxyModel;

};

KInvestmentView::KInvestmentView(QWidget *parent) :
    QWidget(parent),
    d(new Private),
    m_currencyMarket("ISO 4217")
{
  setupUi(this);

  // load the header state of the equities list
  KConfigGroup grp = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
  QByteArray columns;
  columns = grp.readEntry("HeaderState", columns);
  m_investmentsList->header()->restoreState(columns);

  // load the header state of the securities list
  grp = KSharedConfig::openConfig()->group("KInvestmentView_Securities");
  columns.clear();
  columns = grp.readEntry("HeaderState", columns);
  m_securitiesList->header()->restoreState(columns);

  //first set up everything for the equities tab
  d->m_filterProxyModel = new AccountNamesFilterProxyModel(this);
  d->m_filterProxyModel->addAccountType(MyMoneyAccount::Investment);
  d->m_filterProxyModel->setHideEquityAccounts(false);
  d->m_filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  d->m_filterProxyModel->sort(0);
  m_accountComboBox->setModel(d->m_filterProxyModel);

  m_investmentsList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_investmentsList->setSortingEnabled(true);

  for (int i = 0; i < MaxViewTabs; ++i)
    d->m_needReload[i] = false;

  connect(m_tab, SIGNAL(currentChanged(int)), this, SLOT(slotTabCurrentChanged(int)));

  connect(m_investmentsList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(slotInvestmentContextMenu(QPoint)));
  connect(m_investmentsList, SIGNAL(itemSelectionChanged()), this, SLOT(slotInvestmentSelectionChanged()));
  connect(m_accountComboBox, SIGNAL(accountSelected(QString)),
          this, SLOT(slotSelectAccount(QString)));
  connect(m_investmentsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), kmymoney->action("investment_edit"), SLOT(trigger()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));

  // create the searchline widget
  // and insert it into the existing layout
  m_searchSecuritiesWidget = new KTreeWidgetSearchLineWidget(this, m_securitiesList);
  m_searchSecuritiesWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_securitiesLayout->insertWidget(0, m_searchSecuritiesWidget);

  KGuiItem removeButtonItem(i18n("&Delete"),
                            QIcon::fromTheme("edit-delete"),
                            i18n("Delete this entry"),
                            i18n("Remove this security item from the file"));
  KGuiItem::assign(m_deleteSecurityButton, removeButtonItem);

  KGuiItem editButtonItem(i18n("&Edit"),
                          QIcon::fromTheme("document-edit"),
                          i18n("Modify the selected entry"),
                          i18n("Change the security information of the selected entry."));
  KGuiItem::assign(m_editSecurityButton, editButtonItem);

  connect(m_showCurrencyButton, SIGNAL(toggled(bool)), this, SLOT(slotLoadView()));
  connect(m_securitiesList, SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateSecuritiesButtons()));
  connect(m_editSecurityButton, SIGNAL(clicked()), this, SLOT(slotEditSecurity()));
  connect(m_deleteSecurityButton, SIGNAL(clicked()), this, SLOT(slotDeleteSecurity()));
}

KInvestmentView::~KInvestmentView()
{
  // save the header state of the equities list
  KConfigGroup grp = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
  QByteArray columns = m_investmentsList->header()->saveState();
  grp.writeEntry("HeaderState", columns);

  // save the header state of the securities list
  grp = KSharedConfig::openConfig()->group("KInvestmentView_Securities");
  columns = m_securitiesList->header()->saveState();
  grp.writeEntry("HeaderState", columns);

  delete d;
}

void KInvestmentView::loadView(InvestmentsViewTab tab)
{
  if (d->m_needReload[tab]) {
    switch (tab) {
      case EquitiesTab:
        loadInvestmentTab();
        // force a new account if the current one is empty
        d->m_newAccountLoaded = d->m_account.id().isEmpty();
        break;
      case SecuritiesTab:
        loadSecuritiesList();
        break;
      default:
        break;
    }
    d->m_needReload[tab] = false;
  }
}

void KInvestmentView::slotInvestmentSelectionChanged()
{
  kmymoney->slotSelectInvestment();

  QTreeWidgetItem *item = m_investmentsList->currentItem();
  if (item) {
    try {
      MyMoneyAccount account = MyMoneyFile::instance()->account(item->data(0, Qt::UserRole).value<MyMoneyAccount>().id());
      kmymoney->slotSelectInvestment(account);

    } catch (const MyMoneyException &) {
    }
  }
}

void KInvestmentView::slotInvestmentContextMenu(const QPoint& /*point*/)
{
  kmymoney->slotSelectInvestment();
  QTreeWidgetItem *item = m_investmentsList->currentItem();
  if (item) {
    kmymoney->slotSelectInvestment(MyMoneyFile::instance()->account(item->data(0, Qt::UserRole).value<MyMoneyAccount>().id()));
  }
  emit investmentRightMouseClick();
}

void KInvestmentView::slotLoadView(void)
{
  d->m_needReload[EquitiesTab] = true;
  d->m_needReload[SecuritiesTab] = true;
  if (isVisible())
    slotTabCurrentChanged(m_tab->currentIndex());
}

void KInvestmentView::slotTabCurrentChanged(int index)
{
  InvestmentsViewTab tab = static_cast<InvestmentsViewTab>(index);

  loadView(tab);
}

void KInvestmentView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if the current account still exists and make it the
  // current account
  if (!d->m_account.id().isEmpty()) {
    try {
      d->m_account = file->account(d->m_account.id());
    } catch (const MyMoneyException &) {
      d->m_account = MyMoneyAccount();
    }
  }

  d->m_filterProxyModel->invalidate();
  m_accountComboBox->expandAll();

  if (d->m_account.id().isEmpty()) {
    // there are no favorite accounts find any account
    QModelIndexList list = d->m_filterProxyModel->match(d->m_filterProxyModel->index(0, 0),
                           Qt::DisplayRole,
                           QVariant(QString("*")),
                           -1,
                           Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
    for (QModelIndexList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
      if (!it->parent().isValid())
        continue; // skip the top level accounts
      QVariant accountId = (*it).data(AccountsModel::AccountIdRole);
      if (accountId.isValid()) {
        MyMoneyAccount a = file->account(accountId.toString());
        if (a.value("PreferredAccount") == "Yes") {
          d->m_account = a;
          break;
        } else if (d->m_account.id().isEmpty()) {
          d->m_account = a;
        }
      }
    }
  }

  if (!d->m_account.id().isEmpty()) {
    m_accountComboBox->setSelected(d->m_account.id());
    try {
      d->m_precision = MyMoneyMoney::denomToPrec(d->m_account.fraction());
    } catch (const MyMoneyException &) {
      qDebug("Security %s for account %s not found", qPrintable(d->m_account.currencyId()), qPrintable(d->m_account.name()));
      d->m_precision = 2;
    }
  }
}


bool KInvestmentView::slotSelectAccount(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
    return false;

  if (d->m_recursion)
    return false;

  d->m_recursion = true;
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  bool rc = slotSelectAccount(acc.id());
  d->m_recursion = false;
  return rc;
}

bool KInvestmentView::slotSelectAccount(const QString& id, const QString& transactionId, const bool /* reconciliation*/)
{
  bool    rc = true;

  if (!id.isEmpty()) {
    // if the account id differs, then we have to do something
    if (d->m_account.id() != id) {
      try {
        d->m_account = MyMoneyFile::instance()->account(id);
        // if a stock account is selected, we show the
        // the corresponding parent (investment) account
        if (d->m_account.isInvest()) {
          d->m_account = MyMoneyFile::instance()->account(d->m_account.parentAccountId());
        }
        // TODO if we don't have an investment account, then we should switch to the ledger view
        d->m_newAccountLoaded = true;
        if (d->m_account.accountType() == MyMoneyAccount::Investment) {
          slotLoadView();
        } else {
          emit accountSelected(id, transactionId);
          d->m_account = MyMoneyAccount();
          d->m_needReload[EquitiesTab] = true;
          rc = false;
        }

      } catch (const MyMoneyException &) {
        qDebug("Unable to retrieve account %s", qPrintable(id));
        rc = false;
      }
    } else {
      emit accountSelected(d->m_account);
    }
  }

  return rc;
}

void KInvestmentView::clear(void)
{
  // setup header font
  QFont font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm(font);
  int height = fm.lineSpacing() + 6;
  m_investmentsList->header()->setMinimumHeight(height);
  m_investmentsList->header()->setMaximumHeight(height);
  m_investmentsList->header()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  m_investmentsList->setFont(font);

  // clear the table
  m_investmentsList->clear();

  // and the selected account in the combo box
  m_accountComboBox->setSelected(QString());

  // right align col headers for quantity, price and value
  for (int i = 2; i < 5; ++i) {
    m_investmentsList->headerItem()->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
  }
}

void KInvestmentView::loadInvestmentTab(void)
{
  // no account selected
  emit accountSelected(MyMoneyAccount());

  // clear the current contents ...
  clear();

  // ... load the combobox widget and select current account ...
  loadAccounts();

  if (d->m_account.id().isEmpty()) {
    // if we don't have an account we bail out
    setEnabled(false);
    return;
  }
  setEnabled(true);

  MyMoneyFile* file = MyMoneyFile::instance();
  bool showClosedAccounts = kmymoney->toggleAction("view_show_all_accounts")->isChecked()
                            || !KMyMoneyGlobalSettings::hideClosedAccounts();
  try {
    d->m_account = file->account(d->m_account.id());
    QStringList securities = d->m_account.accountList();

    for (QStringList::ConstIterator it = securities.constBegin(); it != securities.constEnd(); ++it) {
      MyMoneyAccount acc = file->account(*it);
      if (!acc.isClosed() || showClosedAccounts)
        loadInvestmentItem(acc);
    }
  } catch (const MyMoneyException &) {
    qDebug("KInvestmentView::loadView() - selected account does not exist anymore");
    d->m_account = MyMoneyAccount();
  }

  // and tell everyone what's selected
  emit accountSelected(d->m_account);
}

void KInvestmentView::loadInvestmentItem(const MyMoneyAccount& account)
{
  QTreeWidgetItem* item = new InvestmentItem(m_investmentsList);
  MyMoneySecurity security;
  MyMoneyFile* file = MyMoneyFile::instance();

  security = file->security(account.currencyId());
  MyMoneySecurity tradingCurrency = file->security(security.tradingCurrency());

  int prec = MyMoneyMoney::denomToPrec(tradingCurrency.smallestAccountFraction());

  //column 0 (COLUMN_NAME_INDEX) is the name of the stock
  item->setText(eInvestmentNameColumn, account.name());
  item->setData(eInvestmentNameColumn, Qt::UserRole, QVariant::fromValue(account));

  //column 1 (COLUMN_SYMBOL_INDEX) is the ticker symbol
  item->setText(eInvestmentSymbolColumn, security.tradingSymbol());

  //column 2 is the net value (price * quantity owned)
  const MyMoneyPrice &price = file->price(account.currencyId(), tradingCurrency.id());
  const MyMoneyMoney &balance = file->balance(account.id());
  if (price.isValid()) {
    const MyMoneyMoney &value = balance * price.rate(tradingCurrency.id());
    item->setText(eValueColumn, value.formatMoney(tradingCurrency.tradingSymbol(), prec));
    item->setData(eValueColumn, Qt::UserRole, QVariant::fromValue(value));
  } else {
    item->setText(eValueColumn, "---");
  }
  item->setTextAlignment(eValueColumn, Qt::AlignRight | Qt::AlignVCenter);

  //column 3 (COLUMN_QUANTITY_INDEX) is the quantity of shares owned
  prec = MyMoneyMoney::denomToPrec(security.smallestAccountFraction());

  item->setText(eQuantityColumn, balance.formatMoney("", prec));
  item->setTextAlignment(eQuantityColumn, Qt::AlignRight | Qt::AlignVCenter);
  item->setData(eQuantityColumn, Qt::UserRole, QVariant::fromValue(balance));

  //column 4 is the current price
  // Get the price precision from the configuration
  prec = KMyMoneyGlobalSettings::pricePrecision();

  // prec = MyMoneyMoney::denomToPrec(m_tradingCurrency.smallestAccountFraction());
  if (price.isValid()) {
    item->setText(ePriceColumn, price.rate(tradingCurrency.id()).formatMoney(tradingCurrency.tradingSymbol(), prec));
    item->setData(ePriceColumn, Qt::UserRole, QVariant::fromValue(price.rate(tradingCurrency.id())));
  } else {
    item->setText(ePriceColumn, "---");
  }
  item->setTextAlignment(ePriceColumn, Qt::AlignRight | Qt::AlignVCenter);
}

void KInvestmentView::showEvent(QShowEvent* event)
{
  emit aboutToShow();

  /*if (d->m_needReload) {
    loadInvestmentTab();
    d->m_needReload = false;
    d->m_newAccountLoaded = false;

  } else {
    emit accountSelected(d->m_account);
  }*/

  slotTabCurrentChanged(m_tab->currentIndex());

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInvestmentView::loadSecuritiesList(void)
{
  m_securitiesList->setColumnWidth(eIdColumn, 0);
  m_securitiesList->setSortingEnabled(false);
  m_securitiesList->clear();

  QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
  QList<MyMoneySecurity>::ConstIterator it;
  if (m_showCurrencyButton->isChecked()) {
    list += MyMoneyFile::instance()->currencyList();
  }
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(m_securitiesList);
    loadSecurityItem(newItem, *it);

  }
  m_securitiesList->setSortingEnabled(true);

  slotUpdateSecuritiesButtons();
}

void KInvestmentView::loadSecurityItem(QTreeWidgetItem* item, const MyMoneySecurity& security)
{
  QString market = security.tradingMarket();
  MyMoneySecurity tradingCurrency;
  if (security.isCurrency())
    market = m_currencyMarket;
  else
    tradingCurrency = MyMoneyFile::instance()->security(security.tradingCurrency());

  item->setText(eIdColumn, security.id());
  item->setText(eTypeColumn, KMyMoneyUtils::securityTypeToString(security.securityType()));
  item->setText(eSecurityNameColumn, security.name());
  item->setText(eSecuritySymbolColumn, security.tradingSymbol());
  item->setText(eMarketColumn, market);
  item->setText(eCurrencyColumn, tradingCurrency.tradingSymbol());
  item->setTextAlignment(eCurrencyColumn, Qt::AlignHCenter);
  item->setText(eAcctFractionColumn, QString::number(security.smallestAccountFraction()));

  // smallestCashFraction is only applicable for currencies
  if (security.isCurrency())
    item->setText(eCashFractionColumn, QString::number(security.smallestCashFraction()));
}

void KInvestmentView::slotUpdateSecuritiesButtons(void)
{
  QTreeWidgetItem* item = m_securitiesList->currentItem();

  if (item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());
    m_editSecurityButton->setEnabled(item->text(eMarketColumn) != m_currencyMarket);
    m_deleteSecurityButton->setEnabled(!MyMoneyFile::instance()->isReferenced(security));

  } else {
    m_editSecurityButton->setEnabled(false);
    m_deleteSecurityButton->setEnabled(false);
  }
}

void KInvestmentView::slotEditSecurity(void)
{
  QTreeWidgetItem* item = m_securitiesList->currentItem();
  if (item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());

    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(security, this);
    dlg->setObjectName("KNewInvestmentWizard");
    if (dlg->exec() == QDialog::Accepted) {
      dlg->createObjects(QString());
      try {
        // For some reason, the item gets deselected, and the pointer
        // invalidated. So fix it here before continuing.
        item = m_securitiesList->findItems(security.id(), Qt::MatchExactly).at(0);
        m_securitiesList->setCurrentItem(item);
        if (item) {
          security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());
          loadSecurityItem(item, security);
        }
      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("Failed to edit security: %1", e.what()));
      }
    }
    delete dlg;
  }
}

void KInvestmentView::slotDeleteSecurity(void)
{
  QTreeWidgetItem* item = m_securitiesList->currentItem();
  if (item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());
    QString msg;
    QString dontAsk;
    if (security.isCurrency()) {
      msg = i18n("<p>Do you really want to remove the currency <b>%1</b> from the file?</p><p><i>Note: adding currencies is not currently supported.</i></p>", security.name());
      dontAsk = "DeleteCurrency";
    } else {
      msg = i18n("<p>Do you really want to remove the %1 <b>%2</b> from the file?</p>", KMyMoneyUtils::securityTypeToString(security.securityType()), security.name());
      dontAsk = "DeleteSecurity";
    }
    if (KMessageBox::questionYesNo(this, msg, i18n("Delete security"), KStandardGuiItem::yes(), KStandardGuiItem::no(), dontAsk) == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        if (security.isCurrency())
          MyMoneyFile::instance()->removeCurrency(security);
        else
          MyMoneyFile::instance()->removeSecurity(security);
        ft.commit();
      } catch (const MyMoneyException &) {
      }
    }
  }
}
