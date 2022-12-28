/*
    SPDX-FileCopyrightText: 2009-2016 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
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
        , m_skipSelection(true)
        , m_selectAfterFocusOut(false)
    {
    }

    void addTagWidget(int row)
    {
        const auto idx = m_tagCombo->model()->index(row, 0);
        const auto id = idx.data(eMyMoney::Model::IdRole).toString();
        addTagWidget(id);
    }

    void addTagWidget(const QString& id)
    {
        Q_Q(KTagContainer);

        m_skipSelection = true;
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

        Q_EMIT q->tagsChanged(tagIdList());
    }

    QStringList tagIdList() const
    {
        QStringList tags;
        for(const auto tag : qAsConst(m_tagLabelList)) {
            tags << tag->id();
        }
        return tags;
    }

    KTagContainer* q_ptr;
    QComboBox* m_tagCombo;
    QScopedPointer<IdFilter> m_idFilter;
    QList<KTagLabel*> m_tagLabelList;
    bool m_skipSelection;
    bool m_selectAfterFocusOut;
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
    d->m_tagCombo->lineEdit()->setPlaceholderText(i18nc("@info:placeholder tag combo box", "Tag"));

    d->m_tagCombo->setModel(d->m_idFilter.data());
    d->m_idFilter.data()->setSortLocaleAware(true);
    d->m_idFilter.data()->sort(0);

    connect(d->m_tagCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int row) {
        Q_D(KTagContainer);
        if (!d->m_skipSelection) {
            d->addTagWidget(row);
        }
    });

    connect(d->m_tagCombo, QOverload<int>::of(&QComboBox::activated), [=](int row) {
        Q_D(KTagContainer);
        if (d->m_selectAfterFocusOut) {
            d->addTagWidget(row);
            d->m_selectAfterFocusOut = false;
        }
    });

    d->m_tagCombo->installEventFilter(this);
    d->m_tagCombo->view()->installEventFilter(this);
}

KTagContainer::~KTagContainer()
{
    Q_D(KTagContainer);
    delete d;
}

bool KTagContainer::eventFilter(QObject* o, QEvent* e)
{
    Q_D(KTagContainer);
    if (o == d->m_tagCombo->view()) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* kev = static_cast<QKeyEvent*>(e);
            switch (kev->key()) {
            case Qt::Key_Escape:
            case Qt::Key_F4:
                d->m_skipSelection = true;
                break;
            default:
                break;
            }
        } else if (e->type() == QEvent::Show) {
            d->m_skipSelection = false;
        }

    } else if (o == d->m_tagCombo) {
        if (e->type() == QEvent::FocusOut) {
            const auto row = d->m_tagCombo->currentIndex();
            if (row > 0) {
                d->addTagWidget(row);
            } else {
                d->m_selectAfterFocusOut = true;
            }
        }
    }
    return false;
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

    Q_EMIT tagsChanged(d->tagIdList());

    d->m_tagCombo->setCurrentIndex(0);
    d->m_tagCombo->setFocus();
}
