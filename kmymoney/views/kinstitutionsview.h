/***************************************************************************
                             kinstitutionssview.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyinstitution.h>
#include <mymoneyutils.h>

#include "ui_kinstitutionsviewdecl.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements the institutions hierarchical 'view'.
  */

class KMyMoneyApp;

class KInstitutionsView : public QWidget, private Ui::KInstitutionsViewDecl
{
  Q_OBJECT

public:
  KInstitutionsView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview);
  virtual ~KInstitutionsView();

  KRecursiveFilterProxyModel    *getProxyModel();
  QList<AccountsModel::Columns> *getProxyColumns();
  void                          setDefaultFocus();
  bool                          isLoaded();

public slots:
  void slotLoadAccounts();

protected:
  void loadAccounts();

  // for now it contains the implementation from show()
  virtual void showEvent(QShowEvent * event);

protected slots:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotExpandCollapse();

private:
  /**
    * This method returns an icon according to the account type
    * passed in the argument @p type.
    *
    * @param type account type as defined in MyMoneyAccount::accountTypeE
    */
  const QPixmap accountImage(const MyMoneyAccount::accountTypeE type) const;

  /** Initializes page and sets its load status to initialized
   */
  void init();

private:
  KMyMoneyApp                         *m_kmymoney;
  KMyMoneyView                        *m_kmymoneyview;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  /**
    * This member holds the load state of page
    */
  bool                                m_needLoad;

  AccountsViewFilterProxyModel        *m_filterProxyModel;
};

#endif
