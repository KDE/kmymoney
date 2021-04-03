/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSVWIZARDPAGE_H
#define CSVWIZARDPAGE_H

#include <QWizardPage>
#include "core/csvenums.h"

class CSVWizard;
class CSVImporterCore;

class CSVWizardPage : public QWizardPage
{
public:
    CSVWizardPage(CSVWizard *dlg, CSVImporterCore *imp) : QWizardPage(nullptr), m_dlg(dlg), m_imp(imp) {}

protected:
    CSVWizard   *m_dlg;
    CSVImporterCore *m_imp;
};

#endif // CSVWIZARDPAGE_H
