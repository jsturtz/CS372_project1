import socket
import sys
import threading

try: 
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error as err:
    print("Error creating socket %s", %(err))

port = 3291

s.connect("96.10.42.139", port)


