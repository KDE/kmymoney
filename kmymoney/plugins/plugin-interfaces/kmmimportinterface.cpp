/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmimportinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>
#include <QGridLayout>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyPlugin::KMMImportInterface::KMMImportInterface(QObject* parent, const char* name)
    : ImportInterface(parent, name)
{
}

QUrl KMyMoneyPlugin::KMMImportInterface::selectFile(const QString& title,
                                                    const QString& path,
                                                    const QString& mask,
                                                    QFileDialog::FileMode mode,
                                                    QWidget* widget) const
{
    //    QString path(_path);

    // if the path is not specified open the file dialog in the last used directory
    // 'kmymoney' is the keyword that identifies the last used directory in KFileDialog
    //    if (path.isEmpty()) {
    //      path = KRecentDirs::dir(":kmymoney-import");
    //    }

    QPointer<QFileDialog> dialog = new QFileDialog(nullptr, title, path, mask);
    dialog->setFileMode(mode);
    dialog->setOption(QFileDialog::DontUseNativeDialog);

    // insert additional widgets into the file open dialog
    QGridLayout* layout = qobject_cast<QGridLayout*>(dialog->layout());
    if (layout) {
        layout->addWidget(widget, layout->rowCount(), 0, 1, layout->columnCount());
    }

    QUrl url;
    if (dialog->exec() == QDialog::Accepted && dialog != 0) {
        QList<QUrl> selectedUrls = dialog->selectedUrls();
        if (!selectedUrls.isEmpty()) {
            url = selectedUrls.first();
            //        if (_path.isEmpty()) {
            //          KRecentDirs::add(":kmymoney-import", url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path());
            //        }
        }
    }

    // in case we have an additional widget, we remove it from the
    // dialog, so that the caller can still access it. Therefore, it is
    // the callers responsibility to delete the object

    if (widget)
        widget->setParent(0);

    delete dialog;

    return url;
}
