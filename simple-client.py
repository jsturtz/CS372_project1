# Import socket module 
import socket                
import sys
import threading

def sender(connection, lock):

    while True:
        message = "Client> " + input()
        
        try:
            lock.acquire()
            connection.send(message.encode())
            lock.release()
        except Exception as err:
            print("Connection closed at send: " + str(err))
            return

        if message == '\exit':
            return

def receiver(connection, lock):
    
    while True:
        try:
            message = connection.recv(1024)

            # send signal to sender thread, acquire lock to block
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

# Create a socket object 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)          
  
# Define the port on which you want to connect 
port = int(sys.argv[1])
  
# connect to the server on local computer 
s.connect(('127.0.0.1', port)) 

# validate port number and number of arguments
port = int(sys.argv[1])

# loop for multiple connections

# create mutex
lock = threading.Lock()

# create multiple threads for sending/receiving
send_thread = threading.Thread(target=sender, args=(s, lock), daemon=True)
recv_thread = threading.Thread(target=receiver, args=(s, lock), daemon=True)

# start threads
send_thread.start()
recv_thread.start()

while send_thread.is_alive() and recv_thread.is_alive():
    pass
