#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

struct args {
    int socket;
    char* handle;
    int* alive_flag;
};

void replaceNewLine(char* str)
{
    int i = 0;
    while(str[i] != '\0' && str[i] != '\n') i++;
    str[i] = '\0';
}
 
void *sender(void *input) 
{
    // get variable names from input args
    pthread_mutex_t lock = *((struct args*)input)->lock;
    int socket = ((struct args*)input)->socket;
    char *handle = ((struct args*)input)->handle;
    
    // copy handle and extra characters into message buffer 
    char message[500 + 13];   
    char inputBuffer[501];     // to account for space for handle and "> "
   
    while(1)
    {
        printf("Entering sender while loop\n");
        // copy handle and two characters
        strcpy(message, handle);
        strcat(message, "> ");

        // read from stdin, concatenate to message
        fgets(inputBuffer, 501, stdin);
        replaceNewLine(inputBuffer);
        strcat(message, inputBuffer);
        
        // lock to avoid race conditions
        pthread_mutex_lock(&lock);
        if (!(write(socket, message, strlen(message)) == 0))
        {
            pthread_mutex_unlock(&lock);
        }
        else
        {
            pthread_mutex_unlock(&lock);
            printf("Connection Closed\n");
            break;
        }

        if (memcmp(inputBuffer, "\\quit", 5) == 0)
        {
            printf("Shutting down\n");
            break;
        }

    } 
    raise(SIGINT);
}

void *receiver(void *input) 
{
    // get args from input
    int socket = ((struct args*)input)->socket;
    char *handle = ((struct args*)input)->handle;
    int *alive_flag = ((struct args*)input)->alive_flag;
    
    // buffer for message
    char message[500];

    while(*alive_flag) 
    {
        memset(message, 500, '\0');
        if (!(read(socket, message, 500) == -1))
        {
            printf("%s\n", message);

        }
        else 
        {
            printf("Server ended connection, shutting down\n");
            break;
        }

        // first eight bytes are server handle data
        if (memcmp(message + 8, "\\quit", 5) == 0)
        {
            printf("Server ended connection, shutting down\n");
            break;
        }
    }
    raise(SIGINT);
}

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

int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
            
    if ((he = gethostbyname(hostname )) == NULL) 
    {
        printf("Invalid hostname\n");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;
    
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one to be found
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    
    return 1;
}	


int main(int argc, char const *argv[]) 
{ 
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
    int port = atoi(argv[2]);

    // check valid IP from hostname
    char ip[100];
    char *hostname = argv[1];
    
    if (hostname_to_ip(hostname , ip) == -1)    // will fill ip buffer with correct ip
    {
        printf("Invalid hostname: %s", argv[1]);
        return -1;
    }

    // get handle from user
    char handle[11];
    printf("Enter your handle for communication:\n");
    fgets(handle, 11, stdin);
    replaceNewLine(handle);
    
    // create socket
    int sock = 0;
    struct sockaddr_in serv_addr; 
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

    // create mutex
    pthread_mutex_t lock; 
    
    // create args for calling sender/receiver functions
    struct args *arguments = (struct args *)malloc(sizeof(struct args));
    arguments->socket = sock;
    arguments->lock = &lock;
    arguments->handle = handle;
    

    // create multiple threads
    pthread_t sender_id; 
    pthread_t receiver_id; 
    pthread_create(&sender_id, NULL, sender, (void*) arguments);     
    pthread_create(&receiver_id, NULL, receiver, (void*) arguments);     
    
   // wait until receiver returns (guarantees to return)
    pthread_join(receiver_id, NULL);

    free(arguments);
    return 0; 
} 
