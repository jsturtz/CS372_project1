# Program:            chatserve.py
# Programmer:         Jordan Sturtz
# Course Name:        CS372 Introduction to Computer Networks
# Description:        This is a very simple chat server application that will 
#                     listen on a user-defined port number for connection 
#                     requests and then make a TCP connection to allow for
#                     back-and-forth chatting.
# Last Modified:      07 - 24 - 2019

import socket
import sys
import signal
import threading
import ctypes
import time
import os

# description:        validates that cmd-line arguments are valid, i.e. there's
#                     only one argument and it is a valid port number
# pre-conditions:     args is a list containing only one element: the port number
#                     represented as a string.
# post-conditions:    returns True if args are valid, False otherwise

def validate(args):
    if len(args) != 1:
        return False

    if int(args[0]) < 1 or int(args[0]) > 66535:
        return False

    return True

# description:        creates socket, binds to provided port number, and then
#                     listens on that port
# pre-conditions:     port is an int representing a valid port number (1 - 66535)
# post-conditions:    returns socket object

def setupSocket(port):

    # create socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # bind to server port
    s.bind(('', port))

    # listen on port number
    s.listen(5)
    return s

# description:        waits for message from socket connection, returns false
#                     if connection closes or message is "\quit"
# pre-conditions:     connection can be either alive or dead
# post-conditions:    if successful message, prints to stdout and returns True

def recvMessage(connection):
    # wait for client to send message
    message = connection.recv(1024).decode()
    
    # empty string for message implies connection has ended
    if not message or '\quit' in message:
        return False

    else:
        print(message)
        return True

# description:        waits for stdin and sends message to socket connection
# pre-conditions:     connection can be alive or dead
# post-conditions:    returns False if user types '\quit' or error in sending

def sendMessage(connection):

    # then wait for server to respond
    message = "Server> " + input("Server> ") + "\0"

    # check if user typed \quit
    if "\quit" in message:
        return False

    # try to send (error if connection ended)
    try:
        connection.send(message.encode())
        return True
    except:
        return False

def main(args):
    
    if not validate(args):
        print("ERROR: Usage: [port number]")
        return

    s = setupSocket(int(args[0]))

    # loop for multiple connections
    while True:

        # accept connection
        try:
            (connection, addr) = s.accept()
        except KeyboardInterrupt:
            break;

        print("Connection successful from", addr)

        while True:
            
            if not recvMessage(connection): break;
            if not sendMessage(connection): break;

        print("Connection terminated") 
    return

if __name__ == '__main__':
    main(sys.argv[1:])
