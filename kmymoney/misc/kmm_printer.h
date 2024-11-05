/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMM_PRINTER
#define KMM_PRINTER

#include <QPrinter>
#include <kmm_printer_export.h>

class QPageSetupDialog;
class QPrintDialog;

class KMM_PRINTER_EXPORT KMyMoneyPrinter
{
    KMyMoneyPrinter();
protected:
    static QPrintDialog* dialog();

public:
    static QPrinter* instance(QPrinter::PrinterMode mode = QPrinter::ScreenResolution);
    static QPrinter* startPrint(QPrinter::PrinterMode mode = QPrinter::ScreenResolution);
    static void cleanup();
};

class KMM_PRINTER_EXPORT KMyMoneyPDFPrinter
{
    KMyMoneyPDFPrinter();

protected:
    static QPageSetupDialog* dialog(const QString& title = QString());

public:
    static QPrinter* instance();
    static QPrinter* startPrint(const QString& title = QString());
    static void cleanup();
};

#endif
