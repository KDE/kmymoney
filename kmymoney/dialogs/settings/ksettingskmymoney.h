/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSKMYMONEY_H
#define KSETTINGSKMYMONEY_H

#include <KConfigDialog>

/**
 * @brief The general settings dialog
 */
class KSettingsKMyMoney : public KConfigDialog
{
public:
    explicit KSettingsKMyMoney(QWidget *parent, const QString &name, KCoreConfigSkeleton *config);

Q_SIGNALS:
    void pluginsChanged();

private Q_SLOTS:
    void slotPluginsChanged(bool changed);
    void slotEnableFinishButton(bool enable);
};


#endif /* KSETTINGSKMYMONEY_H */
