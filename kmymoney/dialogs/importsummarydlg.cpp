/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importsummarydlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QHeaderView>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "statementmodel.h"

#include "ui_importsummarydlg.h"

class ImportSummaryDialogPrivate
{
    Q_DISABLE_COPY_MOVE(ImportSummaryDialogPrivate)

public:
    ImportSummaryDialogPrivate(ImportSummaryDialog* p)
        : parent(p)
        , ui(new Ui_ImportSummaryDialog)
        , resultModel(nullptr)
    {
    }

    ~ImportSummaryDialogPrivate()
    {
        delete ui;
    }

    ImportSummaryDialog* parent;
    Ui_ImportSummaryDialog* ui;
    StatementModel* resultModel;
};

ImportSummaryDialog::ImportSummaryDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , d(new ImportSummaryDialogPrivate(this))
{
    d->ui->setupUi(this);
    d->ui->m_resultTreeView->setSkipRootLevelEntries(true);

    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    restoreGeometry(grp.readEntry("ImportSummaryDialogGeometry", QByteArray()));
    d->ui->m_resultTreeView->header()->restoreState(grp.readEntry("ImportSummaryDialogColumns", QByteArray()));
}

ImportSummaryDialog::~ImportSummaryDialog()
{
    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("ImportSummaryDialogGeometry", saveGeometry());
    grp.writeEntry("ImportSummaryDialogColumns", d->ui->m_resultTreeView->header()->saveState());
    delete d;
}

int ImportSummaryDialog::exec()
{
    if (!d->ui->m_resultTreeView->model()) {
        qWarning() << "ImportSummaryDialog::exec() executed without a model. Use setModel() before calling exec().";
        return QDialog::Rejected;
    }
    return QDialog::exec();
}

void ImportSummaryDialog::setModel(StatementModel* model) const
{
    d->resultModel = model;
    d->ui->m_resultTreeView->setModel(model);

    const auto statementCount = model->statementCount();
    d->ui->m_resultSummaryLabel->setText(
        i18np("One statement has been processed with the following results:", "%1 statements have been processed with the following results:", statementCount));

    auto headerView = d->ui->m_resultTreeView->header();
    d->ui->m_resultTreeView->expandAll();
    headerView->resizeSections(QHeaderView::ResizeToContents);
}
