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

class KInstitutionsView : public QWidget, private Ui::KInstitutionsViewDecl
{
  Q_OBJECT

public:
  KInstitutionsView(QWidget *parent = 0);
  virtual ~KInstitutionsView();

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

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTreeView::selectObject()
    */
  void selectObject(const MyMoneyObject&);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTreeView::openContextMenu(const MyMoneyObject&)
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an account
    * or institution.
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  /**
    * This member holds the load state of page
    */
  bool                                m_needLoad;

  AccountsViewFilterProxyModel        *m_filterProxyModel;
};

#endif
