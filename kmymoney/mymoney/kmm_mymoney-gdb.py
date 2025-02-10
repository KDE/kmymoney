import gdb.printing

def call_method(val, method_name):
    addr = str(val.address).split()
    eval_string = "(*(" + str(val.dynamic_type) + "*)(" + addr[0] + "))." + method_name + "()"
    return gdb.parse_and_eval(eval_string)

class MyMoneyMoneyPrinter(gdb.ValuePrinter):
    "Print a MyMoneyMoney"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return call_method(self.val, "toDouble")

    def display_hint(self):
        return 'string'

pp = gdb.printing.RegexpCollectionPrettyPrinter("kmm_mymoney")
pp.add_printer('MyMoneyMoney', '^MyMoneyMoney$', MyMoneyMoneyPrinter)

# In gdb the registration of the pretty-printer can be checked with 'set verbose on' before executing the program.
# Installed pretty printers can be displayed with 'info pretty-printer'
# Pretty printer support can be checked after starting the program with 'p MyMoneyMoney::ONE'
# The pretty printer is registered globally if no object file is available.
gdb.printing.register_pretty_printer(gdb.current_objfile(), pp)

