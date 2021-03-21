/*
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBMESSAGESMODEL_H
#define ONLINEJOBMESSAGESMODEL_H

#include <QAbstractTableModel>

#include "mymoney/onlinejob.h"

class onlineJobMessagesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit onlineJobMessagesModel(QObject* parent = 0);
    QVariant data(const QModelIndex& index, int role) const final override;
    int columnCount(const QModelIndex& parent) const final override;
    int rowCount(const QModelIndex& parent) const final override;
    QModelIndex parent(const QModelIndex& child) const final override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const final override;

public Q_SLOTS:
    void setOnlineJob(const onlineJob& job);

protected:
    onlineJob m_job;
};

#endif // ONLINEJOBMESSAGESMODEL_H
