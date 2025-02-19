# SPDX-FileCopyrightText: 2025 Ralf Habacker <ralf.habacker@freenet.de>
#
# SPDX-License-Identifier: GPL-3.0-or-later

import gdb.printing

in_qt_creator = False
if 'theDumper' in globals():
    in_qt_creator = True

def eprint(*args, **kwargs):
    print('++++++', *args, file=sys.stdout, **kwargs)

def call_method(val, method_name, suffix = '()'):
    addr = str(val.address).split()
    eval_string = "(*(" + str(val.dynamic_type) + "*)(" + addr[0] + "))." + method_name + suffix
    #eprint(eval_string)
    return gdb.parse_and_eval(eval_string)

def call_sub_method(val, method_name, method_return_type, submethod):
    addr = str(val.address).split()
    eval_string = "((" + method_return_type + ")(*(" + str(val.dynamic_type) + "*)(" + addr[0] + "))." + method_name + "()" + ")." + submethod + "()"
    #eprint(eval_string)
    return gdb.parse_and_eval(eval_string)

def generate_methods_to_call(m, types):
    ts = '.toStdString().c_str()'
    v = m + '()'
    if not in_qt_creator:
        if m in types:
            if types[m] == 'QDate':
                v += '.toString(1)' + ts
            elif types[m] == 'QString':
                v += ts
        else: # default is QString
            v += ts
    #eprint(v)
    return v

class MyMoneyPrinterBase(gdb.ValuePrinter):
    def __init__(self, val, method):
        self.__val = val
        self.__method = method

    def to_string(self):
        return call_method(self.__val, self.__method)

    def display_hint(self):
        return 'string'

class MyMoneyMoneyPrinter(MyMoneyPrinterBase):
    "Print a MyMoneyMoney"

    def __init__(self, val):
        super().__init__(val, 'toDouble')

class MyMoneyMethodsPrinterBase(gdb.ValuePrinter):

    def __init__(self, val, methods, types = {}):
        self.__val = val
        self.__methods = methods
        self.__types = types

    def to_string(self):
        return ''

    def display_hint(self):
        return ''

    def children(self):
        for m in self.__methods:
            v = generate_methods_to_call(m, self.__types)
            yield m, call_method(self.__val, v, '')

class MyMoneyPricePrinter(MyMoneyMethodsPrinterBase):
    "Print a MyMoneyPrice"

    def __init__(self, val):
        self.__val = val
        methods = [ 'date', 'source', 'from', 'to' ]
        types = { 'date': 'QDate' }
        super().__init__(val, methods, types)

    def to_string(self):
        return call_sub_method(self.__val, 'rate', 'MyMoneyMoney', 'toDouble')

class MyMoneySecurityPrinter(MyMoneyPrinterBase):
    "Print a MyMoneySecurity"

    def __init__(self, val):
        super().__init__(val, 'name')

class MyMoneySplitPrinter(gdb.ValuePrinter):
    "Print a MyMoneySplit"

    def __init__(self, val):
        self.__val = val
        self.__methods = [  'accountId',
                            'action',
                            'bankID',
                            'costCenterId',
                            'investmentTransactionType',
                            'isAmortizationSplit',
                            'isAutoCalc',
                            'memo',
                            'number',
                            'payeeId',
                            'reconcileDate',
                            'reconcileFlag',
                            'tagIdList',
                            'transactionId',
                        ]

        self.__types =  { 'investmentTransactionType': 'int',
                          'isAmortizationSplit': 'bool',
                          'isAutoCalc': 'bool',
                          'reconcileDate': 'QDate',
                          'reconcileFlag': 'int',
                          'tagIdList': 'QList',
                        }

    def to_string(self):
        return ''

    def children(self):
        for m in self.__methods:
            v = generate_methods_to_call(m, self.__types)
            yield m, call_method(self.__val, v, '')
        yield 'price', call_sub_method(self.__val, 'price', 'MyMoneyMoney', 'toDouble')
        yield 'shares', call_sub_method(self.__val, 'shares', 'MyMoneyMoney', 'toDouble')
        yield 'value', call_sub_method(self.__val, 'value', 'MyMoneyMoney', 'toDouble')

    def display_hint(self):
        return ''

pp = gdb.printing.RegexpCollectionPrettyPrinter("kmm_mymoney")
pp.add_printer('MyMoneyMoney', '^MyMoneyMoney$', MyMoneyMoneyPrinter)
pp.add_printer('MyMoneySecurity', '^MyMoneySecurity$', MyMoneySecurityPrinter)
pp.add_printer('MyMoneySplit', '^MyMoneySplit$', MyMoneySplitPrinter)
pp.add_printer('MyMoneyPrice', '^MyMoneyPrice$', MyMoneyPricePrinter)

# In gdb the registration of the pretty-printer can be checked with 'set verbose on' before executing the program.
# Installed pretty printers can be displayed with 'info pretty-printer'
# Pretty printer support can be checked after starting the program with 'p MyMoneyMoney::ONE'
# The pretty printer is registered globally if no object file is available.
gdb.printing.register_pretty_printer(gdb.current_objfile(), pp)

eprint("kmymoney pretty printer registered")
