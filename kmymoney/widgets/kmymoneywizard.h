/***************************************************************************
                             kmymoneywizard.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#ifndef KMYMONEYWIZARD_H
#define KMYMONEYWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QList>
#include <QPalette>
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QFrame;

// ----------------------------------------------------------------------------
// KDE Includes

class QPushButton;

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyTitleLabel;
class KMyMoneyWizardPage;

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
  *       http://doc.trolltech.com/4.1/dialogs-complexwizard.html
  */
class KMyMoneyWizard : public QDialog
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

signals:
  /**
    * This signal is sent out, when a new payee needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the payee to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createPayee(const QString& txt, QString& id);

  /**
    * This signal is sent out, when a new category needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the category to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createCategory(const QString& txt, QString& id);

protected:
  /**
    * Constructor (kept protected, so that one cannot create such an object directly)
    */
  explicit KMyMoneyWizard(QWidget* parent = nullptr, bool modal = false, Qt::WindowFlags f = 0);

  /**
    * This method sets up the first page after creation of the object
    *
    * @param page pointer to first page of wizard
    */
  void setFirstPage(KMyMoneyWizardPage* page);

  /**
    * This method allows to hide or show a @p step.
    *
    * @param step step to be shown/hidden
    * @param hidden hide step if true (the default) or show it if false
    */
  void setStepHidden(int step, bool hidden = true);

protected slots:
  virtual void accept();
  void completeStateChanged();

private:
  void updateStepCount();

private slots:
  void backButtonClicked();
  void nextButtonClicked();
  void helpButtonClicked();

protected:
  /*
   * The buttons
   */
  QPushButton*          m_cancelButton;
  QPushButton*          m_backButton;
  QPushButton*          m_nextButton;
  QPushButton*          m_finishButton;
  QPushButton*          m_helpButton;

private:
  /**
    * Switch to page which is currently the top of the history stack.
    * @p oldPage is a pointer to the current page or 0 if no page
    * is shown.
    *
    * @param oldPage pointer to currently displayed page
    */
  void switchPage(KMyMoneyWizardPage* oldPage);

  /**
    * This method selects the step given by @p step.
    *
    * @param step step to be selected
    */
  void selectStep(int step);

  /*
   * The layouts
   */
  QVBoxLayout*          m_wizardLayout;
  QVBoxLayout*          m_stepLayout;
  QVBoxLayout*          m_pageLayout;
  QHBoxLayout*          m_buttonLayout;

  /*
   * Some misc. widgets required
   */
  QFrame*               m_stepFrame;
  QLabel*               m_stepLabel;
  QPalette              m_stepPalette;

  QList<QLabel*>        m_steps;      // the list of step labels
  int                   m_step;       // the currently selected step

  /*
   * The title bar
   */
  KMyMoneyTitleLabel*   m_titleLabel;

  /*
   * The history stack
   */
  QList<KMyMoneyWizardPage*> m_history;

  QString               m_helpContext;
};



#endif
