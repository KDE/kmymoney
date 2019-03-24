/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 *
 * Copyright (C) 2019      Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
