/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "unavailabletask.h"

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

unavailableTask* unavailableTask::clone() const
{
  return new unavailableTask(m_data);
}

bool unavailableTask::isValid() const
{
  return true;
}
