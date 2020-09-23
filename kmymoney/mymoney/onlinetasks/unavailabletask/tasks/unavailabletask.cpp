/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright 2014       Christian Dávid <christian-david@web.de>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "unavailabletask.h"

#include <QSet>

#include <KLocalizedString>

unavailableTask::unavailableTask(const QDomElement& element)
    : m_data(element)
{
}

QString unavailableTask::jobTypeName() const
{
  return i18n("Could not load responsible plugin to view this task.");
}

QString unavailableTask::responsibleAccount() const
{
  return QString();
}

unavailableTask* unavailableTask::createFromXml(const QDomElement& element) const
{
  return new unavailableTask(element);
}

void unavailableTask::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent = m_data;
}

bool unavailableTask::hasReferenceTo(const QString& id) const
{
  Q_UNUSED(id);
  return false;
}

QSet<QString> unavailableTask::referencedObjects() const
{
  return {};
}

unavailableTask* unavailableTask::clone() const
{
  return new unavailableTask(m_data);
}

bool unavailableTask::isValid() const
{
  return true;
}
