import argparsing
import socket
from threading import Thread, Semaphore

def threadPrint(semaphore, message):
    semaphore.acquire()
    print(message)
    semaphore.release()

setProtocol = {
    "TCP": (socket.AF_INET, socket.SOCK_STREAM),
    "UDP": (socket.AF_INET, socket.SOCK_DGRAM)
}

def scanPort(hostname, port, protocol):
    print_access = Semaphore(value=1)
    
    try:
        address_family, protocol = setProtocol[protocol]
        with socket.socket(address_family, protocol) as sock:
            sock.settimeout(2)
            sock.connect((hostname, int(port)))
            sock.send(bytes("Is there anybody out there?\r\n".encode('utf-8')))
            conn_result = sock.recv(100)
            output = "Port {} is open.\nResponse: {}".format(port, conn_result)
            sock.close()
    except socket.timeout:
        output = "Port {} is not open.".format(port)
    except ConnectionResetError:
        output = "Port {} access denied.".format(port)
    
    threadPrint(print_access, output)

def formatPortPrint(ports):
    str_segments = []
    for i, p in enumerate(ports):
        str_to_append = "{}, ".format(p) if ( i != len(ports)-1 ) else "& {}".format(p)
        str_segments.append(str_to_append)
           
    return ''.join(str_segments)

def main():
    hostname, ports, protocol = argparsing.getArgs(None)
    
    ports_as_str = formatPortPrint(ports) if ( len(ports) > 1 ) else str(ports[0])
    print("Scanning port(s) {} on hostname {} using {}.".format(ports_as_str, hostname, protocol))

    for port in ports:
        thread = Thread(target=scanPort, args=(hostname, port, protocol))
        thread.start()

main()
