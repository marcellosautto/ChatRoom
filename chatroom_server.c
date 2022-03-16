#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define CLIENT_CAPACITY 100
#define BUFFER_SIZE 2048
#define NAME_LEN 32

static _Atomic unsigned int client_count = 0;
static int uid = 10;

// client structure
typedef struct
{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[NAME_LEN];
} client_t;

// client container
client_t *clients[CLIENT_CAPACITY];

// thread for each client
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void string_overwrite()
{
    printf("\r%s", "> ");
    fflush(stdout);
}

void string_trim(char *arr, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}

void queue_add(client_t *client)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < CLIENT_CAPACITY; i++)
    {
        if (!clients[i])
        {
            clients[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < CLIENT_CAPACITY; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// send current message to all clients except for the client who sent the message
void send_message(char *msg, int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < CLIENT_CAPACITY; i++)
    {
        if (clients[i] && clients[i]->uid != uid)
        {
            if (write(clients[i]->sockfd, msg, strlen(msg)) < 0)
            {
                printf("ERROR: Write to descriptor failed\n");
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

//used for printing ip address of user
void print_ip_address(struct sockaddr_in addr)
{
    printf("%d.%d.%d.%d",
           addr.sin_addr.s_addr & 0xff,
           (addr.sin_addr.s_addr & 0xff00) >> 8,
           (addr.sin_addr.s_addr & 0xff0000) >> 16,
           (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

//handle client state
void *handle_client(void *arg)
{
    char buffer[BUFFER_SIZE];
    char name[NAME_LEN];
    int leave_flag = 0;
    client_count++;

    client_t *cli = (client_t *)arg;

    // name
    if (recv(cli->sockfd, name, NAME_LEN, 0) <= 0 || strlen(name) < 2 || strlen(name) >= NAME_LEN - 1)
    {
        printf("ERROR: Invalid name\n");
        leave_flag = 1;
    }
    else
    {
        strcpy(cli->name, name);
        sprintf(buffer, "%s has joined the chat room. Say Hi!\n", cli->name);
        printf("%s", buffer);
        send_message(buffer, cli->uid);
    }

    // clear buffer
    bzero(buffer, BUFFER_SIZE);

    while (1)
    {
        if (leave_flag)
            break;

        int recieve = recv(cli->sockfd, buffer, BUFFER_SIZE, 0);

        if (recieve > 0)
        {
            if (strlen(buffer) > 0)
            {
                send_message(buffer, cli->uid);
                string_trim(buffer, strlen(buffer));
                printf("%s -> %s", buffer, cli->name);
            }
        }
        else if (recieve == 0 || strcmp(buffer, "exit") == 0)
        {
            sprintf(buffer, "%s has left the Chat Room.\n", cli->name);
            printf("%s", buffer);
            send_message(buffer, cli->uid);
            leave_flag = 1;
        }
        else
        {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        // clear buffer
        bzero(buffer, BUFFER_SIZE);
    }

    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    client_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    pthread_t tid;

    // socket settings
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);

    // signals
    signal(SIGPIPE, SIG_IGN);

    if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
    {
        printf("ERROR: setsocketopt\n");
        return EXIT_FAILURE;
    }

    // bind
    if (bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("ERROR: bind failure\n");
        return EXIT_FAILURE;
    }

    // listen
    if (listen(listenfd, 10) < 0)
    {
        printf("ERROR: listen\n");
        return EXIT_FAILURE;
    }

    printf("~~~~~Welcome to the Chat Room!~~~~~\n");

    while (1)
    {
        socklen_t client_address_len = sizeof(client_address);
        connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_address_len);

        // check for chatroom capacity
        if (client_count + 1 == CLIENT_CAPACITY)
        {
            printf("CONNECTION FAILED: Chat Room At Max Capacity.\n");
            print_ip_address(client_address);
            close(connfd);
            continue;
        }

        // client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = client_address;
        cli->sockfd = connfd;
        cli->uid = uid++;

        // add client to queue and create thread
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        // reduce CPU ussage
        sleep(1);
    }

    return EXIT_SUCCESS;
}