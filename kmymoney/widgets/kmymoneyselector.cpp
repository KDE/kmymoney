/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyselector_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QRegularExpression>
#include <QStyle>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "widgetenums.h"

using namespace eWidgets;

KMyMoneySelector::KMyMoneySelector(QWidget *parent, Qt::WindowFlags flags) :
    QWidget(parent, flags),
    d_ptr(new KMyMoneySelectorPrivate(this))
{
    Q_D(KMyMoneySelector);
    d->init();
}

KMyMoneySelector::KMyMoneySelector(KMyMoneySelectorPrivate &dd, QWidget* parent, Qt::WindowFlags flags) :
    QWidget(parent, flags),
    d_ptr(&dd)
{
    Q_D(KMyMoneySelector);
    d->init();
}

KMyMoneySelector::~KMyMoneySelector()
{
    Q_D(KMyMoneySelector);
    delete d;
}

void KMyMoneySelector::clear()
{
    Q_D(KMyMoneySelector);
    d->m_treeWidget->clear();
}

void KMyMoneySelector::setSelectable(QTreeWidgetItem *item, bool selectable)
{
    if (selectable) {
        item->setFlags(item->flags() | Qt::ItemIsSelectable);
    } else {
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }
}

void KMyMoneySelector::slotSelectAllItems()
{
    selectAllItems(true);
}

void KMyMoneySelector::slotDeselectAllItems()
{
    selectAllItems(false);
}

void KMyMoneySelector::setSelectionMode(const QTreeWidget::SelectionMode mode)
{
    Q_D(KMyMoneySelector);
    if (d->m_selMode != mode) {
        d->m_selMode = mode;
        clear();

        // make sure, it's either Multi or Single
        if (mode != QTreeWidget::MultiSelection) {
            d->m_selMode = QTreeWidget::SingleSelection;
            connect(d->m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &KMyMoneySelector::stateChanged);
            connect(d->m_treeWidget, &QTreeWidget::itemActivated, this, &KMyMoneySelector::slotItemSelected);
            connect(d->m_treeWidget, &QTreeWidget::itemClicked, this, &KMyMoneySelector::slotItemSelected);
        } else {
            disconnect(d->m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &KMyMoneySelector::stateChanged);
            disconnect(d->m_treeWidget, &QTreeWidget::itemActivated, this, &KMyMoneySelector::slotItemSelected);
            disconnect(d->m_treeWidget, &QTreeWidget::itemClicked, this, &KMyMoneySelector::slotItemSelected);
        }
    }
    QWidget::update();
}

QTreeWidget::SelectionMode KMyMoneySelector::selectionMode() const
{
    Q_D(const KMyMoneySelector);
    return d->m_selMode;
}

void KMyMoneySelector::slotItemSelected(QTreeWidgetItem *item)
{
    Q_D(KMyMoneySelector);
    if (d->m_selMode == QTreeWidget::SingleSelection) {
        if (item && item->flags().testFlag(Qt::ItemIsSelectable)) {
            Q_EMIT itemSelected(item->data(0, (int)Selector::Role::Id).toString());
        }
    }
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name, QTreeWidgetItem* after, const QString& key, const QString& id)
{
    Q_D(KMyMoneySelector);
    QTreeWidgetItem* item = new QTreeWidgetItem(d->m_treeWidget, after);

    item->setText(0, name);
    item->setData(0, (int)Selector::Role::Key, key);
    item->setData(0, (int)Selector::Role::Id, id);
    item->setText(1, key); // hidden, but used for sorting
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);

    if (id.isEmpty()) {
        QFont font = item->font(0);
        font.setBold(true);
        item->setFont(0, font);
        setSelectable(item, false);
    }
    item->setExpanded(true);
    return item;
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name, QTreeWidgetItem* after, const QString& key)
{
    return newItem(name, after, key, QString());
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name, QTreeWidgetItem* after)
{
    return newItem(name, after, QString(), QString());
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name, const QString& key, const QString& id)
{
    return newItem(name, 0, key, id);
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name, const QString& key)
{
    return newItem(name, 0, key, QString());
}

QTreeWidgetItem* KMyMoneySelector::newItem(const QString& name)
{
    return newItem(name, 0, QString(), QString());
}

