#
# This file is part of KMyMoney, A Personal Finance Manager by KDE
# SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
# SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
# SPDX-FileCopyrightText: 2022 Dawid Wróbel <me@dawidwrobel.com>
# SPDX-License-Identifier: GPL-2.0-or-later
#

import logging
import logging.config
import sys

from woob.core import Woob
from woob.capabilities.bank import CapBank
from woob.exceptions import AppValidation, AppValidationCancelled, AppValidationExpired


# based on https://stackoverflow.com/a/53257669/665932
class _ExcludeErrorsFilter(logging.Filter):
    def filter(self, record):
        """Only lets through log messages with log level below ERROR ."""
        return record.levelno < logging.ERROR


config = {
    'version': 1,
    'filters': {
        'exclude_errors': {
            '()': _ExcludeErrorsFilter
        }
    },
    'handlers': {
        'console_stderr': {
            # Sends log messages with log level ERROR or higher to stderr
            'class': 'logging.StreamHandler',
            'level': 'ERROR',
            'stream': sys.stderr
        },
        'console_stdout': {
            # Sends log messages with log level lower than ERROR to stdout
            'class': 'logging.StreamHandler',
            'level': 'DEBUG',
            'filters': ['exclude_errors'],
            'stream': sys.stdout
        },
    },
    'root': {
        # In general, this should be kept at 'NOTSET'.
        # Otherwise it would interfere with the log levels set for each handler.
        'level': 'NOTSET',
        'handlers': ['console_stderr', 'console_stdout']
    },
}

logging.config.dictConfig(config)
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
    accounts = {}
    try:
        accounts = backend.iter_accounts()
    except AppValidation:
        # hacky copy from Woob's mfa.py do_double_authentication()
        # See https://gitlab.com/woob/woob/-/issues/597
        config_key = 'resume'
        if config_key in backend.config:
            for config_key, handle_method in backend.browser.AUTHENTICATION_METHODS.items():
                try:
                    handle_method()
                except AppValidationCancelled:
                    raise
                except AppValidationExpired:
                    raise

            accounts = backend.iter_accounts()
    for account in accounts:
        results[account.id] = {'name': account.label,
                               'balance': int(account.balance * 100),
                               'type': int(account.type),
                               }
    return results


def get_transactions(bname, accid, maximum):
    w = Woob()

    w.load_backends(names=[bname])
    backend = w.get_backend(bname)

    try:
        acc = backend.get_account(accid)
    except AppValidation:
        config_key = 'resume'
        if config_key in backend.config:
            for config_key, handle_method in backend.browser.AUTHENTICATION_METHODS.items():
                try:
                    handle_method()
                except AppValidationCancelled:
                    raise
                except AppValidationExpired:
                    raise

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
