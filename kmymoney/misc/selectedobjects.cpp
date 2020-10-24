/*
 * Copyright (C) 2020      Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
