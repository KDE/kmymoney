/*
 * Copyright 2013-2014  Allan Anderson <agander93@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVEXPORTDLG_H
#define CSVEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QString>
#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

#include "ui_csvexportdlg.h"

/**
  * This class is used to collect the required user input to export
  * a specified account to the CSV format.
  * It relies upon the CSV file handling routines in
  * CsvWriter to do the actual writing of CSV files.
  *
  * It uses a KConfig object to read and write the application settings.
  *
  * @see MyMoneyAccount
  *
  * @author Felix Rodriguez, Michael Edwardes, Thomas Baumgart 2000-2003, Allan Anderson 2013.
  *
  * @short A class to select user data required to export a specified account to CSV format.
  **/

class CsvExportDlg : public QDialog
{
  Q_OBJECT

public:
  explicit CsvExportDlg(QWidget *parent = 0);
  ~CsvExportDlg();

  /**
    * This method returns the filename entered into the edit field
    *
    * @return QString with filename
    */
  const QString filename() const {
    return ui->m_qlineeditFile->text();
  };

  /**
    * This method returns the start date of the export dialog
    */
  QDate startDate() const {
    return ui->m_kmymoneydateStart->date();
  };

  /**
    * This method returns the end date of the export dialog
    */
  QDate endDate() const {
    return ui->m_kmymoneydateEnd->date();
  };

  /**
    * This method returns the state of the account radioButton
    */
  bool accountSelected() const {
    return ui->m_radioButtonAccount->isChecked();
  };

  /**
    * This method returns the state of the category radioButton
    */
  bool categorySelected() const {
    return ui->m_radioButtonCategories->isChecked();
  };

  /**
    * This method returns the accountId of the selected file
    */
  const QString accountId() const {
    return m_accountId;
  };

  /**
    * This method returns the field separator value
    */
  QString separator() {
    return m_separator;
  };

protected Q_SLOTS:
  /**
    * Called when the user clicked on the Export button
    */
  void slotOkClicked();

  /**
    * Called when the user needs to browse the filesystem for a CSV file
    */
  void slotBrowse();

  /**
    * Called when the user changes the field separator setting
    */
  void separator(int separatorIndex);


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
    * @param account The name of the selected account.
    */
  void checkData(const QString& account = QString());

  /**
    * This method returns a list of Asset or Liability types, but
    * excluding any Stocks.
    */
  QStringList getAccounts();

public Q_SLOTS:

  void slotStatusProgressBar(int current, int total);

private:

  void readConfig();
  void writeConfig();

  /**
    * This method is used to load the available accounts into the
    * combo box for selection.
    */
  void loadAccounts();

  Ui::CsvExportDlg* ui;
  QString           m_accountId;
  QString           m_separator;
  QStringList       m_idList;
  QStringList       m_fieldDelimiterCharList;
};

bool caseInsensitiveLessThan(const QString &s1, const QString &s2);

#endif
