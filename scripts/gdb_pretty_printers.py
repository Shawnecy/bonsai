import gdb;
from pprint import pprint;

class counted_string_printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val['Start'].string('ascii', 'strict', int(self.val['Count']))


class xml_property_printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        name = counted_string_printer(self.val['Name']).to_string()
        value = counted_string_printer(self.val['Value']).to_string()

        if name and value:
            return "%s=\"%s\"" % (name, value)
        elif name:
            return "%s" % (name)
        else:
            return "Error printing xml_property, Name field not set, Value set to : \"%s\"" % (value)


class xml_token_printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        type_string = str(self.val['Type']).split('_')[1]
        value_string = xml_property_printer(self.val['Property']).to_string()

        if (type_string == "Boolean" or
            type_string == "Float" or
            type_string == "Int" or
            type_string == "Property"):
            return "%s" % (value_string)
        elif type_string == "Open":
            return "<%s>" % (value_string)
        elif  type_string == "Close":
            return "</%s>" % (value_string)
        elif  type_string == "Unknown":
            return "Zero Token"
        else:
            return "Invalid type string encountered in pretty printer {}".format(type_string)






def bonsai_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("bonsai_printers")
    pp.add_printer('counted_string'       , '^counted_string$' , counted_string_printer)
    pp.add_printer('xml_token_printer'    , '^xml_token$'      , xml_token_printer)
    pp.add_printer('xml_property_printer' , '^xml_property$'   , xml_property_printer)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    bonsai_pretty_printers())
