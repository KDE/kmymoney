/*
    SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kaccounttemplateselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccounttemplateselector.h"

#include <mymoneytemplate.h>
#include <templatesmodel.h>
#include <mymoneyaccount.h>

class KAccountTemplateSelectorPrivate
{
    Q_DISABLE_COPY(KAccountTemplateSelectorPrivate)

public:
    Ui::KAccountTemplateSelector*     ui;
    TemplatesModel*                   model;
    QMap<QString, QTreeWidgetItem*>   templateHierarchy;

public:
    KAccountTemplateSelectorPrivate()
        : ui(new Ui::KAccountTemplateSelector)
        , model(nullptr)
    {
    }

    ~KAccountTemplateSelectorPrivate()
    {
        delete ui;
    }


    QTreeWidgetItem* hierarchyItem(const QString& parent, const QString& name)
    {
        if (!templateHierarchy.contains(parent)
                || templateHierarchy[parent] == 0) {
            QRegExp exp("(.*):(.*)");
            if (exp.indexIn(parent) != -1)
                templateHierarchy[parent] = hierarchyItem(exp.cap(1), exp.cap(2));
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(templateHierarchy[parent]);
        item->setText(0, name);
        return item;
    }

    bool hierarchy(QMap<QString, QTreeWidgetItem*>& list, const QString& parent, QDomNode account)
    {
        bool rc = true;
        while (rc == true && !account.isNull()) {
            if (account.isElement()) {
                QDomElement accountElement = account.toElement();
                if (accountElement.tagName() == QLatin1String("account")) {
                    QString name = QString("%1:%2").arg(parent).arg(accountElement.attribute("name"));
                    list[name] = 0;
                    hierarchy(list, name, account.firstChild());
                }
            }
            account = account.nextSibling();
        }
        return rc;
    }

    void hierarchy(QDomNode accounts, QMap<QString, QTreeWidgetItem*>& list)
    {
        bool rc = !accounts.isNull();
        while (rc == true && !accounts.isNull() && accounts.isElement()) {
            QDomElement rootNode = accounts.toElement();
            QString name = rootNode.attribute(QLatin1String("name"));
            if (rootNode.tagName() == QLatin1String("account")) {
                rootNode = rootNode.firstChild().toElement();
                eMyMoney::Account::Type type = static_cast<eMyMoney::Account::Type>(accounts.toElement().attribute(QLatin1String("type")).toUInt());
                switch (type) {
                case eMyMoney::Account::Type::Asset:
                case eMyMoney::Account::Type::Liability:
                case eMyMoney::Account::Type::Income:
                case eMyMoney::Account::Type::Expense:
                case eMyMoney::Account::Type::Equity:
                    if (name.isEmpty())
                        name = MyMoneyAccount::accountTypeToString(type);
                    list[name] = 0;
                    rc = hierarchy(list, name, rootNode);
                    break;

                default:
                    rc = false;
                    break;
                }
            } else {
                rc = false;
            }
            accounts = accounts.nextSibling();
        }
    }

    void loadHierarchy()
    {
        if (model == nullptr) {
            return;
        }
        templateHierarchy.clear();
        const auto selection = ui->m_groupList->selectionModel()->selectedIndexes();
        for (const auto& idx : qAsConst(selection)) {
            const auto tmpl = model->itemByIndex(idx);
            hierarchy(tmpl.accountTree(), templateHierarchy);
        }

        // I need to think about this some more. The code works and shows
        // the current account hierarchy. It might be useful, to show
        // existing accounts dimmed and the new ones in bold or so.
#if 0
        // add the hierarchy from the MyMoneyFile object
        QList<MyMoneyAccount> aList;
        QList<MyMoneyAccount>::const_iterator it_a;
        auto file = MyMoneyFile::instance();
        file->accountList(aList);
        if (aList.count() > 0) {
            templateHierarchy[file->accountToCategory(file->asset().id(), true)] = 0;
            templateHierarchy[file->accountToCategory(file->liability().id(), true)] = 0;
            templateHierarchy[file->accountToCategory(file->income().id(), true)] = 0;
            templateHierarchy[file->accountToCategory(file->expense().id(), true)] = 0;
            templateHierarchy[file->accountToCategory(file->equity().id(), true)] = 0;
        }

        for (it_a = aList.begin(); it_a != aList.end(); ++it_a) {
            templateHierarchy[file->accountToCategory((*it_a).id(), true)] = 0;
        }
#endif

        ui->m_accountList->clear();

        QRegExp exp("(.*):(.*)");
        for (QMap<QString, QTreeWidgetItem*>::iterator it_h = templateHierarchy.begin(); it_h != templateHierarchy.end(); ++it_h) {
            if (exp.indexIn(it_h.key()) == -1) {
                (*it_h) = new QTreeWidgetItem(ui->m_accountList);
                (*it_h)->setText(0, it_h.key());
            } else {
                (*it_h) = hierarchyItem(exp.cap(1), exp.cap(2));
            }
            (*it_h)->setExpanded(true);
        }

        ui->m_description->clear();
        const auto idx = ui->m_groupList->currentIndex();
        if (idx.isValid()) {
            const auto desc = idx.data(eMyMoney::Model::TemplatesLongDescriptionRole).toString();
            ui->m_description->setText(desc);
        }
    }
};

KAccountTemplateSelector::KAccountTemplateSelector(QWidget* parent) :
    QWidget(parent),
    d_ptr(new KAccountTemplateSelectorPrivate)
{
    Q_D(KAccountTemplateSelector);
    d->ui->setupUi(this);
    d->ui->m_accountList->header()->hide();
    d->ui->m_groupList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->ui->m_groupList->setFocus();
}

KAccountTemplateSelector::~KAccountTemplateSelector()
{
    Q_D(KAccountTemplateSelector);
    delete d;
}

void KAccountTemplateSelector::setModel(TemplatesModel* model)
{
    Q_D(KAccountTemplateSelector);
    d->model = model;
    d->ui->m_groupList->setModel(model);
    connect(d->ui->m_groupList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
        Q_D(KAccountTemplateSelector);
        d->loadHierarchy();
    }, Qt::UniqueConnection);
}

QList<MyMoneyTemplate> KAccountTemplateSelector::selectedTemplates() const
{
    Q_D(const KAccountTemplateSelector);
    QList<MyMoneyTemplate> list;
    if (d->model != nullptr) {
        const auto selection = d->ui->m_groupList->selectionModel()->selectedIndexes();
        for (const auto& idx : qAsConst(selection)) {
            list << d->model->itemByIndex(idx);
        }
    }
    return list;
}

void KAccountTemplateSelector::setupInitialSelection()
{
    Q_D(KAccountTemplateSelector);
    const auto defaultCountry = QLocale().country();

    const auto rows = d->model->rowCount();
    for (int row = 0; row < rows; ++row) {
        auto idx = d->model->index(row, 0);
        const auto locale = idx.data(eMyMoney::Model::TemplatesLocaleRole).toString();

        if (QLocale(locale).country() == defaultCountry) {
            d->ui->m_groupList->collapseAll();
            d->ui->m_groupList->setExpanded(idx, true);
            d->ui->m_groupList->scrollTo(idx, QAbstractItemView::PositionAtTop);
            d->ui->m_groupList->setCurrentIndex(idx);
            d->ui->m_groupList->resizeColumnToContents(0);
            break;
        }
    }
}
