/* Program:            chatclient.c */
/* Programmer:         Jordan Sturtz */
/* Course Name:        CS372 Introduction to Computer Networks */
/* Description:        This is a very simple chat client application that will */ 
/*                     attempt to connect at the hostname and port number specified */
/*                     at the commandline. Upon reciept, chatclient.c and */
/*                     chatserve.py will be able to exchange messages, quitting */
/*                     if either host types '\quit' or if the connection ends. */
/* Last Modified:      07 - 24 - 2019 */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <netdb.h>
#include <signal.h>

/* description:         will find and replace the first instance of newline character with null terminator */ 
/* preconditions:       none */
/* postconditions:      the data pointed at by str will be modified*/
void replaceNewLine(char* str)
{
    int i = 0;
    while(str[i] != '\0' && str[i] != '\n') i++;
    str[i] = '\0';
}

/* description:         will verify that the port argument is a valid port number between 1 and 66535*/ 
/* preconditions:       none */
/* postconditions:      returns 0 for true, -1 for false */
int valid_port(char const *port) 
{
    int num;
    if ((num = atoi(port)))
    {
        if ((num < 65536) && (num > 0))
        {
            return 0;
        }
        else return -1;
    }
    else return -1;
}

/* SOURCE: Adapted heavily from this website: */
/* https://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/ */ 

/* description:         will attempt to convert name of hostname to ip address and fill ip buffer*/ 
/* preconditions:       ip must be long enough to hold ip address*/
/* postconditions:      ip data will change, will return -1 for invalid conversion or 0 for success*/
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
            
    if ((he = gethostbyname(hostname )) == NULL) 
    {
        printf("Invalid hostname\n");
        return -1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;
    
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one to be found
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    
    return -1;
}	

/* description:         waits for input from stdin and sends to socket*/ 
/* preconditions:       message must be 513 in length, handle must be less than 10 chars, inputBuffer must be 501 in length*/
/* postconditions:      will send data to socket and return -1 if either connection has ended or message is '\quit'*/

int sendToServer(int sock, char *inputBuffer, char *message, char *handle)
{
    memset(message, 513, '\0');
    printf("%s> ", handle);

    // copy handle and two characters
    strcpy(message, handle);
    strcat(message, "> ");

    // read from stdin, concatenate to message
    fgets(inputBuffer, 501, stdin);
    replaceNewLine(inputBuffer);
    strcat(message, inputBuffer);

    // check whether to quit
    if (memcmp(inputBuffer, "\\quit", 5) == 0)
    {
        printf("Closed connection\n");
        return -1;
    }
    
    // try to send message
    if (write(sock, message, strlen(message)) == 0)
    {
        printf("Server closed connection\n");
        return -1;
    }
    return 0;
}

/* description:         will wait to receive data from socket*/ 
/* preconditions:       message must be 513 in length, handle must be less than 10 chars in length*/
/* postconditions:      message will be sent to socket, will return -1 if connection has ended or message is "\quit"*/
int readFromServer(int sock, char *message, char *handle)
{
    // now, wait for message from server
    memset(message, 513, '\0');
    if (!(read(sock, message, 513) == -1))
    {
        printf("%s\n", message);
    }

    else
    {
        printf("Server closed connection\n");
        return -1;
    }

    if (memcmp(message + 8, "\\quit", 5) == 0)
    {
        printf("Server ended connection, shutting down\n");
        return -1;
    }
    
    return 0;
}

int main(int argc, char const *argv[]) 
{ 
    char message[500 + 13];             // holds message for sending/receiving
    char inputBuffer[501];              // buffer for input
    int port;                           // port number for connection
    char ip[100];                       // IP to be converted from hostname
    char *hostname = argv[1];           // hostname as string of characters
    char handle[11];                    // handle for client user
    int sock = 0;                       // socket file descriptor
    struct sockaddr_in serv_addr;       // struct to hold server info

    // Check right number of args
    if (argc != 3) 
    {
        printf("Usage: [IP address] [Port Number]");
        return -1;
    }

    // check valid ports
    if (valid_port(argv[2]) == -1)
    {
        printf("Invalid port number: %s", argv[2]);
        return -1;
    }
    port = atoi(argv[2]);
    
    // fill ip with result fron DNS query and check if valid
    if (hostname_to_ip(hostname , ip) == -1) 
    {
        printf("Invalid hostname: %s", argv[1]);
        return -1;
    }

    // get handle from user
    printf("Enter your handle for communication:\n");
    fgets(handle, 11, stdin);
    replaceNewLine(handle);
    
    // create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n socket creation error \n"); 
        return -1; 
    } 
    
    // set up sockaddr_in struct 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    // Convert IPv4 address from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
     
    // establish connection 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    while (1)
    {
        if (sendToServer(sock, inputBuffer, message, handle) == -1)
        {
            break;
        }
        
        if (readFromServer(sock, message, handle) == -1)
        {
            break;
        }
    } 
    return 0; 
} 
