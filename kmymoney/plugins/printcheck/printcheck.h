/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifndef PRINTCHECK_H
#define PRINTCHECK_H

#include <memory>

#include "kmymoneyplugin.h"
#include "selectedtransactions.h"

class KPluginInfo;
class QPrinter;
class QObject;
class KMMPrintCheckPlugin: public KMyMoneyPlugin::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.printcheck" FILE "kmm_printcheck.json")

public:
  KMMPrintCheckPlugin();
  ~KMMPrintCheckPlugin();

private:
  void readCheckTemplate();

  bool canBePrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction) const;
  void markAsPrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction);

protected Q_SLOTS:
  void slotPrintCheck();
  void slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);
  // the plugin loader plugs in a plugin
  void slotPlug(KPluginInfo*);
  // the plugin's configurations has changed
  void slotUpdateConfig();

private:
  struct Private;
  std::unique_ptr<Private> d;
  QPrinter                *m_currentPrinter;
};

#endif // PRINTCHECK_H

