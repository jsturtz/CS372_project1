import socket
import sys
import signal
import threading
import ctypes
import time
import os

def signal_handler(signal, frame):
    sys.exit(0)

def send_handler(signum, frame):
    raise SendInterrupt

def quit_handler(signum, frame):
    raise QuitInterrupt

class SendInterrupt(Exception):
    pass

class QuitInterrupt(Exception):
    pass

class Receiver_Thread(threading.Thread):

    def __init__(self, connection):
        self.connection = connection
        threading.Thread.__init__(self)
        self.isDeadFlag = threading.Event()

    def run(self):
        
        while not self.isDeadFlag.isSet():
            message = self.connection.recv(1024)
            os.kill(os.getpid(), signal.SIGUSR1)

            if message and not "\quit" in message.decode():
                print("\n" + message.decode())

            else:
                print("Gonna send the quit signal")
                os.kill(os.getpid(), signal.SIGUSR2)

def main(args):
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    signal.signal(signal.SIGUSR1, send_handler)
    signal.signal(signal.SIGUSR2, quit_handler)

    # check args are correct
    if len(args) != 1:
        print("chatserv.py takes only one argument")
        return

    port = int(sys.argv[1])
    if port < 1 and port > 66535:
        print("Invalid port number")

    # create socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except:
        print("Socket creation failed with error")
        return

    # bind to server port
    s.bind(('', port))

   # listen on port number
    s.listen(5)

    # loop for multiple connections
    while True:

        # accept connection
        print("Ready to accept connections...")
        (connection, addr) = s.accept()
        print("Connection successful from", addr)
        
        # receiver thread for async
        receiver = Receiver_Thread(connection)
        receiver.start()

        while True:

            # wait for user input
            try:
                message = "Server> " + input("Server> ") + "\0"
                connection.send(message.encode())

                if '\quit' in message:
                    print("Closing connection with client")
                    break
            
            # raised by receiver thread to interrupt input() block
            except SendInterrupt:
                pass

            # raised by receiver when connection is dead or "\quit" detected
            except QuitInterrupt:
                break
        
        # to allow old receiver thread to break out of infinite loop
        receiver.isDeadFlag.set()

if __name__ == '__main__':
    main(sys.argv[1:])
