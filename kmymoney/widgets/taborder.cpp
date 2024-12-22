/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "taborder.h"

// ----------------------------------------------------------------------------
// QT Includes

// #include <QMetaMethod>
#include <QDebug>
#include <QGroupBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "widgethintframe.h"

class TabOrderPrivate
{
public:
    void setupTabOrder()
    {
        const auto widgetCount = m_tabOrder.count();
        if (m_topLevelWidget && widgetCount > 0) {
            auto prev = m_topLevelWidget->findChild<QWidget*>(m_tabOrder.at(0));
            for (int i = 1; (prev != nullptr) && (i < widgetCount); ++i) {
                const auto next = m_topLevelWidget->findChild<QWidget*>(m_tabOrder.at(i));
                if (next) {
                    m_topLevelWidget->setTabOrder(prev, next);
                    prev = next;
                } else {
                    qDebug() << m_tabOrder.at(i) << "not found :(";
                }
            }
        }
    }

    QWidget* m_topLevelWidget{nullptr};
    QString m_name;
    QStringList m_tabOrder;
    QStringList m_defaultTabOrder;
};

TabOrder::TabOrder(const QString& name, const QStringList& defaultTabOrder)
    : d(new TabOrderPrivate)
{
    d->m_name = name;
    d->m_defaultTabOrder = defaultTabOrder;
}

void TabOrder::setWidget(QWidget* topLevelWidget)
{
    if (topLevelWidget != nullptr) {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group(QLatin1String("TabOrder"));
        d->m_tabOrder = grp.readEntry(d->m_name, d->m_defaultTabOrder);
        // make the settings available for the taborder editor
        topLevelWidget->setProperty("kmm_defaulttaborder", d->m_defaultTabOrder);
        topLevelWidget->setProperty("kmm_currenttaborder", d->m_tabOrder);
        d->m_topLevelWidget = topLevelWidget;
        d->setupTabOrder();
    }
}

void TabOrder::setTabOrder(const QStringList& tabOrder)
{
    if (d->m_topLevelWidget) {
        d->m_tabOrder = tabOrder;
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group(QLatin1String("TabOrder"));
        grp.writeEntry(d->m_name, tabOrder);
        d->m_topLevelWidget->setProperty("kmm_currenttaborder", d->m_tabOrder);
        d->setupTabOrder();
    }
}

QWidget* TabOrder::initialFocusWidget(WidgetHintFrameCollection* frameCollection) const
{
    QWidget* focusWidget = nullptr;
    if (!d->m_tabOrder.isEmpty()) {
        if (frameCollection != nullptr) {
            for (const auto& widgetName : d->m_tabOrder) {
                const auto w = d->m_topLevelWidget->findChild<QWidget*>(widgetName);
                if (w && w->isVisibleTo(d->m_topLevelWidget) && frameCollection->isFrameVisible(w)) {
                    focusWidget = w;
                    break;
                }
            }
        }

        if (!focusWidget) {
            for (const auto& widgetName : d->m_tabOrder) {
                const auto w = d->m_topLevelWidget->findChild<QWidget*>(widgetName);
                if (w && w->isVisibleTo(d->m_topLevelWidget)) {
                    focusWidget = w;
                    break;
                }
            }
        }
        if (focusWidget) {
            QMetaObject::invokeMethod(focusWidget, "setFocus", Qt::QueuedConnection);
        }
    } else if (focusWidget) {
        focusWidget = focusWidget->focusWidget();
    }
    return focusWidget;
}

QWidget* TabOrder::tabFocusHelper(bool next, QWidget* focusWidget)
{
    if (d->m_tabOrder.isEmpty())
        return nullptr;

    if (focusWidget == nullptr) {
        focusWidget = d->m_topLevelWidget->focusWidget();
    }

    enum firstOrLastVisible {
        FirstVisible,
        LastVisible,
    };

    auto findFirstOrLastVisible = [&](firstOrLastVisible type) {
        const int ofs = (type == FirstVisible) ? 1 : -1;
        int idx = (type == FirstVisible) ? 0 : d->m_tabOrder.count() - 1;
        for (; idx >= 0 && idx < d->m_tabOrder.count(); idx += ofs) {
            auto w = d->m_topLevelWidget->findChild<QWidget*>(d->m_tabOrder.at(idx));
            // in case of embedded transaction editors, we may search
            // for a widget that is known to the parent
            if (!w) {
                auto parent = d->m_topLevelWidget->parentWidget();
                while (true) {
                    w = parent->findChild<QWidget*>(d->m_tabOrder.at(idx));
                    if (!w && qobject_cast<QGroupBox*>(parent)) {
                        parent = parent->parentWidget();
                        continue;
                    }
                    break;
                }
            }
            if (w && w->isVisible() && w->isEnabled()) {
                return w;
            }
        }
        return static_cast<QWidget*>(nullptr);
    };

    auto selectWidget = [&](QWidget* w) {
        if (w) {
            // if we point to a constructed widget (e.g. ButtonBox) we
            // need to select the last widget if going backward
            if (!next && !w->findChildren<QWidget*>().isEmpty()) {
                auto parent = w;
                while (w->nextInFocusChain()->parentWidget() == parent) {
                    w = w->nextInFocusChain();
                }
            }
        }
        return w;
    };

    auto adjustToContainer = [&](const char* containerClass, const char* widgetClass) {
        if (!next && focusWidget->qt_metacast(widgetClass) && focusWidget->parentWidget()->qt_metacast(containerClass)) {
            if (focusWidget->previousInFocusChain() == focusWidget->parentWidget()) {
                focusWidget = focusWidget->parentWidget();
            }
        }
    };

    // In case of a CreditDebitEdit widget and we leave from the left backwards,
    // we need to adjust the widget to point to the container widget.
    adjustToContainer("CreditDebitEdit", "AmountEdit");
    // In case of a QDialogButtonBox widget and we leave from the left backwards,
    // we need to adjust the widget to point to the container widget.
    // adjustToContainer("QDialogButtonBox", "QPushButton");

    if (!next && (findFirstOrLastVisible(FirstVisible) == focusWidget)) {
        return selectWidget(findFirstOrLastVisible(LastVisible));

    } else if (next && (findFirstOrLastVisible(LastVisible) == focusWidget)) {
        return selectWidget(findFirstOrLastVisible(FirstVisible));
    }
    return nullptr;
}

#if 0
static QString widgetName(QWidget* w)
{
    return w->objectName().isEmpty() ? w->metaObject()->className() : w->objectName();
}

static void dumpFocusChain(QWidget* w, QWidget* end, int additionalTabstops = 0, bool forward = true)
{
    QString txt;
    int loopValid = 80;
    int trailing = -1;
    do {
        const auto policy = w->focusPolicy();
        if (policy == Qt::TabFocus || policy == Qt::StrongFocus ) {
            if (!txt.isEmpty()) {
                txt += forward ? QLatin1String(" -> ") : QLatin1String(" <- ");
            }
            txt += widgetName(w);
            --loopValid;
            w = forward ? w->nextInFocusChain() : w->previousInFocusChain();
            if (w == end && (trailing < 0)) {
                trailing = additionalTabstops+1;
                loopValid = trailing;
            }
        }
        --trailing;
    } while ((loopValid > 0) && (trailing != 0));

    if (end) {
        txt += forward ? QLatin1String(" -> ") : QLatin1String(" <- ");
        txt += widgetName(w);
    }

    qDebug() << txt;
}
#endif
