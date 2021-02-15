/*
    SPDX-FileCopyrightText: 2006 Thomas Baumagrt <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYWIZARD_H
#define KMYMONEYWIZARD_H

#include "kmm_wizard_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyWizardPage;

template <class T> class QList;

/**
  * @author Thomas Baumgart (C) 2006
  *
  * This is a base class for implementation of the KMyMoneyWizard. It provides
  * the following layout of a wizard:
  *
  * @code
  * +-wizardLayout-----------------------------------------------+
  * |                                                            |
  * +------------------------------------------------------------+
  * |+-stepLayout--++-------------------------------------------+|
  * ||             ||+-pageLayout------------------------------+||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             ||+-----------------------------------------+||
  * ||             |||+-buttonLayout--------------------------+|||
  * ||             ||||                                       ||||
  * ||             |||+---------------------------------------+|||
  * ||             ||+-----------------------------------------+||
  * |+-------------++-------------------------------------------+|
  * +------------------------------------------------------------+
  * @endcode
  *
  * The top bar is filled with a KMyMoneyTitleLabel as known from
  * KMyMoney's views. To the left there is an area in the same color
  * as the title bar showing the steps for this wizard. Each such step
  * can consist of one or more wizard pages. At the bottom of this area
  * the text "Step x of y" is shown and updated. To the right of this
  * part, the actual wizard page is shown. At the bottom of the page
  * the class inserts a standard button widget consisting of a Help,
  * Back, Next/Finish and Cancel button.
  *
  * The wizard serves as container for the wizard pages. In order to access
  * the data filled into the pages, one would have to provide getter methods.
  *
  * Here is an example how this object could be used. Please also see the
  * example described with the KMyMoneyWizardPage class.
  *
  * @code
  *
  * class KNewUserGeneral;
  * class KNewUserPersonal;
  *
  * class KNewUserWizard : public KMyMoneyWizard
  * {
  *   Q_OBJECT
  * public:
  *   KNewUserWizard(QWidget* parent = nullptr, const char* name = 0, bool modal = false, Qt::WindowFlags flags = 0);
  *
  * private:
  *   KNewUserGeneral*  m_generalPage;
  *   KNewUserPersonal* m_personalPage;
  *   KNewUserFinal*    m_finalPage;
  *   // add more pages here
  *
  *   friend class KNewUserGeneral;
  *   friend class KNewUserPersonal;
  *   friend class KNewUserFinal;
  *   // add more pages here
  * };
  * @endcode
  *
  * The implementation is also easy and looks like this:
  *
  * @code
  * KNewUserWizard::KNewUserWizard(QWidget* parent, const char* name, bool modal, Qt::WindowFlags flags) :
  *   KMyMoneyWizard(parent, name, modal, flags)
  * {
  *   setTitle("KMyMoney New User Setup");
  *   addStep("General Data");
  *   addStep("Personal Data");
  *   addStep("Finish");
  *
  *   m_generalPage = new KNewUserGeneral(this);
  *   m_personalPage = new KNewUserPersonal(this);
  *   m_finalPage = new KNewUserFinal(this);
  *
  *   setFirstPage(m_testPage1);
  * }
  * @endcode
  *
  * Don't forget to call setFirstPage() to get things started.
  *
  * The code to use this whole structure would then look something like this:
  *
  * @code
  *     KNewUserWizard* wizard = new KNewUserWizard(this, "NewUserWizard");
  *     int rc = wizard->exec();
  * @endcode
  *
  * The return code of exec() is either @p QDialog::Accepted or
  * @p QDialog::Rejected.
  *
  * @note The implementation of this class is heavily based on ideas found at
  *       https://doc.qt.io/qt-5/qtwidgets-dialogs-licensewizard-example.html
  */
class KMyMoneyWizardPrivate;
class KMM_WIZARD_EXPORT KMyMoneyWizard : public QDialog
{
  friend class KMyMoneyWizardPage;

  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyWizard)

public:
  /**
    * Modify the title of the wizard to be @p txt.
    *
    * @param txt The text that should be used as title
    */
  void setTitle(const QString& txt);

  /**
    * Add step @p text to the wizard
    *
    * @param text Text to be shown for this step
    */
  void addStep(const QString& text);

  QList<KMyMoneyWizardPage*> historyPages() const;

  /**
    * This method repeats selection of the current step in the
    * step frame.
    * This is used to allow changes made to showing and hiding
    * pages to immediately to be reflected in the step frame
    */
  void reselectStep();

  /**
   * Setup a global help context for the wizard. It will be used whenever
   * there is no specific help context available for the current page.
   *
   * @sa KMyMoneyWizardPage::helpContext()
   */
  void setHelpContext(const QString& ctx);

  virtual ~KMyMoneyWizard();

Q_SIGNALS:
//  /**
//    * This signal is sent out, when a new payee needs to be created
//    * @sa KMyMoneyCombo::createItem()
//    *
//    * @param txt The name of the payee to be created
//    * @param id A connected slot should store the id of the created object in this variable
//    */
//  void createPayee(const QString& txt, QString& id);

//  /**
//    * This signal is sent out, when a new category needs to be created
//    * @sa KMyMoneyCombo::createItem()
//    *
//    * @param txt The name of the category to be created
//    * @param id A connected slot should store the id of the created object in this variable
//    */
//  void createCategory(const QString& txt, QString& id);

protected:
  KMyMoneyWizardPrivate * const d_ptr;
  KMyMoneyWizard(KMyMoneyWizardPrivate &dd, QWidget* parent = nullptr, bool modal = false, Qt::WindowFlags f = 0);
  /**
    * Constructor (kept protected, so that one cannot create such an object directly)
    */
  explicit KMyMoneyWizard(QWidget* parent = nullptr, bool modal = false, Qt::WindowFlags f = 0);

protected Q_SLOTS:
  void accept() override;
  void completeStateChanged();

private Q_SLOTS:
  void backButtonClicked();
  void nextButtonClicked();
  void helpButtonClicked();

private:
  Q_DECLARE_PRIVATE(KMyMoneyWizard)
};

#endif
