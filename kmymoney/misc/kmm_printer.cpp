/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmm_printer.h"

#include <QPageSetupDialog>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QScopedPointer>

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
    QPrinter* printer = instance(mode);

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

KMyMoneyPDFPrinter::KMyMoneyPDFPrinter()
{
}

QPrinter* KMyMoneyPDFPrinter::instance()
{
    static QPrinter* printer(nullptr);

    if (printer == nullptr) {
        printer = new QPrinter(QPrinter::HighResolution);
        printer->setOutputFormat(QPrinter::PdfFormat);
    }
    return printer;
}

QPageSetupDialog* KMyMoneyPDFPrinter::dialog(const QString& title)
{
    static QPageSetupDialog* dialog(nullptr);

    if (dialog == nullptr) {
        dialog = new QPageSetupDialog(instance());
    }
    dialog->setWindowTitle(title);
    return dialog;
}

QPrinter* KMyMoneyPDFPrinter::startPrint(const QString& title)
{
    QPrinter* printer = instance();

    if (dialog(title)->exec() != QDialog::Accepted)
        return nullptr;
    return printer;
}

void KMyMoneyPDFPrinter::cleanup()
{
    auto printer = instance();
    auto dlg = dialog();

    delete dlg;
    delete printer;
}
