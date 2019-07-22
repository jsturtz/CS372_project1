import socket
import sys
import threading

def validate():
    if len(sys.argv) != 2:
        print("chatserve.py takes one only one argument (port number)")
        return False
    if int(sys.argv[1]) < 1 or int(sys.argv[1]) > 66535:
        print("valid port numbers are between 1 and 66535")
        return False
    return True

def sender(connection, lock):
    
    while True:
        message = "Server> " + input() + "\0"
        
        try:
            # prevents buffering issues
            lock.acquire()
            connection.send(message.encode())
            lock.release()

        # just in case client disconnects before message sends
        except Exception as err:
            print("Connection closed")
            return

        if message == 'Server> \quit':
            return

def receiver(connection, lock):
    
    while True:
        try:
            message = connection.recv(1024)
            lock.acquire()

            if message:
                print(message.decode())
                lock.release()
            else:
                print("Connection closed at recv")
                # remove(connection)
                return
        
        except Exception as err:
            print("Connection closed at recv" + str(err))
            return

        if message == '\exit': 
            return

# validate port number and number of arguments
if not validate():
    exit()
port = int(sys.argv[1])

# create socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except:
    print("Socket creation failed with error")
    exit()

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

    # create multiple threads for sending/receiving
    send_thread = threading.Thread(target=sender, args=(c, lock), daemon=True)
    recv_thread = threading.Thread(target=receiver, args=(c, lock), daemon=True)

    # start threads
    send_thread.start()
    recv_thread.start()
