/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
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
