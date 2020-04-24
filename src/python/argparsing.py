"""
    File: argparsing.py,
    Description: An setup tool for portscan.py, intakes arguments from the command line and 
        sets them up as needed (in the correct order).
    Author: piergerlofsdonia
    License: GPL-3.0

"""

args_data = "arguments.json"
from argparse import ArgumentParser # Argparse used to take in command-line arguments.
import json 

class ArgumentError():
    def __init__(self, argument_type):
        print("Incorrect argument provided, please input a valid {} and try again.".format(argument_type))

    pass

def setArgument(parser_object, flag, name, default_value, help_text, num_args, type_of):
    parser_object.add_argument(flag, name, default=default_value, help=help_text, nargs=num_args, type=type(type_of))
    return True

json_args_order = ['flag', 'long-flag', 'default', 'help', 'nargs', 'type']

def parseJsonData():
    argument_array = []

    with open(args_data, 'r') as json_file:
        json_dict = json.load(json_file)
        
    for json_arg in json_dict:
        argument_tuple = [None for n in json_args_order]
        for key in json_dict[json_arg]:
            arg = json_dict[json_arg][key]
            
            for i, order in enumerate(json_args_order):
                if ( key.lower() == order.lower() ):
                    argument_tuple[i] = arg
        argument_tuple = tuple(argument_tuple)
        argument_array.append(argument_tuple)

    return argument_array

def setupArgparse():
    args_array = parseJsonData()
    cl_parse = ArgumentParser(description="Scan a hostname for open ports.\n")
    for args_tuple in args_array:
        setArgument(cl_parse, *args_tuple)
    print(cl_parse.parse_args())
    return cl_parse

def cleanHostname(hostname_str):
    # Allow entry of invalid hostname format (e.g. http://website.com/) by cleaning up invalid hostnames.
    if ( hostname_str.lower()[:7] == 'http://' ):
        hostname_str = hostname_str[7:]
        if ( hostname_str[-1] == '/' ):
            hostname_str = hostname_str[:-1]

    return hostname_str

def checkProtocol(protocol_str):
    protocol_pass = False
    protocol_options = ["udp", "tcp"]
    for protocol in protocol_options:
        if ( protocol in protocol_str.lower() ):
            protocol_pass = True
            protocol_str = protocol.upper()
    if not ( protocol_pass ):
        raise ArgumentError("protocol")
    return protocol_str

def parseArgs(arg_tuple):
    hostname, ports, protocol = arg_tuple        
    hostname = cleanHostname(hostname)
    protocol = checkProtocol(protocol)
    return (hostname, ports, protocol)

def getArgs(argparse_obj=None):
    try:
        arg_tuple = vars(argparse_obj.parge_args()).values()
        return parseArgs(arg_tuple)

    except (UnboundLocalError, AttributeError):
        argparse_obj = setupArgparse()
        return parseArgs(vars(argparse_obj.parse_args()).values())
