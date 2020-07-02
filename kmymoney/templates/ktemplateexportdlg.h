/*
 * Copyright 2017       Ralf Habacker <ralf.habacker@freenet.de>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KTEMPLATEEXPORTDLG_H
#define KTEMPLATEEXPORTDLG_H

#include "kmm_templates_export.h"

#include <QDialog>

namespace Ui {
  class KTemplateExportDlg;
}

class KMM_TEMPLATES_EXPORT KTemplateExportDlg : public QDialog
{
    Q_OBJECT

public:
    explicit KTemplateExportDlg(QWidget *parent = nullptr);
    ~KTemplateExportDlg();

    QString title() const;
    QString shortDescription() const;
    QString longDescription() const;

private:
    Ui::KTemplateExportDlg *ui;
};

#endif // KTEMPLATEEXPORTDLG_H
