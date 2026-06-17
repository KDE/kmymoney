#

# SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
# SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
# SPDX-FileCopyrightText: 2022-2026 Dawid Wróbel <me@dawidwrobel.com>
# SPDX-License-Identifier: GPL-2.0-or-later
#

import datetime
import logging
import logging.config
import os
import sys

from woob.tools.storage import StandardStorage
from woob.tools.application.base import Woob
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

def _get_woob():
    w = Woob(storage=StandardStorage(
        os.path.expanduser('~/.config/woob/bank.storage')
    ))
    return w


def get_backends():
    w = _get_woob()

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
    w = _get_woob()
    w.load_backends(names=[bname])
    backend = w.get_backend(bname)
    results = {}
    try:
        accounts = backend.iter_accounts()
    except AppValidation:
        backend.config['resume'].set('1')
        accounts = backend.iter_accounts()
    except AppValidationCancelled:
        LOGGER.warning("bank: 2FA cancelled by user")
        return {}
    except AppValidationExpired:
        LOGGER.warning("bank: 2FA timed out")
        return {}

    count = 0
    for account in accounts:
        count += 1
        results[account.id] = {
            'name': account.label,
            'number': account.number,
            'balance': int(account.balance * 100),
            'type': int(account.type),
            'currency': account.currency,
        }
    backend.dump_state()
    LOGGER.debug("bank: got %d accounts", count)
    return results


def get_transactions(bname, accid, end_date=None):
    w = _get_woob()
    w.load_backends(names=[bname])
    backend = w.get_backend(bname)

    try:
        acc = backend.get_account(accid)
    except AppValidation:
        backend.config['resume'].set('1')
        acc = backend.get_account(accid)

    results = {'id': acc.id, 'name': acc.label,
               'balance': int(acc.balance * 100), 'type': int(acc.type),
               'transactions': []}

    if end_date is not None and end_date != '':
        try:
            end_date = datetime.date.fromisoformat(end_date)
        except (ValueError, TypeError):
            LOGGER.warning("bank: invalid end_date '%s', falling back to fetching all history", end_date)
            end_date = None

    i = 0
    first = True
    rewriteid = False
    seen = set()
    for tr in backend.iter_history(acc):
        if first:
            if tr.id == '0' or tr.id == '':
                rewriteid = True
            first = False
        if rewriteid:
            tr.id = tr.unique_id(seen)

        if end_date is not None:
            tr_date = tr.date.date() if hasattr(tr.date, 'date') else tr.date
            if tr_date < end_date:
                break

        payee = ''
        if tr.counterparty and tr.counterparty.label:
            payee = tr.counterparty.label

        results['transactions'].append({
            'id': tr.id,
            'date': tr.date.strftime('%Y-%m-%d'),
            'rdate': tr.rdate.strftime('%Y-%m-%d'),
            'type': int(tr.type),
            'raw': tr.raw,
            'category': tr.category,
            'label': tr.label,
            'payee': payee,
            'amount': int(tr.amount * 100),
        })
        i += 1

    backend.dump_state()
    LOGGER.debug("bank: got %d transactions for account %s", i, accid)
    return results