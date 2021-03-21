/*
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktreewidgetfilterlinewidget.h"

#include <QLabel>

#include <KLocalizedString>

KTreeWidgetFilterLineWidget::KTreeWidgetFilterLineWidget(QWidget* parent, QTreeWidget* treeWidget)
    : KTreeWidgetSearchLineWidget(parent, treeWidget)
{
}

void KTreeWidgetFilterLineWidget::createWidgets()
{
    KTreeWidgetSearchLineWidget::createWidgets();

    // The layout pointer is stored in the private class, so we do not have access to it directly
    // => use findChild()
    QLabel* label = findChild<QLabel*>();
    if (!label)
        return;

    label->setText(i18nc("Filter widget label", "Fi&lter:"));
}