QTreeWidgetItem* KMyMoneySelector::newTopItem(const QString& name, const QString& key, const QString& id)
{
    Q_D(KMyMoneySelector);
    QTreeWidgetItem* item = new QTreeWidgetItem(d->m_treeWidget);

    item->setText(0, name);
    item->setData(0, (int)Selector::Role::Key, key);
    item->setData(0, (int)Selector::Role::Id, id);
    item->setText(1, key); // hidden, but used for sorting
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);

    if (d->m_selMode == QTreeWidget::MultiSelection) {
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
    }
    return item;
}

QTreeWidgetItem* KMyMoneySelector::newItem(QTreeWidgetItem* parent, const QString& name, const QString& key, const QString& id)
{
    Q_D(KMyMoneySelector);
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);

    item->setText(0, name);
    item->setData(0, (int)Selector::Role::Key, key);
    item->setData(0, (int)Selector::Role::Id, id);
    item->setText(1, key); // hidden, but used for sorting
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);

    if (d->m_selMode == QTreeWidget::MultiSelection) {
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
    }
    return item;
}

void KMyMoneySelector::protectItem(const QString& itemId, const bool protect)
{
    Q_D(KMyMoneySelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* it_v;

    // scan items
    while ((it_v = *it) != 0) {
        if (it_v->data(0, (int)Selector::Role::Id).toString() == itemId) {
            setSelectable(it_v, !protect);
            break;
        }
        ++it;
    }
}

QTreeWidgetItem* KMyMoneySelector::item(const QString& id) const
{
    Q_D(const KMyMoneySelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* it_v;

    while ((it_v = *it) != 0) {
        if (it_v->data(0, (int)Selector::Role::Id).toString() == id)
            break;
        ++it;
    }
    return it_v;
}

bool KMyMoneySelector::allItemsSelected() const
{
    Q_D(const KMyMoneySelector);
    QTreeWidgetItem* rootItem = d->m_treeWidget->invisibleRootItem();

    if (d->m_selMode == QTreeWidget::SingleSelection)
        return false;

    for (auto i = 0; i < rootItem->childCount(); ++i) {
        QTreeWidgetItem* item = rootItem->child(i);
        if (item->flags().testFlag(Qt::ItemIsUserCheckable)) {
            if (!(item->checkState(0) == Qt::Checked && allItemsSelected(item)))
                return false;
        } else {
            if (!allItemsSelected(item))
                return false;
        }
    }
    return true;
}

bool KMyMoneySelector::allItemsSelected(const QTreeWidgetItem *item) const
{
    for (auto i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (child->flags().testFlag(Qt::ItemIsUserCheckable)) {
            if (!(child->checkState(0) == Qt::Checked && allItemsSelected(child)))
                return false;
        }
    }
    return true;
}

void KMyMoneySelector::removeItem(const QString& id)
{
    Q_D(KMyMoneySelector);
    QTreeWidgetItem* it_v;
    QTreeWidgetItemIterator it(d->m_treeWidget);

    while ((it_v = *it) != 0) {
        if (id == it_v->data(0, (int)Selector::Role::Id).toString()) {
            if (it_v->childCount() > 0) {
                setSelectable(it_v, false);
            } else {
                delete it_v;
            }
        }
        it++;
    }

    // get rid of top items that just lost the last children (e.g. Favorites)
    it = QTreeWidgetItemIterator(d->m_treeWidget, QTreeWidgetItemIterator::NotSelectable);
    while ((it_v = *it) != 0) {
        if (it_v->childCount() == 0)
            delete it_v;
        it++;
    }
}


void KMyMoneySelector::selectAllItems(const bool state)
{
    Q_D(KMyMoneySelector);
    selectAllSubItems(d->m_treeWidget->invisibleRootItem(), state);
    Q_EMIT stateChanged();
}

void KMyMoneySelector::selectItems(const QStringList& itemList, const bool state)
{
    Q_D(KMyMoneySelector);
    selectSubItems(d->m_treeWidget->invisibleRootItem(), itemList, state);
    Q_EMIT stateChanged();
}

void KMyMoneySelector::selectSubItems(QTreeWidgetItem* item, const QStringList& itemList, const bool state)
{
    for (auto i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (child->flags().testFlag(Qt::ItemIsUserCheckable) && itemList.contains(child->data(0, (int)Selector::Role::Id).toString())) {
            child->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
        }
        selectSubItems(child, itemList, state);
    }
    Q_EMIT stateChanged();
}

void KMyMoneySelector::selectAllSubItems(QTreeWidgetItem* item, const bool state)
{
    for (auto i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (child->flags().testFlag(Qt::ItemIsUserCheckable)) {
            child->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
        }
        selectAllSubItems(child, state);
    }
    Q_EMIT stateChanged();
}

void KMyMoneySelector::selectedItems(QStringList& list) const
{
    Q_D(const KMyMoneySelector);
    list.clear();
    if (d->m_selMode == QTreeWidget::SingleSelection) {
        QTreeWidgetItem* it_c = d->m_treeWidget->currentItem();
        if (it_c != 0 && it_c->isSelected())
            list << it_c->data(0, (int)Selector::Role::Id).toString();
    } else {
        QTreeWidgetItem* rootItem = d->m_treeWidget->invisibleRootItem();
        for (auto i = 0; i < rootItem->childCount(); ++i) {
            QTreeWidgetItem* child = rootItem->child(i);
            if (child->flags().testFlag(Qt::ItemIsUserCheckable)) {
                if (child->checkState(0) == Qt::Checked)
                    list << child->data(0, (int)Selector::Role::Id).toString();
            }
            selectedItems(list, child);
        }
    }
}

void KMyMoneySelector::selectedItems(QStringList& list, QTreeWidgetItem* item) const
{
    for (auto i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        if (child->flags().testFlag(Qt::ItemIsUserCheckable)) {
            if (child->checkState(0) == Qt::Checked)
                list << child->data(0, (int)Selector::Role::Id).toString();
        }
        selectedItems(list, child);
    }
}

void KMyMoneySelector::itemList(QStringList& list) const
{
    Q_D(const KMyMoneySelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* it_v;

    while ((it_v = *it) != 0) {
        list << it_v->data(0, (int)Selector::Role::Id).toString();
        it++;
    }
}

void KMyMoneySelector::setSelected(const QString& id, const bool state)
{
    Q_D(const KMyMoneySelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* item;
    QTreeWidgetItem* it_visible = 0;

    while ((item = *it) != 0) {
        if (item->data(0, (int)Selector::Role::Id).toString() == id) {
            if (item->flags().testFlag(Qt::ItemIsUserCheckable)) {
                item->setCheckState(0, state ? Qt::Checked : Qt::Unchecked);
            }
            d->m_treeWidget->setCurrentItem(item);
            if (!it_visible)
                it_visible = item;
        }
        it++;
    }

    // make sure the first one found is visible
    if (it_visible)
        d->m_treeWidget->scrollToItem(it_visible);
}

QTreeWidget* KMyMoneySelector::listView() const
{
    Q_D(const KMyMoneySelector);
    return d->m_treeWidget;
}

int KMyMoneySelector::slotMakeCompletion(const QString& _txt)
{
    QString txt(QRegularExpression::escape(_txt));
    if (KMyMoneySettings::stringMatchFromStart() && QLatin1String(this->metaObject()->className()) == QLatin1String("KMyMoneySelector"))
        txt.prepend('^');
    return slotMakeCompletion(QRegularExpression(txt, QRegularExpression::CaseInsensitiveOption));
}

bool KMyMoneySelector::match(const QRegularExpression& exp, QTreeWidgetItem* item) const
{
    return exp.match(item->text(0)).hasMatch();
}

int KMyMoneySelector::slotMakeCompletion(const QRegularExpression& _exp)
{
    Q_D(KMyMoneySelector);
    auto exp(_exp);
    auto pattern = exp.pattern();
    auto replacement = QStringLiteral(".*:");
    if (!KMyMoneySettings::stringMatchFromStart() || QLatin1String(this->metaObject()->className()) != QLatin1String("KMyMoneySelector")) {
        replacement.append(QLatin1String(".*"));
    }
    pattern.replace(QLatin1String(":"), replacement);
    exp.setPattern(pattern);

    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);

    QTreeWidgetItem* it_v;

    // The logic used here seems to be awkward. The problem is, that
    // QListViewItem::setVisible works recursively on all it's children
    // and grand-children.
    //
    // The way out of this is as follows: Make all items visible.
    // Then go through the list again and perform the checks.
    // If an item does not have any children (last leaf in the tree view)
    // perform the check. Then check recursively on the parent of this
    // leaf that it has no visible children. If that is the case, make the
    // parent invisible and continue this check with it's parent.
    while ((it_v = *it) != 0) {
        it_v->setHidden(false);
        ++it;
    }

    QTreeWidgetItem* firstMatch = 0;

    if (!exp.pattern().isEmpty()) {
        it = QTreeWidgetItemIterator(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
        while ((it_v = *it) != 0) {
            if (it_v->childCount() == 0) {
                if (!match(exp, it_v)) {
                    // this is a node which does not contain the
                    // text and does not have children. So we can
                    // safely hide it. Then we check, if the parent
                    // has more children which are still visible. If
                    // none are found, the parent node is hidden also. We
                    // continue until the top of the tree or until we
                    // find a node that still has visible children.
                    bool hide = true;
                    while (hide) {
                        it_v->setHidden(true);
                        it_v = it_v->parent();
                        if (it_v && (it_v->flags() & Qt::ItemIsSelectable)) {
                            hide = !match(exp, it_v);
                            for (auto i = 0; hide && i < it_v->childCount(); ++i) {
                                if (!it_v->child(i)->isHidden())
                                    hide = false;
                            }
                        } else
                            hide = false;
                    }
                } else if (!firstMatch) {
                    firstMatch = it_v;
                }
                ++it;

            } else if (match(exp, it_v)) {
                if (!firstMatch) {
                    firstMatch = it_v;
                }
                // a node with children contains the text. We want
                // to display all child nodes in this case, so we need
                // to advance the iterator to the next sibling of the
                // current node. This could well be the sibling of a
                // parent or grandparent node.
                QTreeWidgetItem* curr = it_v;
                QTreeWidgetItem* item;
                while ((item = curr->treeWidget()->itemBelow(curr)) == 0) {
                    curr = curr->parent();
                    if (curr == 0)
                        break;
                    if (match(exp, curr))
                        firstMatch = curr;
                }
                do {
                    ++it;
                } while (*it && *it != item);
            } else {
                // It's a node with children that does not match. We don't
                // change it's status here.
                ++it;
            }
        }
    }

    // make the first match the one that is selected
    // if we have no match, make sure none is selected
    if (d->m_selMode == QTreeWidget::SingleSelection) {
        if (firstMatch) {
            d->m_treeWidget->setCurrentItem(firstMatch);
            d->m_treeWidget->scrollToItem(firstMatch);
        } else
            d->m_treeWidget->clearSelection();
    }

    // Get the number of visible nodes for the return code
    auto cnt = 0;

    it = QTreeWidgetItemIterator(d->m_treeWidget, QTreeWidgetItemIterator::Selectable | QTreeWidgetItemIterator::NotHidden);
    while ((it_v = *it) != 0) {
        cnt++;
        it++;
    }
    return cnt;
}

bool KMyMoneySelector::contains(const QString& txt) const
{
    Q_D(const KMyMoneySelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* it_v;
    while ((it_v = *it) != 0) {
        if (it_v->text(0) == txt) {
            return true;
        }
        it++;
    }
    return false;
}

void KMyMoneySelector::slotItemPressed(QTreeWidgetItem* item, int /* col */)
{
    Q_D(KMyMoneySelector);
    if (QApplication::mouseButtons() != Qt::RightButton)
        return;

    if (item->flags().testFlag(Qt::ItemIsUserCheckable)) {
        QStyleOptionButton opt;
        opt.rect = d->m_treeWidget->visualItemRect(item);
        QRect rect = d->m_treeWidget->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, d->m_treeWidget);
        if (rect.contains(d->m_treeWidget->mapFromGlobal(QCursor::pos()))) {
            // we get down here, if we have a right click onto the checkbox
            item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
            selectAllSubItems(item, item->checkState(0) == Qt::Checked);
        }
    }
}

QStringList KMyMoneySelector::selectedItems() const
{
    QStringList list;
    selectedItems(list);
    return list;
}

QStringList KMyMoneySelector::itemList() const
{
    QStringList list;
    itemList(list);
    return list;
}
