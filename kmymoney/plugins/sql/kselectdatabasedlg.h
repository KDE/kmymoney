/*
    SPDX-FileCopyrightText: 2005 Tony Bloomfield <tonybloom@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSELECTDATABASEDLG_H
#define KSELECTDATABASEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class KSelectDatabaseDlg;
}

class KMandatoryFieldGroup;
class KSelectDatabaseDlg : public QDialog
{
    Q_OBJECT
public:
    explicit KSelectDatabaseDlg(int openMode, QUrl openURL = QUrl(), QWidget *parent = nullptr);
    ~KSelectDatabaseDlg();

    /**
     * Check whether we have required database drivers
     * @return - false, no drivers available, true, can proceed
     */
    bool checkDrivers();

    /**
     * Return URL of database
     * @return - pseudo-URL of database selected by user
     */
    const QUrl selectedURL();

    /**
     * Execute the database selection dialog
     * @return - as QDialog::exec()
     */
    int exec() override;

public Q_SLOTS:
    void slotDriverSelected(int idx);
    void slotHelp();

private:
    Ui::KSelectDatabaseDlg* m_widget;
    int m_mode;
    QUrl m_url;
    KMandatoryFieldGroup* m_requiredFields;
    bool m_sqliteSelected;
};

#endif
