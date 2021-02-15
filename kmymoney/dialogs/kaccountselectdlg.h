/*
    SPDX-FileCopyrightText: 2003-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACCOUNTSELECTDLG_H
#define KACCOUNTSELECTDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Thomas Baumgart
  */

class MyMoneyAccount;
class MyMoneyInstitution;

namespace eDialogs { enum Category : int; }

class KAccountSelectDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KAccountSelectDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KAccountSelectDlg)

public:
  explicit KAccountSelectDlg(const eDialogs::Category type, const QString& purpose, QWidget *parent = nullptr);
  ~KAccountSelectDlg();

  /**
    * This method is used to setup the descriptive text in the account
    * selection dialog box. The @p msg should contain a descriptive
    * text about the purpose of the dialog and it's options.
    *
    * @param msg const reference to QString object containing the text.
    */
  void setDescription(const QString& msg);

  /**
    * This method is used to setup the buddy text of the account
    * selection box. the @p msg should contain a short text
    * which is placed above the selection box with the account
    * names.
    *
    * @param msg const reference to QString object containing the text.
    */
  void setHeader(const QString& msg);

  /**
    * This method is used to pass information to the account selection
    * dialog which will be used as initial selection in the account
    * selection combo box and during account creation.
    *
    * @param account MyMoneyAccount filled with the relevant and available information
    * @param id account id to be used.
    */
  void setAccount(const MyMoneyAccount& account, const QString& id);

  /**
    * This method returns the name of the selected account in the combo box.
    *
    * @return QString containing the id of the selected account
    */
  QString selectedAccount() const;

  /**
    * This method is used to set the mode of the dialog. Two modes
    * are supplied: a) select or create and b) create only.
    * If @p mode is 0, select or create is selected, otherwise create only
    * is selected.
    *
    * @param mode selected mode
    */
  void setMode(const int mode);

  /**
    * This method allows to control the visibility of the abort button
    * in this dialog according to the parameter @p visible.
    *
    * @param visible @p true shows the abort button, @p false hides it.
    */
  void showAbortButton(const bool visible);

  /**
    * This method is used to determine if the user pressed the 'Skip' or
    * the 'Abort' button. The return value is valid only, if the exec()
    * function of the dialog returns false.
    *
    * @retval false Dialog was left using the 'Skip' button
    * @retval true Dialog was left using the 'Abort' button
    */
  bool aborted() const;

  void hideQifEntry();

public Q_SLOTS:
  /**
    * Reimplemented from QDialog
    */
  int exec() override;

protected Q_SLOTS:
  /**
    * This slot is used to fire up the new account wizard and preset it
    * with the values found in m_account. If an account was created using
    * the wizard, this will be the selected account.
    */
  void slotCreateAccount();

  /**
    * This slot is used to react on the abort button
    */
  void abort();

Q_SIGNALS:
  void createAccount(MyMoneyAccount& account);
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

private:
  KAccountSelectDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KAccountSelectDlg)
};

#endif

// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
