#
# This file is part of KMyMoney, A Personal Finance Manager by KDE
# SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
# SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
# SPDX-License-Identifier: GPL-2.0-or-later
#

import logging
from woob.core import Woob
from woob.capabilities.bank import CapBank

LOGGER = logging.getLogger(__name__)


def get_backends():
    w = Woob()

    result = {}
    for instance_name, name, params in sorted(w.backends_config.iter_backends()):
        try:
            module = w.modules_loader.get_or_load_module(name)
        except Exception as e:
            LOGGER.error("Failed to read module %s: %s", name, e)
            continue

        if not module.has_caps(CapBank):
            continue

        result[instance_name] = {'module': name}

    return result

def get_accounts(bname):
    w = Woob()

    w.load_backends(names=[bname])
    backend = w.get_backend(bname)

    results = {}
    for account in backend.iter_accounts():
        results[account.id] = {'name':    account.label,
                               'balance': int(account.balance * 100),
                               'type':    int(account.type),
                              }
    return results

def get_transactions(bname, accid, maximum):
    w = Woob()

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
