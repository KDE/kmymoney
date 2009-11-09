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

#include "ksecuritylisteditor.h"

#include "mymoneysecurity.h"
#include "mymoneyfile.h"
#include "knewinvestmentwizard.h"

#include "kmymoneyutils.h"

#define ID_COL        0
#define TYPE_COL      1
#define NAME_COL      2
#define SYMBOL_COL    3
#define MARKET_COL    4
#define CURR_COL      5
#define ACCFRACT_COL  6
#define CASHFRACT_COL 7

#define CURRENCY_MARKET    QString("ISO 4217")

KSecurityListEditor::KSecurityListEditor(QWidget *parent) :
  KSecurityListEditorDecl(parent)
{
  m_listView->setColumnWidth(ID_COL, 0);
  m_listView->setColumnWidthMode(NAME_COL, Q3ListView::Maximum);
  m_listView->setColumnWidthMode(ID_COL, Q3ListView::Manual);
  m_listView->setColumnAlignment(CURR_COL, Qt::AlignHCenter);
  m_listView->setMultiSelection(false);
  m_listView->setAllColumnsShowFocus(true);

  KGuiItem removeButtenItem( i18n( "&Delete" ),
                    KIcon("edit-delete"),
                    i18n("Delete this entry"),
                    i18n("Remove this security item from the file"));
  m_deleteButton->setGuiItem(removeButtenItem);

  KGuiItem addButtenItem( i18n( "&Add" ),
                    KIcon("file_new"),
                    i18n("Add a new entry"),
                    i18n("Create a new security entry."));
  m_addButton->setGuiItem(addButtenItem);

  KGuiItem editButtenItem( i18n( "&Edit" ),
                    KIcon("document-edit"),
                    i18n("Modify the selected entry"),
                    i18n("Change the security information of the selected entry."));
  m_editButton->setGuiItem(editButtenItem);

  KGuiItem okButtenItem( i18n("&Close" ),
                    KIcon("dialog-ok"),
                    i18n("Close the dialog"),
                    i18n("Use this to close the dialog and return to the application."));
  m_closeButton->setGuiItem(okButtenItem);

  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_showCurrencyButton, SIGNAL(toggled(bool)), this, SLOT(slotLoadList()));
  connect(m_listView, SIGNAL(selectionChanged()), this, SLOT(slotUpdateButtons()));

  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditSecurity()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteSecurity()));

  // FIXME for now, the only way to add a new security is to add a new investment
  m_addButton->hide();

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
  if(m_showCurrencyButton->isChecked()) {
    list += MyMoneyFile::instance()->currencyList();
  }
  for(it = list.begin(); it != list.end(); ++it) {
    K3ListViewItem* newItem = new K3ListViewItem(m_listView, QString((*it).id()));
    fillItem(newItem, *it);

  }
  slotUpdateButtons();
}

void KSecurityListEditor::fillItem(Q3ListViewItem* item, const MyMoneySecurity& security)
{
  QString market = security.tradingMarket();
  MyMoneySecurity tradingCurrency;
  if(security.isCurrency())
    market = CURRENCY_MARKET;
  else
    tradingCurrency = MyMoneyFile::instance()->security(security.tradingCurrency());

  item->setText(TYPE_COL, KMyMoneyUtils::securityTypeToString(security.securityType()));
  item->setText(NAME_COL, security.name());
  item->setText(SYMBOL_COL, security.tradingSymbol());
  item->setText(MARKET_COL, market);
  item->setText(CURR_COL, tradingCurrency.tradingSymbol());
  item->setText(ACCFRACT_COL, QString::number(security.smallestAccountFraction()));

  // smallestCashFraction is only applicable for currencies
  if(security.isCurrency())
    item->setText(CASHFRACT_COL, QString::number(security.smallestCashFraction()));
}

void KSecurityListEditor::slotUpdateButtons(void)
{
  Q3ListViewItem* item = m_listView->selectedItem();

  if(item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(ID_COL).toLatin1());
    m_editButton->setEnabled(item->text(MARKET_COL) != CURRENCY_MARKET);
    m_deleteButton->setEnabled(!MyMoneyFile::instance()->isReferenced(security));

  } else {
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
  }
}

void KSecurityListEditor::slotEditSecurity(void)
{
  Q3ListViewItem* item = m_listView->selectedItem();
  if(item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(ID_COL).toLatin1());

    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(security, this );
    dlg->setObjectName("KNewInvestmentWizard");
    if(dlg->exec() == QDialog::Accepted) {
      dlg->createObjects(QString());
      try {
        security = MyMoneyFile::instance()->security(item->text(ID_COL).toLatin1());
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
  Q3ListViewItem* item = m_listView->selectedItem();
  if(item) {
    MyMoneySecurity security = MyMoneyFile::instance()->security(item->text(ID_COL).toLatin1());
    QString msg;
    QString dontAsk;
    if(security.isCurrency()) {
      msg = QString("<p>") + i18n("Do you really want to remove the currency <b>%1</b> from the file?</p><p><i>Note: It is currently not supported to add currencies.</i></p>",security.name());
      dontAsk = "DeleteCurrency";
    } else {
      msg = QString("<p>") + i18n("Do you really want to remove the %1 <b>%2</b> from the file?</p>",KMyMoneyUtils::securityTypeToString(security.securityType()),security.name());
      dontAsk = "DeleteSecurity";
    }
    if(KMessageBox::questionYesNo(this, msg, i18n("Delete security"), KStandardGuiItem::yes(), KStandardGuiItem::no(), dontAsk) == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        if(security.isCurrency())
          MyMoneyFile::instance()->removeCurrency(security);
        else
          MyMoneyFile::instance()->removeSecurity(security);
        ft.commit();
        slotLoadList();
      } catch(MyMoneyException *e) {
        delete e;
      }
    }
  }
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef ID_COL
#undef TYPE_COL
#undef NAME_COL
#undef SYMBOL_COL
#undef MARKET_COL
#undef CURR_COL
#undef ACCFRACT_COL
#undef CASHFRACT_COL

#include "ksecuritylisteditor.moc"
