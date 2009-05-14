/***************************************************************************
                         kmymoneyaccounttree.h  -  description
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

#ifndef KMYMONEYACCOUNTTREE_H
#define KMYMONEYACCOUNTTREE_H


#include <kmymoneyaccounttreebase.h>
//Added by qt3to4:
#include <Q3ValueList>

class KMyMoneyAccountTreeItem;

class KMyMoneyAccountTree : public KMyMoneyAccountTreeBase
{
  Q_OBJECT
public:
  KMyMoneyAccountTree(QWidget* parent = 0, const char *name = 0);
  int taxReportColumn(void) const { return m_taxReportColumn; }
  int vatCategoryColumn(void) const { return m_vatCategoryColumn; }
private:
  int m_taxReportColumn;
  int m_vatCategoryColumn;
};

class KMyMoneyAccountTreeItem : public KMyMoneyAccountTreeBaseItem
{
public:
  /**
 * Constructor to be used to construct an institution entry
 * object.
 *
 * @param parent pointer to the K3ListView object this entry should be
 *               added to.
 * @param institution const reference to MyMoneyInstitution for which
 *               the K3ListView entry is constructed
   */
  KMyMoneyAccountTreeItem(K3ListView *parent, const MyMoneyInstitution& institution);

  /**
   * Constructor to be used to construct a standard account entry object (e.g. Asset,
   * Liability, etc.).
   *
   * @param parent pointer to the K3ListView object this entry should be
   *               added to.
   * @param account const reference to MyMoneyAccount for which
   *               the K3ListView entry is constructed
   * @param security const reference to the security used to show the value. Usually
   *                 one should pass MyMoneyFile::baseCurrency() here.
   * @param name name of the account to be used instead of the one stored with @p account
   *               If empty, the one stored with @p account will be used. Default: empty
   */
  KMyMoneyAccountTreeItem(K3ListView *parent, const MyMoneyAccount& account, const MyMoneySecurity& security = MyMoneySecurity(), const QString& name = QString());

  /**
   * Constructor to be used to construct an account entry
   * object.
   *
   * @param parent pointer to the parent KAccountListView object this entry should be
   *               added to.
   * @param account const reference to MyMoneyAccount for which
   *               the K3ListView entry is constructed
   * @param price price to be used to calculate value (defaults to 1)
   *              This is used for accounts denominated in foreign currencies or stocks
   * @param security const reference to the security used to show the value. Usually
   *                 one should pass MyMoneyFile::baseCurrency() here.
   */
  KMyMoneyAccountTreeItem(KMyMoneyAccountTreeBaseItem *parent, const MyMoneyAccount& account, const Q3ValueList<MyMoneyPrice>& price = Q3ValueList<MyMoneyPrice>(), const MyMoneySecurity& security = MyMoneySecurity());

  void setReconciliation(bool);


protected:
  /**
    * Returns the current balance of this account.
    *
    * This is a pure virtual function, to allow subclasses to calculate
    * the balance in different ways.
    *
    * Parent items in the tree will only be recomputed if the balance() for
    * a son changes.
    * @param account Account to get the balance for
    * @return Balance of this account
    */
    MyMoneyMoney balance() const;

    bool m_reconcileFlag;

    /**
     * populates the columns. Derived classes should override this. The
     * name column is already filled and should not be changed.
     */
    void fillColumns();
};

#endif

