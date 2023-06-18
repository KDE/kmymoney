/*
 *    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmmsearchwidget.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"

#include "ui_kmmsearchwidget.h"

using namespace Icons;

class KMMSearchWidgetPrivate
{
public:
    KMMSearchWidgetPrivate(KMMSearchWidget* qq)
        : q_ptr(qq)
        , ui(new Ui_KMMSearchWidget)
    {
        ui->setupUi(qq);
        ui->m_closeButton->setIcon(Icons::get(Icon::DialogClose));
    }

    KMMSearchWidget* q_ptr;
    Ui_KMMSearchWidget* ui;
};

KMMSearchWidget::KMMSearchWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new KMMSearchWidgetPrivate(this))
{
    hide();
    lineEdit()->installEventFilter(this);
    connect(closeButton(), &QToolButton::clicked, this, [&] {
        close();
    });
}

QToolButton* KMMSearchWidget::closeButton() const
{
    Q_D(const KMMSearchWidget);
    return d->ui->m_closeButton;
}

QLineEdit* KMMSearchWidget::lineEdit() const
{
    Q_D(const KMMSearchWidget);
    return d->ui->m_searchWidget;
}

QComboBox* KMMSearchWidget::comboBox() const
{
    Q_D(const KMMSearchWidget);
    return d->ui->m_filterBox;
}

void KMMSearchWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    lineEdit()->setFocus();
}

void KMMSearchWidget::close()
{
    lineEdit()->clear();
    hide();
    Q_EMIT closed();
}

bool KMMSearchWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == lineEdit()) {
        if (event->type() == QEvent::KeyPress) {
            const auto kev = static_cast<QKeyEvent*>(event);
            if (kev->modifiers() == Qt::NoModifier && kev->key() == Qt::Key_Escape) {
                close();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
