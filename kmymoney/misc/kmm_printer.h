/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#ifndef KMM_PRINTER
#define KMM_PRINTER

#include <QPrinter>
#include <kmm_printer_export.h>

class QPrintDialog;

class KMM_PRINTER_EXPORT KMyMoneyPrinter
{
    KMyMoneyPrinter();
protected:
    static QPrintDialog* dialog();
    static QPrinter* instance(QPrinter::PrinterMode mode = QPrinter::ScreenResolution);

public:
    static QPrinter* startPrint(QPrinter::PrinterMode mode = QPrinter::ScreenResolution);
    static void cleanup();
};

#endif
