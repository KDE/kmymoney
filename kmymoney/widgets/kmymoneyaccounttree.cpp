/***************************************************************************
                         kmymoneyaccounttree.cpp  -  description
                            -------------------
   begin                : Sat Jan 1 2005
   copyright            : (C) 2005 by Thomas Baumgart
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

#include "kmymoneyaccounttree.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPoint>
#include <QEvent>
#include <QTimer>
#include <QCursor>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyglobalsettings.h>
#include <kmymoneyutils.h>

KMyMoneyAccountTree::KMyMoneyAccountTree(QWidget* parent) :
    KMyMoneyAccountTreeBase(parent)
{
  showType();

  m_taxReportColumn = addColumn(i18nc("Column heading for category in tax report", "Tax"));
  setColumnWidthMode(m_taxReportColumn, Q3ListView::Manual);
  setColumnAlignment(m_taxReportColumn, Qt::AlignHCenter);

  m_vatCategoryColumn = addColumn(i18nc("Column heading for VAT category", "VAT"));
  setColumnWidthMode(m_vatCategoryColumn, Q3ListView::Manual);
  setColumnAlignment(m_vatCategoryColumn, Qt::AlignHCenter);

  showValue();
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(K3ListView *parent, const MyMoneyAccount& account, const MyMoneySecurity& security , const QString& name) :
    KMyMoneyAccountTreeBaseItem(parent, account, security, name),
    m_reconcileFlag(false)
{
  updateAccount();
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(KMyMoneyAccountTreeBaseItem *parent, const MyMoneyAccount& account, const QList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
    KMyMoneyAccountTreeBaseItem(parent, account, price, security),
    m_reconcileFlag(false)
{
  updateAccount();
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(K3ListView *parent, const MyMoneyInstitution& institution) :
    KMyMoneyAccountTreeBaseItem(parent, institution),
    m_reconcileFlag(false)
{
}

void KMyMoneyAccountTreeItem::fillColumns()
{
  KMyMoneyAccountTree* lv = dynamic_cast<KMyMoneyAccountTree*>(listView());
  if (!lv)
    return;
  KMyMoneyAccountTreeBaseItem::fillColumns();
  QPixmap checkMark = QPixmap(KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::Small));
  MyMoneyMoney vatRate;
  if (!isInstitution())
    setPixmap(lv->nameColumn(), m_account.accountPixmap(m_reconcileFlag, 22));
  switch (m_account.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      if (m_account.value("Tax").toLower() == "yes")
        setPixmap(lv->taxReportColumn(), checkMark);
      if (!m_account.value("VatAccount").isEmpty()) {
        setPixmap(lv->vatCategoryColumn(), checkMark);
      }
      if (!m_account.value("VatRate").isEmpty()) {
        vatRate = MyMoneyMoney(m_account.value("VatRate")) * MyMoneyMoney(100, 1);
        setText(lv->vatCategoryColumn(), QString("%1 %").arg(vatRate.formatMoney("", 1)));
      }
      break;
    default:
      break;
  }
}

void KMyMoneyAccountTreeItem::setReconciliation(bool on)
{
  if (m_reconcileFlag == on)
    return;
  m_reconcileFlag = on;
  updateAccount();
}

MyMoneyMoney KMyMoneyAccountTreeItem::balance() const
{
  MyMoneyMoney result;
  // account.balance() is not compatable with stock accounts
  if (m_account.isInvest())
    result = MyMoneyFile::instance()->balance(m_account.id());
  else
    result = m_account.balance();
  // for income and liability accounts, we reverse the sign
  switch (m_account.accountGroup()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      result = -result;
      break;

    default:
      break;
  }
  return result;
}


#include "kmymoneyaccounttree.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
