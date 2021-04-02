/*
    SPDX-FileCopyrightText: 2005 Tony Bloomfield <tonybloom@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGNCIMPORTOPTIONSDLG_H
#define KGNCIMPORTOPTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTextCodec;

class KGncImportOptionsDlgPrivate;
class KGncImportOptionsDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KGncImportOptionsDlg)

public:
    explicit KGncImportOptionsDlg(QWidget *parent = nullptr);
    ~KGncImportOptionsDlg();

    int investmentOption() const;
    bool quoteOption() const;
    bool scheduleOption() const;
    QTextCodec* decodeOption();
    bool txNotesOption() const;
    bool generalDebugOption() const;
    bool xmlDebugOption() const;
    bool anonymizeOption() const;

public Q_SLOTS:
    void slotDecodeOptionChanged(bool);
    void slotHelp();

private:
    KGncImportOptionsDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KGncImportOptionsDlg)
};

#endif
