#
# This file is part of KMyMoney, A Personal Finance Manager by KDE
# Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
# Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from weboob.core import Weboob
from weboob.capabilities.bank import CapBank

def get_protocols():
    w = Weboob()

    return w.repositories.get_all_modules_info(CapBank).keys()

def get_backends():
    w = Weboob()

    result = {}
    for instance_name, name, params in sorted(w.backends_config.iter_backends()):
        module = w.modules_loader.get_or_load_module(name)
        if not module.has_caps(CapBank):
            continue

        result[instance_name] = {'module': name}

    return result

def get_accounts(bname):
    w = Weboob()

    w.load_backends(names=[bname])
    backend = w.get_backend(bname)

    results = {}
    for account in backend.iter_accounts():
        # a unicode bigger than 8 characters used as key of the table make some bugs in the C++ code
        # convert to string before to use it
        results[str(account.id)] = {'name':    account.label,
                               'balance': int(account.balance * 100),
                               'type':    int(account.type),
                              }
    return results

def get_transactions(bname, accid, maximum):
    w = Weboob()

    w.load_backends(names=[bname])
    backend = w.get_backend(bname)

    acc = backend.get_account(accid)
    results = {}
    results['id'] = acc.id
    results['name'] = acc.label
    results['balance'] = int(acc.balance * 100)
    results['type'] = int(acc.type)
    results['transactions'] = []

    try:
        count = int(maximum)
        if count < 1:
            count = 0
    except:
        count = 0
    i = 0
    first = True
    rewriteid = False
    seen = set()
    for tr in backend.iter_history(acc):
        if first:
            if tr.id == u'0' or tr.id == u'':
                rewriteid = True
            first = False
        if rewriteid:
            tr.id = tr.unique_id(seen)
        t = {'id':          tr.id,
             'date':        tr.date.strftime('%Y-%m-%d'),
             'rdate':       tr.rdate.strftime('%Y-%m-%d'),
             'type':        int(tr.type),
             'raw':         tr.raw,
             'category':    tr.category,
             'label':       tr.label,
             'amount':      int(tr.amount * 100),
        }
        results['transactions'].append(t)
        i += 1
        if count != 0 and i >= count:
            break

    return results
