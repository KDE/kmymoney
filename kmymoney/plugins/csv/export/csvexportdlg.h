/*
    SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSVEXPORTDLG_H
#define CSVEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QDate>
#include <QDialog>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

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
class CsvExportDlgPrivate;
class CsvExportDlg : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CsvExportDlg);

public:
    explicit CsvExportDlg(QWidget *parent = 0);
    ~CsvExportDlg() noexcept;

    /**
      * This method returns the filename entered into the edit field
      *
      * @return QString with filename
      */
    QString filename() const;

    /**
      * This method returns the start date of the export dialog
      */
    QDate startDate() const;

    /**
      * This method returns the end date of the export dialog
      */
    QDate endDate() const;

    /**
      * This method returns the state of the account radioButton
      */
    bool accountSelected() const;

    /**
      * This method returns the state of the category radioButton
      */
    bool categorySelected() const;

    /**
      * This method returns the accountId of the selected file
      */
    QString accountId() const;

    /**
      * This method returns the field separator value
      */
    QString separator() const;

private:
    QScopedPointer<CsvExportDlgPrivate> d_ptr;
};
#endif
