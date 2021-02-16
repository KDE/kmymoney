/*
    SPDX-FileCopyrightText: 2017 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTEMPLATEEXPORTDLG_H
#define KTEMPLATEEXPORTDLG_H

#include <QDialog>

namespace Ui {
class KTemplateExportDlg;
}

class KTemplateExportDlg : public QDialog
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
