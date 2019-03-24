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

#include "kmm_printer.h"

#include <QPrinter>
#include <QPointer>
#include <QPrintDialog>
#include <QScopedPointer>
#include <QDebug>

// Q_LOGGING_CATEGORY(Print, "Printing")

KMyMoneyPrinter::KMyMoneyPrinter()
{
}

QPrinter* KMyMoneyPrinter::instance(QPrinter::PrinterMode mode)
{
    static QPrinter* printer(nullptr);

    if (printer == nullptr) {
        printer = new QPrinter(mode);
    }
    return printer;
}

QPrintDialog* KMyMoneyPrinter::dialog()
{
    static QPrintDialog* dialog(nullptr);

    if (dialog == nullptr) {
      dialog = new QPrintDialog(instance());
      dialog->setWindowTitle(QString());
    }
    return dialog;
}

QPrinter* KMyMoneyPrinter::startPrint(QPrinter::PrinterMode mode)
{
    QPrinter *printer = instance(mode);

    if (dialog()->exec() != QDialog::Accepted)
        return nullptr;
    return printer;
}

void KMyMoneyPrinter::cleanup()
{
  auto printer = instance();
  auto dlg = dialog();

  delete dlg;
  delete printer;
}
