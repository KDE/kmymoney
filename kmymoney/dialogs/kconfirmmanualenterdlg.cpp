/***************************************************************************
                          kconfirmmanualenterdlg.cpp
                             -------------------
    begin                : Mon Apr  9 2007
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

#include "kconfirmmanualenterdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyutils.h>

KConfirmManualEnterDlg::KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent) :
  KConfirmManualEnterDlgDecl(parent)
{
  buttonGroup1->setId(m_discardRadio, 0);
  buttonGroup1->setId(m_onceRadio, 1);
  buttonGroup1->setId(m_setRadio, 2);

  buttonOk->setGuiItem(KStandardGuiItem::ok());
  buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  m_onceRadio->setChecked(true);

  if(schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    m_setRadio->setEnabled(false);
    m_discardRadio->setEnabled(false);
  }
}

void KConfirmManualEnterDlg::loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn)
{
  QString messageDetail("<qt>");
  MyMoneyFile* file = MyMoneyFile::instance();
  int noItemsChanged=0;

  try
  {
    if (to.splits().isEmpty())
      throw new MYMONEYEXCEPTION(i18n("Transaction %1 has no splits").arg(to.id()));
    if (tn.splits().isEmpty())
      throw new MYMONEYEXCEPTION(i18n("Transaction %1 has no splits").arg(tn.id()));

    QString po, pn;
    if(!to.splits().front().payeeId().isEmpty())
      po = file->payee(to.splits().front().payeeId()).name();
    if(!tn.splits().front().payeeId().isEmpty())
      pn = file->payee(tn.splits().front().payeeId()).name();

    if (po != pn) {
      noItemsChanged++;
      messageDetail += i18n("Payee changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>", po, pn);
    }

    if(to.splits().front().accountId() != tn.splits().front().accountId()) {
      noItemsChanged++;
      messageDetail += i18n("Account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>"
        , file->account(to.splits().front().accountId()).name()
	, file->account(tn.splits().front().accountId()).name());
    }

    if(file->isTransfer(to) && file->isTransfer(tn)) {
      if(to.splits()[1].accountId() != tn.splits()[1].accountId()) {
        noItemsChanged++;
        messageDetail += i18n("Transfer account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>"
	  , file->account(to.splits()[1].accountId()).name()
	  , file->account(tn.splits()[1].accountId()).name());
      }
    } else {
      QString co, cn;
      switch(to.splitCount()) {
        default:
          co = i18nc("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          co = file->accountToCategory(to.splits()[1].accountId());
        case 1:
          break;
      }

      switch(tn.splitCount()) {
        default:
          cn = i18nc("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          cn = file->accountToCategory(tn.splits()[1].accountId());
        case 1:
          break;
      }
      if (co != cn)
      {
        noItemsChanged++;
        messageDetail += i18n("Category changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>", co, cn);
      }
    }

    QString mo, mn;
    mo = to.splits().front().memo();
    mn = tn.splits().front().memo();
    if(mo.isEmpty())
       mo = QString("<i>")+i18nc("Empty memo", "empty")+QString("</i>");
    if(mn.isEmpty())
       mn = QString("<i>")+i18nc("Empty memo", "empty")+QString("</i>");
    if (mo != mn)
    {
      noItemsChanged++;
      messageDetail += i18n("Memo changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>", mo, mn);
    }

    const MyMoneySecurity& sec = MyMoneyFile::instance()->security(to.commodity());
    MyMoneyMoney ao, an;
    ao = to.splits().front().value();
    an = tn.splits().front().value();
    if (ao != an) {
      noItemsChanged++;
      messageDetail += i18n("Amount changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>", ao.formatMoney(sec.smallestAccountFraction()), an.formatMoney(sec.smallestAccountFraction()));
    }

    MyMoneySplit::reconcileFlagE fo, fn;
    fo = to.splits().front().reconcileFlag();
    fn = tn.splits().front().reconcileFlag();
    if(fo != fn) {
      noItemsChanged++;
      messageDetail += i18n("Reconciliation flag changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>",    KMyMoneyUtils::reconcileStateToString(fo, true), KMyMoneyUtils::reconcileStateToString(fn, true));
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::error(this, i18n("Fatal error in determining data: ") + e->what());
    delete e;
  }

  messageDetail += "</qt>";
  m_details->setText(messageDetail);
  return;
}

KConfirmManualEnterDlg::Action KConfirmManualEnterDlg::action(void) const
{
  if(m_discardRadio->isChecked())
    return UseOriginal;
  if(m_setRadio->isChecked())
    return ModifyAlways;
  return ModifyOnce;
}

#include "kconfirmmanualenterdlg.moc"
