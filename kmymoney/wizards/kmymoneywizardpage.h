/***************************************************************************
                             kmymoneywizardpage.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYWIZARDPAGE_H
#define KMYMONEYWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QWidget;

class KMyMoneyWizard;

/**
  * @author Thomas Baumgart (C) 2006
  *
  * @note: the following documentation is somewhat outdated
  * as of May 2007. Wizards should use a namespace
  * for the pages and can use the WizardPage<T> template class.
  * See the NewUserWizard class and NewUserWizardPages namespace
  * as an example of this setup.
  *
  * This class represents the base class for wizard pages for the
  * KMyMoneyWizard. One cannot create a wizard page directly, but
  * must derive from it. The KMyMoneyWizardPage class provides the
  * necessary functionality to work in concert with KMyMoneyWizard.
  *
  * Therefore, few steps are necessary to use this class. They seem to
  * be awkward at some stage, but I wanted to be able to use Qt designer
  * to actually design the widget for the page. That's why some things
  * aren't implemented in a more straight fashion than one would
  * normally do this.
  *
  * The first step is to derive a specific base page for the specific wizard.
  * In this example we use the name NewUser as template for the specific wizard.
  * This class provides a 'back'-pointer to the actual wizard object
  * for all pages.
  *
  * @code
  * class KNewUserPage : public KMyMoneyWizardPage
  * {
  * public:
  *   KNewUserPage(unsigned int step, QWidget* widget, KNewUserWizard* parent, const char* name);
  *
  * protected:
  *   KNewUserWizard*    m_wizard;
  * }
  * @endcode
  *
  * The implementation of this class is rather straight-forward:
  *
  * @code
  * KNewUserPage::KNewUserPage(unsigned int step, QWidget* widget, KNewUserWizard* parent, const char* name) :
  *   KMyMoneyWizardPage(step, widget, name),
  *   m_wizard(parent)
  * {
  * }
  * @endcode
  *
  * For each page of the wizard, you will have to create a @p ui file with
  * Qt designer.
  * Let's assume we call the first page of the wizard 'General' and go
  * from there.
  * We also assume, that the wizard has more than one page.
  * The ui designer generated class should have the name KNewUserGeneral
  * as all other dialogs. The class definition of KNewUserGeneral will
  * look like this:
  *
  * @code
  * class KNewUserGeneral : public KNewUserGeneral, public KNewUserPage
  * {
  *   Q_OBJECT
  * public:
  *   KNewUserGeneral(KNewUserWizard* parent, const char* name = 0);
  *   KMyMoneyWizardPage* nextPage();
  *   bool isLastPage() { return false; }
  *
  * protected:
  *   KNewUserWizard*    m_wizard;
  * }
  * @endcode
  *
  * The implementation depends heavily on the logic of your code. If you only
  * fill some widgets, it could be as simple as:
  *
  * @code
  * KNewUserGeneral::KNewUserGeneral(KNewUserWizard* parent, const char* name) :
  *   KNewUserGeneral(parent),
  *   KNewUserPage(1, this, parent, name)
  * {
  *   KMandatoryFieldGroup* mandatoryGroup = new KMandatoryFieldGroup(this);
  *   mandatoryGroup->add(m_userName);
  *   connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  * }
  *
  * KMyMoneyWizardPage* KNewUserGeneral::nextPage()
  * {
  *   return m_wizard->m_personalPage;
  * }
  * @endcode
  *
  * A note on the first parameter to KNewUserPage in the above example: it ties
  * this page to be part of step 1 (see KMyMoneyWizard::addStep() for details).
  *
  * Depending on the actual logic of the page, you would want to override the
  * following methods: resetPage, nextPage, isLastPage and isComplete.
  *
  * @note The implementation of this class is heavily based on ideas found at
  *       https://doc.qt.io/qt-5/qtwidgets-dialogs-licensewizard-example.html
  */
class KMyMoneyWizardPagePrivate;
class KMyMoneyWizardPage
{
public:
  /**
    * This method is called by the wizard when the page is entered from
    * the previous page. The default implementation does nothing.
    */
  virtual void enterPage();

  /**
    * This method is called by the wizard when the page is left to return to
    * the previous page. The default implementation does nothing.
    */
  virtual void leavePage();

  /**
    * This method is called by the wizard whenever a page is entered
    * (either in forward or backward direction). The default
    * implementation does nothing.
    */
  virtual void resetPage();

  /**
    * This method returns a pointer to the next page that should be
    * shown when the user presses the 'Next' button.
    *
    * @return pointer to next wizard page
    */
  virtual KMyMoneyWizardPage* nextPage() const;

  /**
    * This returns, if the current page is the last page of the wizard.
    * The default implementation returns @p false if nextPage() returns 0,
    * @p true otherwise.
    *
    * @retval false more pages follow
    * @retval true this is the last page of the wizard
    */
  virtual bool isLastPage() const;

  /**
    * This returns, if all necessary data for this page has been
    * filled. It is used to enabled the 'Next' or 'Finish' button.
    * The button is only enabled, if this method returns @p true,
    * which is the default implementation.
    *
    * @retval false more data required from the user before we can proceed
    * @retval true all data available, we allow to switch to the next page
    */
  virtual bool isComplete() const;

  /**
    * This method returns the step to which this page belongs.
    * It is required by the KMyMoneyWizard and is not intended
    * to be used by application code.
    *
    * @return step of wizard this page belongs to
    */
  unsigned int step() const;

  /**
    * This method returns a pointer to the widget of the page.
    * It is required by the KMyMoneyWizard and is not intended
    * to be used by application code.
    *
    * @return pointer to widget of page
    */
  QWidget* widget() const;

  /**
    * This method returns a pointer to the QObject used for
    * the signal/slot mechanism.
    * It is required by the KMyMoneyWizard and can be used
    * by application code for signal/slot connections as well.
    * Other use is not foreseen.
    */
  const KMyMoneyWizardPagePrivate *object() const;

  /**
    * This method returns a pointer to the widget which should
    * receive the focus when the page is opened.
    *
    * @return pointer to widget or 0 if none is to be selected
    *         The default implementation returns 0
    */
  virtual QWidget* initialFocusWidget() const;

  virtual KMyMoneyWizard* wizard() const = 0;

  /**
   * This method returns a specific help context for the page shown
   * The default returns an empty string.
   */
  virtual QString helpContext() const;

  virtual ~KMyMoneyWizardPage();
protected:
  KMyMoneyWizardPagePrivate * const d_ptr;

  KMyMoneyWizardPage(KMyMoneyWizardPagePrivate &dd, uint step, QWidget *widget);
  Q_DECLARE_PRIVATE(KMyMoneyWizardPage)

  /**
    * Constructor (kept protected, so that one cannot create such an object directly)
    */
  explicit KMyMoneyWizardPage(uint step, QWidget* widget);

  /**
    * This method must be called by the implementation when the
    * data in the fields of the wizard change and the state of
    * completeness changed.
    *
    * @note If you do not override isComplete() then there is no need
    * to call this method.
    */
  void completeStateChanged();
};
#endif
