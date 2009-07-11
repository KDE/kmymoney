/***************************************************************************
                             kinstitutionssview.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef KINSTITUTIONSVIEW_H
#define KINSTITUTIONSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyinstitution.h>
#include <kmymoneyaccounttree.h>
#include <mymoneyutils.h>

#include "ui_kinstitutionsviewdecl.h"
//Added by qt3to4:
#include <QPixmap>

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements the institutions hierarchical 'view'.
  */

class KInstitutionsViewDecl : public QWidget, public Ui::KInstitutionsViewDecl
{
public:
  KInstitutionsViewDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};
class KInstitutionsView : public KInstitutionsViewDecl
{
  Q_OBJECT
private:

public:
  KInstitutionsView(QWidget *parent=0);
  virtual ~KInstitutionsView();

public slots:
  void slotLoadAccounts(void);

  void slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

protected:
  void loadAccounts(void);

  // load accounts that are kept at a specific institution
  void loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QString& institutionId);

  // load stock accounts under the investment account (parent)
  void loadSubAccounts(KMyMoneyAccountTreeItem* parent);

  // for now it contains the implementation from show()
  virtual void showEvent(QShowEvent * event);

protected slots:
  void slotUpdateNetWorth(void);

private:
  /**
    * This method returns an icon according to the account type
    * passed in the argument @p type.
    *
    * @param type account type as defined in MyMoneyAccount::accountTypeE
    */
  const QPixmap accountImage(const MyMoneyAccount::accountTypeE type) const;

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    */
  void selectObject(const MyMoneyObject&);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an account
    * or institution.
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p institution.
    *
    * @param acc const reference to account to be reparented
    * @param institution const reference to new institution
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyInstitution& institution);

private:
  MyMoneyAccount                      m_reconciliationAccount;
  QMap<QString, MyMoneyAccount>       m_accountMap;
  QMap<QString, MyMoneySecurity>      m_securityMap;
  QMap<QString, unsigned long>        m_transactionCountMap;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;
};

#endif
