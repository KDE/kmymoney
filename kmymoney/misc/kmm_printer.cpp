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

static QPrinter* globalPrinter(nullptr);
static QPrinter* globalPdfPrinter(nullptr);
static QPrintDialog* globalDialog(nullptr);
static QPageSetupDialog* globalPdfDialog(nullptr);

KMyMoneyPrinter::KMyMoneyPrinter()
{
}

QPrinter* KMyMoneyPrinter::instance(QPrinter::PrinterMode mode)
{
    if (globalPrinter == nullptr) {
        globalPrinter = new QPrinter(mode);
    }
    return globalPrinter;
}

QPrintDialog* KMyMoneyPrinter::dialog()
{
    if (globalDialog == nullptr) {
        globalDialog = new QPrintDialog(instance());
        globalDialog->setWindowTitle(QString());
    }
    return globalDialog;
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
    delete globalDialog;
    delete globalPrinter;
    globalDialog = nullptr;
    globalPrinter = nullptr;
}

KMyMoneyPDFPrinter::KMyMoneyPDFPrinter()
{
}

QPrinter* KMyMoneyPDFPrinter::instance()
{
    if (globalPdfPrinter == nullptr) {
        globalPdfPrinter = new QPrinter(QPrinter::HighResolution);
        globalPdfPrinter->setOutputFormat(QPrinter::PdfFormat);
    }
    return globalPdfPrinter;
}

QPageSetupDialog* KMyMoneyPDFPrinter::dialog(const QString& title)
{
    if (globalPdfDialog == nullptr) {
        globalPdfDialog = new QPageSetupDialog(instance());
    }
    globalPdfDialog->setWindowTitle(title);
    return globalPdfDialog;
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
    delete globalPdfDialog;
    delete globalPdfPrinter;
    globalPdfDialog = nullptr;
    globalPdfPrinter = nullptr;
}
