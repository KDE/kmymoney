/*
    SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHBoxLayout>
#include <QComboBox>
#include <QAbstractItemModel>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ktaglabel.h"
#include "idfilter.h"
#include "mymoneyenums.h"

class KTagContainerPrivate
{
    Q_DISABLE_COPY(KTagContainerPrivate)
    Q_DECLARE_PUBLIC(KTagContainer)

public:
    KTagContainerPrivate(KTagContainer* parent)
        : q_ptr(parent)
        , m_tagCombo(nullptr)
        , m_idFilter(new IdFilter(parent))
    {
    }

    void addTagWidget(const QString& id)
    {
        Q_Q(KTagContainer);

        if (id.isEmpty() || m_idFilter->filterList().contains(id))
            return;

        // set index to the empty item since we remove
        // the selected item as part of this method
        m_tagCombo->setCurrentIndex(0);

        const auto tagName = m_tagCombo->itemText(m_tagCombo->findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
        KTagLabel *t = new KTagLabel(id, tagName, q);
        q->connect(t, &KTagLabel::clicked, q, &KTagContainer::slotRemoveTagWidget);
        m_tagLabelList.append(t);
        m_idFilter->addFilter(id);
        q->layout()->addWidget(t);
    }

    QStringList tagIdList() const
    {
        QStringList tags;
        for(const auto tag : qAsConst(m_tagLabelList)) {
            tags << tag->id();
        }
        return tags;
    }

    KTagContainer*              q_ptr;
    QComboBox*                  m_tagCombo;
    QScopedPointer<IdFilter>    m_idFilter;
    QList<KTagLabel*>           m_tagLabelList;
};

KTagContainer::KTagContainer(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new KTagContainerPrivate(this))
{
    Q_D(KTagContainer);
    d->m_tagCombo = new QComboBox(this);
    d->m_tagCombo->setEditable(true);
    d->m_tagCombo->setInsertPolicy(QComboBox::NoInsert);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(d->m_tagCombo, 100);
    setLayout(layout);
    setFocusProxy(d->m_tagCombo);
    d->m_tagCombo->lineEdit()->setPlaceholderText(i18n("Tag"));

    d->m_tagCombo->setModel(d->m_idFilter.data());
    d->m_idFilter.data()->setSortLocaleAware(true);
    d->m_idFilter.data()->sort(0);

    connect(d->m_tagCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int row)
    {
        const auto idx = d->m_tagCombo->model()->index(row, 0);
        const auto id = idx.data(eMyMoney::Model::IdRole).toString();
        d->addTagWidget(id);
        emit tagsChanged(d->tagIdList());
    }
           );
}

KTagContainer::~KTagContainer()
{
    Q_D(KTagContainer);
    delete d;
}

void KTagContainer::setModel(QAbstractItemModel* model)
{
    Q_D(KTagContainer);
    d->m_idFilter->setSourceModel(model);
}

QComboBox* KTagContainer::tagCombo()
{
    Q_D(KTagContainer);
    return d->m_tagCombo;
}

const QList<QString> KTagContainer::selectedTags()
{
    Q_D(KTagContainer);
    return d->m_idFilter->filterList();
}

void KTagContainer::loadTags(const QList<QString>& idList)
{
    Q_D(KTagContainer);
    // reset filter
    d->m_idFilter->setFilterList(QStringList());
    for (const auto& id : idList) {
        d->addTagWidget(id);
    }
}

void KTagContainer::slotRemoveTagWidget()
{
    Q_D(KTagContainer);
    const auto tagLabel = qobject_cast<KTagLabel *>(sender());
    d->m_idFilter->removeFilter(tagLabel->id());
    int index = d->m_tagLabelList.indexOf(tagLabel);
    d->m_tagLabelList.removeAt(index);
    delete tagLabel;

    emit tagsChanged(d->tagIdList());

    d->m_tagCombo->setCurrentIndex(0);
    d->m_tagCombo->setFocus();
}
