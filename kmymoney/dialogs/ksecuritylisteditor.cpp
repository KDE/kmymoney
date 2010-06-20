/***************************************************************************
                          ksecuritylisteditor.cpp  -  description
                             -------------------
    begin                : Wed Dec 16 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ksecuritylisteditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <k3listview.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysecurity.h"
#include "mymoneyfile.h"
#include "knewinvestmentwizard.h"
#include "kmymoneyutils.h"

KSecurityListEditor::KSecurityListEditor(QWidget *parent) :
    KSecurityListEditorDecl(parent),
    m_currencyMarket("ISO 4217")
{
  m_listView->setColumnWidth(eIdColumn, 0);
  m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_listView->setAllColumnsShowFocus(true);

  setButtons(KDialog::Ok);
  setButtonsOrientation(Qt::Horizontal);
  setMainWidget(m_layoutWidget);

  KGuiItem removeButtonItem(i18n("&Delete"),
                            KIcon("edit-delete"),
                            i18n("Delete this entry"),
                            i18n("Remove this security item from the file"));
  m_deleteButton->setGuiItem(removeButtonItem);

  KGuiItem editButtonItem(i18n("&Edit"),
                          KIcon("document-edit"),
                          i18n("Modify the selected entry"),
                          i18n("Change the security information of the selected entry."));
  m_editButton->setGuiItem(editButtonItem);

  connect(m_showCurrencyButton, SIGNAL(toggled(bool)), this, SLOT(slotLoadList()));
  connect(m_listView, SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateButtons()));

  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditSecurity()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteSecurity()));

  slotLoadList();
}

KSecurityListEditor::~KSecurityListEditor()
{
}

void KSecurityListEditor::slotLoadList(void)
{
  m_listView->clear();

  QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
  QList<MyMoneySecurity>::ConstIterator it;
  if (m_showCurrencyButton->isChecked()) {
    list += MyMoneyFile::instance()->currencyList();
  }
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(m_listView);
    fillItem(newItem, *it);

  }
  slotUpdateButtons();
}

void KSecurityListEditor::fillItem(QTreeWidgetItem* item, const MyMoneySecurity& security)
{
  QString market = security.tradingMarket();
  MyMoneySecurity tradingCurrency;
  if (security.isCurrency())
    market = m_currencyMarket;
  else
    tradingCurrency = MyMoneyFile::instance()->security(security.tradingCurrency());

  item->setText(eIdColumn, security.id());
  item->setText(eTypeColumn, KMyMoneyUtils::securityTypeToString(security.securityType()));
  item->setText(eNameColumn, security.name());
  item->setText(eSymbolColumn, security.tradingSymbol());
  item->setText(eMarketColumn, market);
  item->setText(eCurrencyColumn, tradingCurrency.tradingSymbol());
  item->setTextAlignment(eCurrencyColumn, Qt::AlignHCenter);
  item->setText(eAcctFractionColumn, QString::number(security.smallestAccountFraction()));

  // smallestCashFraction is only applicable for currencies
  if (security.isCurrency())
    item->setText(eCashFractionColumn, QString::number(security.smallestCashFraction()));
}

void KSecurityListEditor::slotUpdateButtons(void)
{
  QTreeWidgetItem* item = m_listView->currentItem();

  if (item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());
    m_editButton->setEnabled(item->text(eMarketColumn) != m_currencyMarket);
    m_deleteButton->setEnabled(!MyMoneyFile::instance()->isReferenced(security));

  } else {
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
  }
}

void KSecurityListEditor::slotEditSecurity(void)
{
  QTreeWidgetItem* item = m_listView->currentItem();
  if (item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());

    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(security, this);
    dlg->setObjectName("KNewInvestmentWizard");
    if (dlg->exec() == QDialog::Accepted) {
      dlg->createObjects(QString());
      try {
        security = MyMoneyFile::instance()->security(item->text(eIdColumn).toLatin1());
        fillItem(item, security);
      } catch (MyMoneyException* e) {
        KMessageBox::error(this, i18n("Failed to edit security: %1", e->what()));
        delete e;
      }
    }
    delete dlg;
  }
}

void KSecurityListEditor::slotDeleteSecurity(void)
{
  QTreeWidgetItem* item = m_listView->currentItem();
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
        slotLoadList();
      } catch (MyMoneyException *e) {
        delete e;
      }
    }
  }
}

#include "ksecuritylisteditor.moc"
