/*
    SPDX-FileCopyrightText: 2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Javier Campos Morales <javi_c@ctv.es>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@mail.wesleyan.edu>
    SPDX-FileCopyrightText: 2001 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KEXPORTDLG_H
#define KEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QString>
#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "ui_kexportdlgdecl.h"

/**
  * This class is used to select the required user input to export
  * a specified account to the popular QIF format.
  * It relies upon the QIF file handling routines in MyMoneyQifProfile and
  * MyMoneyQifWriter to do the actual writing of QIF files.
  *
  * It uses the global KConfig object to read and write the application
  * settings.
  *
  * @see MyMoneyAccount, MyMoneyQifProfile, MyMoneyQifProfileEditor
  *
  * @author Felix Rodriguez, Michael Edwardes, Thomas Baumgart 2000-2003
  *
  * @short A class to select user data required to export a specified account to the popular QIF format.
  **/

class KExportDlgDecl : public QDialog, public Ui::KExportDlgDecl
{
public:
    explicit KExportDlgDecl(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};

class KExportDlg : public KExportDlgDecl
{
    Q_OBJECT

public:
    explicit KExportDlg(QWidget *parent);
    ~KExportDlg();

    /**
      * This method returns the filename entered into the edit field
      *
      * @return QString with filename
      */
    const QString filename() const {
        return m_qlineeditFile->text();
    };

    /**
      * This method returns the account id that has been selected for export
      *
      * @return QString with account id
      */
    QString accountId() const;

    /**
      * This method returns the name of the profile that has been selected
      * for the export operation
      *
      * @return QString with profile name
      */
    const QString profile() const {
        return m_profileComboBox->currentText();
    };

    /**
      * This method returns the start date of the export dialog
      */
    const QDate startDate() const {
        return m_kmymoneydateStart->date();
    };

    /**
      * This method returns the end date of the export dialog
      */
    const QDate endDate() const {
        return m_kmymoneydateEnd->date();
    };

    /**
      * This method returns the state of the account checkbox
      */
    bool accountSelected() const {
        return m_qcheckboxAccount->isChecked();
    };

    /**
      * This method returns the state of the account checkbox
      */
    bool categorySelected() const {
        return m_qcheckboxCategories->isChecked();
    };

protected Q_SLOTS:
    /**
      * Called when the user clicked on the OK button
      */
    void slotOkClicked();

    /**
      * Called when the user needs to browse the filesystem for a QIF file
      */
    void slotBrowse();

    /**
      * This slot checks whether all data is correct to enable
      * the 'Export' button. The enable state of the 'Export' button
      * is updated appropriately.
      *
      * If the parameter @p account is not empty, then it is assumed
      * a new account is selected and the date fields will be loaded
      * with the date of the first and last transaction within this
      * account.
      *
      * @param account The id of the selected account.
      */
    void checkData(const QString& account = QString());

private:
    void readConfig();
    void writeConfig();

    /**
      * This method loads the available profiles into
      * the combo box. The parameter @p selectLast controls if
      * the last profile used is preset or not. If preset is not
      * selected, the current selection remains. If the currently selected
      * text is not present in the list anymore, the first item will be
      * selected.
      *
      * @param selectLast If true, the last used profile is selected. The
      *                   default is false.
      */
    void loadProfiles(const bool selectLast = false);

    /**
      * This method is used to load the available accounts into the
      * combo box for selection.
      */
    void loadAccounts();

    /**
      * This method is used to load an account hierarchy into a string list
      *
      * @param strList Reference to the string list to setup
      * @param id Account id to add
      * @param leadIn constant leadin to be added in front of the account name
      */
    // void addCategories(QStringList& strList, const QString& id, const QString& leadIn) const;

    /**
      * This method is used to return the account id of a given account name
      *
      * @param account name of the account
      * @return the ID of the account will be returned.
      *         See MyMoneyFile::nameToAccount() for details.
      */
    // QString accountId(const QString& account) const;

private:
    QString   m_lastAccount;
};

#endif
