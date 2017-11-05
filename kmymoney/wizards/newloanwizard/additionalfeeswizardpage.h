/***************************************************************************
                         konlineupdatewizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADDITIONALFEESWIZARDPAGE_H
#define ADDITIONALFEESWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class AdditionalFeesWizardPage; }

class MyMoneyAccount;

/**
 * This class implements the Additional Fees page of the
 * @ref KNewLoanWizard.
 */

class AdditionalFeesWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit AdditionalFeesWizardPage(QWidget *parent = nullptr);
  ~AdditionalFeesWizardPage();

  void updatePeriodicPayment(const MyMoneyAccount& account);

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

protected slots:
  virtual void slotAdditionalFees();

private:
  Ui::AdditionalFeesWizardPage *ui;
};

#endif
