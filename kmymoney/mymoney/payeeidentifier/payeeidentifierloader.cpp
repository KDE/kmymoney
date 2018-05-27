/*
 * Copyright 2014-2016  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "payeeidentifier/payeeidentifierloader.h"

#include <QDebug>
#include <QAbstractItemDelegate>

#include <KServiceTypeTrader>

payeeIdentifierLoader payeeIdentifierLoader::m_self;

payeeIdentifierLoader::payeeIdentifierLoader()
{
}

payeeIdentifierLoader::~payeeIdentifierLoader()
{
}

/**
 * @todo enable delegates again
 */
QAbstractItemDelegate* payeeIdentifierLoader::createItemDelegate(const QString& payeeIdentifierId, QObject* parent)
{
  /** @todo escape ' in  payeeIdentifierId */
  KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/PayeeIdentifierDelegate"), QString("'%1' ~in [X-KMyMoney-payeeIdentifierIds]").arg(payeeIdentifierId));
  if (!offers.isEmpty()) {
    QString error;
    QAbstractItemDelegate* ptr = offers.at(0)->createInstance<QAbstractItemDelegate>(parent, QVariantList(), &error);
    if (ptr == nullptr) {
      qWarning() << "could not load delegate" << error << payeeIdentifierId;
    }
    return ptr;
  }
  return nullptr;
}

bool payeeIdentifierLoader::hasItemEditDelegate(const QString& payeeIdentifierId)
{
  QAbstractItemDelegate* delegate = createItemDelegate(payeeIdentifierId);
  const bool ret = (delegate != nullptr);
  delete delegate;
  return ret;
}

QStringList payeeIdentifierLoader::availableDelegates()
{
  QStringList list;
  KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/PayeeIdentifierDelegate"));
  foreach (KService::Ptr offer, offers) {
    list.append(offer->property("X-KMyMoney-payeeIdentifierIds", QVariant::StringList).toStringList());
  }
  return list;
}
