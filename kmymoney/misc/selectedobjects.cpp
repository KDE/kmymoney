/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "selectedobjects.h"

SelectedObjects::SelectedObjects()
{
}

void SelectedObjects::addSelection(SelectedObjects::Object_t type, const QString& id)
{
  addSelections(type, QStringList() << id);
}

void SelectedObjects::addSelections(SelectedObjects::Object_t type, const QStringList& ids)
{
  for(const auto& id : qAsConst(ids)) {
    if (!m_selections[type].contains(id)) {
      m_selections[type].append(id);
    }
  }
}

void SelectedObjects::setSelection(SelectedObjects::Object_t type, const QString& id)
{
  setSelection(type, QStringList() << id);
}

void SelectedObjects::setSelection(SelectedObjects::Object_t type, const QStringList& ids)
{
  m_selections[type] = ids;
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

bool SelectedObjects::isEmpty() const
{
  return m_selections.isEmpty();
}
