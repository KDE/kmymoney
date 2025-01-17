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

#include "KRecentDirs"

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyPlugin::KMMImportInterface::KMMImportInterface(QObject* parent, const char* name)
    : ImportInterface(parent, name)
{
}

QUrl KMyMoneyPlugin::KMMImportInterface::selectFile(const QString& title,
                                                    const QString& recentDirId,
                                                    const QString& mask,
                                                    QFileDialog::FileMode mode,
                                                    QWidget* widget) const
{
    const auto recentDirSelector = QStringLiteral(":%1").arg(recentDirId);
    auto path = KRecentDirs::dir(recentDirSelector);

    QPointer<QFileDialog> dialog = new QFileDialog(nullptr, title, path, mask);
    dialog->setFileMode(mode);
    dialog->setOption(QFileDialog::DontUseNativeDialog);

    // insert additional widgets into the file open dialog
    QGridLayout* layout = qobject_cast<QGridLayout*>(dialog->layout());
    if (layout) {
        layout->addWidget(widget, layout->rowCount(), 0, 1, layout->columnCount());
    }

    // increase dialog height by the height of the additionally added widget
    dialog->resize(dialog->width(), dialog->height() + widget->height());

    QUrl url;
    if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
        QList<QUrl> selectedUrls = dialog->selectedUrls();
        if (!selectedUrls.isEmpty()) {
            url = selectedUrls.first();
            if (!recentDirId.isEmpty()) {
                KRecentDirs::add(recentDirSelector, url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path());
            }
        }
    }

    // in case we have an additional widget, we remove it from the
    // dialog, so that the caller can still access it. Therefore, it is
    // the callers responsibility to delete the object

    if (widget)
        widget->setParent(nullptr);

    delete dialog;

    return url;
}
