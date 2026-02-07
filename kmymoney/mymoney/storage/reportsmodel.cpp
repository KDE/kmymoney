/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reportsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>
#include <QDebug>
#include <QFont>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


struct ReportsModel::Private
{
    Private()
        : m_useGroups(false)
    {
    }
    bool m_useGroups;
};

ReportsModel::ReportsModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<MyMoneyReport>(parent, QStringLiteral("R"), ReportsModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("ReportsModel"));
}

ReportsModel::~ReportsModel()
{
}

int ReportsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return useGroups() ? 2 : 5;
}

QVariant ReportsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ReportName:
            return i18nc("Reportname", "Name");
            break;
        case Comment:
            return i18nc("Report comment", "Comment");
            break;
        case Favorite:
            return i18nc("Report favorite", "Favorite");
            break;
        case Group:
            return i18nc("Report group", "Group");
            break;
        case Modified:
            return i18nc("Report state", "Modified");
            break;
        }
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

QVariant ReportsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();

    const MyMoneyReport& report = static_cast<TreeItem<MyMoneyReport>*>(index.internalPointer())->constDataRef();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case ReportName:
            return report.name();
        case Comment:
            return report.comment();
        case Favorite:
            return report.isFavorite() ? QStringLiteral("\u2605") : QString();
        case Group:
            return i18n(report.group().toLatin1());
        case Modified:
            return report.isModified() ? QStringLiteral("\u2605") : QString();
        default:
            return QStringLiteral("not yet implemented");
        }
        break;

    case Qt::ForegroundRole:
        return report.isModified() ? QColor(Qt::red) : QVariant();
    case Qt::FontRole: {
        QFont f;
        f.setBold(report.isModified());
        return f;
    }

    case Qt::TextAlignmentRole:
        switch (index.column()) {
        case Favorite:
        case Modified:
            return Qt::AlignCenter;
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
        break;

    case eMyMoney::Model::Roles::IdRole:
        return report.id();
        break;
    }
    return QVariant();
}

bool ReportsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}

int ReportsModel::processItems(Worker* worker)
{
    QModelIndexList indexes = match(index(0, 0), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
    int result = MyMoneyModel::processItems(worker, indexes);
    for (const auto& idx : indexes) {
        auto& report = static_cast<TreeItem<MyMoneyReport>*>(idx.internalPointer())->dataRef();
        if (report.isModified()) {
            report.setModified(false);
            int nCols = columnCount(idx.parent());
            QModelIndex first = index(idx.row(), 0, idx.parent());
            QModelIndex last = index(idx.row(), nCols - 1, idx.parent());

            Q_EMIT dataChanged(first, last, {Qt::DisplayRole, Qt::ForegroundRole, Qt::FontRole});
        }
    }
    return result;
}

bool ReportsModel::useGroups() const
{
    return d->m_useGroups;
}

void ReportsModel::setUseGroups(bool state)
{
    d->m_useGroups = state;
}

Qt::ItemFlags ReportsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return Qt::NoItemFlags;

    if (useGroups()) {
        auto* item = static_cast<TreeItem<MyMoneyReport>*>(index.internalPointer());

        const auto& report = item->dataRef();
        if (report.isGroup()) {
            // enabled (so it expands), but not selectable
            return Qt::ItemIsEnabled;
        }
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ReportsModel::load(const QMap<QString, MyMoneyReport>& reports)
{
    MyMoneyModel<MyMoneyReport>::load(reports);
    setUseGroups(false);
}

void ReportsModel::load(const QList<ReportGroup>& reportGroups)
{
    QElapsedTimer t;
    t.start();

    beginResetModel();

    clearModelItems();

    QMap<QString, TreeItem<MyMoneyReport>*> groupItems;
    int groupNo = 1;

    for (const auto& group : reportGroups) {
        MyMoneyReport groupReport;
        groupReport.setIsGroup();
        groupReport.setName(QString("%1. %2").arg(groupNo++).arg(i18n(group.name().toLatin1().data()))); // store group name
        TreeItem<MyMoneyReport>* groupItem = new TreeItem<MyMoneyReport>(groupReport, m_rootItem);
        m_rootItem->appendChild(groupItem);
        groupItems[group.name()] = groupItem;
        for (const auto& report : group) {
            if (!report.isBuiltIn())
                continue;
            TreeItem<MyMoneyReport>* reportItem = new TreeItem<MyMoneyReport>(report, groupItem);
            groupItem->appendChild(reportItem);

            if (m_idToItemMapper)
                m_idToItemMapper->insert(report.id(), reportItem);
        }
    }
    setDirty(false);
    m_nextId = 0;

    endResetModel();
    Q_EMIT modelLoaded();

    qDebug() << "ReportsModel loaded with" << rowCount() << "groups in" << t.elapsed() << "ms";
    setUseGroups(true);
}
