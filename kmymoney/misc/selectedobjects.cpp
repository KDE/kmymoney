/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectedobjects.h"

SelectedObjects::SelectedObjects()
{
}

void SelectedObjects::addSelection(SelectedObjects::Object_t type, const QString& id)
{
    addSelections(type, QStringList(id));
}

void SelectedObjects::addSelections(SelectedObjects::Object_t type, const QStringList& ids)
{
    for(const auto& id : qAsConst(ids)) {
        if (!id.isEmpty()) {
            if (!m_selections[type].contains(id)) {
                m_selections[type].append(id);
            }
        }
    }
}

void SelectedObjects::removeSelection(SelectedObjects::Object_t type, const QString& id)
{
    m_selections[type].removeAll(id);
    if (m_selections[type].isEmpty()) {
        m_selections.remove(type);
    }
}

void SelectedObjects::setSelection(SelectedObjects::Object_t type, const QString& id)
{
    setSelection(type, QStringList(id));
}

void SelectedObjects::setSelection(SelectedObjects::Object_t type, const QStringList& ids)
{
    if (ids.isEmpty()) {
        m_selections.remove(type);
    } else {
        m_selections[type] = ids;
    }
}

void SelectedObjects::clearSelections(SelectedObjects::Object_t type)
{
    m_selections.remove(type);
}

void SelectedObjects::clearSelections()
{
    m_selections.clear();
}

int SelectedObjects::count(SelectedObjects::Object_t type) const
{
    return m_selections[type].count();
}

bool SelectedObjects::isEmpty(SelectedObjects::Object_t type) const
{
    return m_selections[type].isEmpty();
}

QStringList SelectedObjects::selection(SelectedObjects::Object_t type) const
{
    return m_selections[type];
}

QString SelectedObjects::firstSelection(SelectedObjects::Object_t type) const
{
    if (!m_selections[type].isEmpty()) {
        return m_selections[type].first();
    }
    return {};
}

bool SelectedObjects::isEmpty() const
{
    return m_selections.isEmpty();
}

bool SelectedObjects::operator!=(const SelectedObjects& right) const
{
    return m_selections != right.m_selections;
}
