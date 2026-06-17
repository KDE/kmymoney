# SPDX-FileCopyrightText: 2024
# SPDX-License-Identifier: GPL-2.0-or-later
"""
Companion script for debugging kmymoneywoob.py outside of KMyMoney.

Run this directly from PyCharm (or any IDE) to exercise the Woob bridge
functions without needing the full KMyMoney application.

Usage:
    python testrunner.py backends
    python testrunner.py accounts BACKEND_NAME
    python testrunner.py transactions BACKEND_NAME ACCOUNT_ID [END_DATE]
"""

import datetime
import sys
from pathlib import Path

# Point to the woob source tree relative to this script.
# Assumes the woob repo lives alongside the kmymoney repo:
#   ~/Sourcecode/kmymoney/.../testrunner.py  →  ~/Sourcecode/woob/build/lib
WOOB_BUILD_LIB = Path(__file__).resolve().parents[4] / "woob" / "build" / "lib"
sys.path.insert(0, str(WOOB_BUILD_LIB))

from kmymoneywoob import get_backends, get_accounts, get_transactions


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return

    cmd = sys.argv[1]

    if cmd == "backends":
        backends = get_backends()
        for name, info in backends.items():
            print(f"{name}: {info['module']}")

    elif cmd == "accounts":
        if len(sys.argv) < 3:
            print("Usage: python testrunner.py accounts BACKEND_NAME")
            return
        backend = sys.argv[2]
        accounts = get_accounts(backend)
        for accid, acc in accounts.items():
            print(f"{accid}: {acc['name']} ({acc['number']}) {acc['balance']} {acc['currency']}")

    elif cmd == "transactions":
        if len(sys.argv) < 4:
            print("Usage: python testrunner.py transactions BACKEND_NAME ACCOUNT_ID [END_DATE]")
            return
        backend = sys.argv[2]
        accid = sys.argv[3]
        if len(sys.argv) > 4:
            end_date = sys.argv[4]
        else:
            # Default: today minus 60 days, matching woob's default settings
            end_date = (datetime.date.today() - datetime.timedelta(days=60)).isoformat()
        result = get_transactions(backend, accid, end_date)
        print(f"Account: {result['name']} ({result['id']})")
        print(f"Balance: {result['balance']}")
        for tr in result["transactions"]:
            label = tr['label'] or '-'
            print(f"  {tr['date']}  {label:40s}  {tr['amount']/100:>10.2f}  [{tr['id']}]")

    else:
        print(f"Unknown command: {cmd}")
        print(__doc__)


if __name__ == "__main__":
    main()
