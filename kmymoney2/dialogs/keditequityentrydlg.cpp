/***************************************************************************
                          keditequityentrydlg.cpp  -  description
                             -------------------
    begin                : Sat Mar 6 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "keditequityentrydlg.h"
#include "kupdatestockpricedlg.h"
#include "../widgets/kmymoneypriceview.h"

KEditEquityEntryDlg::KEditEquityEntryDlg(const MyMoneySecurity& selectedSecurity, QWidget *parent, const char *name)
  : KEditEquityEntryDecl(parent, name, true)
{
  m_selectedSecurity = selectedSecurity;

  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(edtEquityName, SIGNAL(textChanged(const QString &)), this, SLOT(slotDataChanged()));
  connect(edtMarketSymbol, SIGNAL(textChanged(const QString &)), this, SLOT(slotDataChanged()));
  connect(edtFraction, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()));
  connect(btnAddEntry, SIGNAL(clicked()), kpvPriceHistory, SLOT(slotAddPrice()));
  connect(btnEditEntry, SIGNAL(clicked()), kpvPriceHistory, SLOT(slotEditPrice()));
  connect(btnRemoveEntry, SIGNAL(clicked()), kpvPriceHistory, SLOT(slotDeletePrice()));
  connect(kpvPriceHistory, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectionChanged(Q3ListViewItem*)));

  //fill in the fields with what we know.
  edtEquityName->setText(m_selectedSecurity.name());
  edtMarketSymbol->setText(m_selectedSecurity.tradingSymbol());
  edtFraction->setPrecision(0);
  edtFraction->setCalculatorButtonVisible(false);
  edtFraction->loadText(QString::number(m_selectedSecurity.smallestAccountFraction()));
  cmbInvestmentType->setCurrentItem((int)m_selectedSecurity.securityType());
  // FIXME PRICE
  // kpvPriceHistory->setHistory(m_selectedSecurity.priceHistory());

  // add icons to buttons
  KIconLoader *il = KIconLoader::global();
  btnOK->setGuiItem(KStandardGuiItem::ok());
  btnCancel->setGuiItem(KStandardGuiItem::cancel());
  btnRemoveEntry->setGuiItem(KStandardGuiItem::remove());
  btnAddEntry->setGuiItem(KStandardGuiItem::add());

  KGuiItem editButtenItem( i18n( "&Edit" ),
                    QIcon(il->loadIcon("edit", KIcon::Small, KIconLoader::SizeSmall)),
                    i18n("Modify the selected entry"),
                    i18n("Change the price information of the selected entry."));
  btnEditEntry->setGuiItem(editButtenItem);

  slotSelectionChanged(0);      // make sure buttons are disabled in the beginning
  slotDataChanged();
  m_changes = false;

  // force a resize to optimize the layout of all widgets
  resize(width()-1, height()-1);
  QTimer::singleShot(10, this, SLOT(slotTimerDone()));
}

KEditEquityEntryDlg::~KEditEquityEntryDlg()
{
}

void KEditEquityEntryDlg::slotTimerDone(void)
{
  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width()+1, height()+1);
}

/** No descriptions */
void KEditEquityEntryDlg::slotOKClicked()
{
  if(m_changes /* || kpvPriceHistory->dirty() */)
  {
    m_selectedSecurity.setName(edtEquityName->text());
    m_selectedSecurity.setTradingSymbol(edtMarketSymbol->text());
    m_selectedSecurity.setSmallestAccountFraction(edtFraction->value().abs());
    // FIXME PRICE
    // m_selectedSecurity.setPriceHistory(kpvPriceHistory->history());
  }

  accept();
}

void KEditEquityEntryDlg::slotSelectionChanged(Q3ListViewItem* item)
{
  btnEditEntry->setEnabled(item != 0);
  btnRemoveEntry->setEnabled(item != 0);
}

void KEditEquityEntryDlg::slotDataChanged(void)
{
  bool okEnabled = true;

  if(!edtFraction->value().isPositive()
  || edtMarketSymbol->text().isEmpty()
  || edtEquityName->text().isEmpty())
    okEnabled = false;

  btnOK->setEnabled(okEnabled);

  m_changes = true;
}

#include "keditequityentrydlg.moc"
