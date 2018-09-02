/*
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
