import socket
import sys
import signal
import threading
import ctypes
import time

def signal_handler(signal, frame):
    sys.exit(0)

class Sender_Thread(threading.Thread):

    def __init__(self, connection, lock):
        threading.Thread.__init__(self)
        self.connection = connection
        self.lock = lock
        self.isDead = threading.Event()
        self.daemon = True

    def run(self):
        while True:
            if self.isDead.isSet(): break
            message = "Server> " + input() + "\0"
            if self.isDead.isSet(): break
            
            # prevent buffering issues between sender/receiver
            self.lock.acquire()
            try:
                self.connection.send(message.encode())

            # in case server ends connection
            except:
                break
            self.lock.release()

            if '\quit' in message:
                break

class Receiver_Thread(threading.Thread):

    def __init__(self, connection, lock):
        super(Receiver_Thread, self).__init__()
        self.connection = connection
        self.lock = lock
        self.daemon = True

    def run(self):
        while True:

            try:
                message = self.connection.recv(1024).decode()

                if message:
                    self.lock.acquire()
                    print(message)
                    self.lock.release()
                else: break
            except: break

            if '\quit' in message: break

def main(args):
    
    # check args are correct
    if len(args) != 1:
        print("chatserv.py takes only one argument")
        return

    port = int(sys.argv[1])
    if port < 1 and port > 66535:
        print("Invalid port number")

    # register handler for SIGINT signal
    signal.signal(signal.SIGINT, signal_handler)

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
        (c, addr) = s.accept()
        print("Connection successful from", addr)

        # create mutex
        lock = threading.Lock()
        
        # create a thread for the sender/receiver for async
        sender = Sender_Thread(c, lock)
        receiver = Receiver_Thread(c, lock)
        
        sender.start()
        receiver.start()
        
        # if either thread dies, the connection has ended
        while sender.isAlive() and receiver.isAlive():
            time.sleep(0.5)
        
        # receiver will terminate when socket is dead, sender will
        # unfortunately block on input() system call
        sender.isDead.set()
        print("Connection ended, enter newline to end blocking input call")

if __name__ == '__main__':
    main(sys.argv[1:])
