#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>

//Intercept exit signal and close sockets before exiting
int listenfd;
void closeSockets() {
    shutdown(listenfd, SHUT_RDWR);
    close(listenfd);
    printf("sockets closed\n");
}
void sigintHandler(int sig_num)
{   
    closeSockets();
    exit(0);
}

// This line must be included if you want to use multithreading.
// Besides, use "gcc ./tcp_receive.c -lpthread -o tcp_receive" to compile
// your code. "-lpthread" means link against the pthread library.
#include <pthread.h>

// This the "main" function of each worker thread. All worker thread runs
// the same function. This function must take only one argument of type 
// "void *" and return a value of type "void *". 
void *worker_thread(void *arg) {

    int connfd = (int) (long)arg;
    printf("[%d] worker thread started.\n", connfd);

    while (1) {

        int filesize = -1;
        int bytesRecv = 0;
        FILE* file = NULL;
        char recv_buffer[1024];
        char filename[1024];
        memset(recv_buffer,'\0', 1024);
        memset(filename,'\0', 1024);
        while (filesize == -1 || bytesRecv < filesize)
        {
            //Receive data
            int ret = recv(connfd, recv_buffer, sizeof(recv_buffer), 0);
            if (ret < 0) {
                printf("[%d] recv() error: %s.\n", connfd, strerror(errno));
                return NULL;
            } else if (ret == 0) {
                // The connection is terminated by the other end.
                printf("[%d] connection lost\n", connfd);
                return NULL;
            }
            //printf("[%d] RECV %s\n", connfd, recv_buffer);

            //Read metadata if it has not yet been read
            if (filesize == -1) {
                int read = sscanf(recv_buffer, "%d %s", &filesize, filename);
                if (read != 2) {
                    printf("Can't parse filesize and name from \"%s\"\n", recv_buffer);
                    closeSockets();
                    exit(-1);
                }
                file = fopen(filename, "wb");
                //printf("[%d] Receiving \"%s\", size is %d bytes\n", connfd, filename, filesize);

                //Don't write file metadata to file
                continue;
            }

            //Limit writing to filesize # of bytes
            int bytesRead = ret;
            bytesRecv += bytesRead;

            //write to file
            fwrite(recv_buffer, 1, bytesRead, file);
            printf("Read %d of %d bytes...\n", bytesRecv, filesize);
        }

        printf("Wrote %d bytes of %s\n", filesize, filename);
        fclose(file);
    }


    printf("[%d] worker thread terminated.\n\n", connfd);
    return NULL;
}


// The main thread, which only accepts new connections. Connection socket
// is handled by the worker thread.
int main(int argc, char *argv[])
{

    //Redirect close to custom function
    signal(SIGINT, sigintHandler);

    //Get listen socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        closeSockets();
        return -1;
    }

    //Get server adress
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(31000);

    //Attempt to bnd and listen
    int ret = bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        printf("bind() error: %s.\n", strerror(errno));
        closeSockets();
        return -1;
    }
    if (listen(listenfd, 10) < 0) {
        printf("listen() error: %s.\n", strerror(errno));
        closeSockets();
        return -1;
    }

    socklen_t len;
    struct sockaddr_in client_addr;
    while (1) {
        printf("waiting for connection...\n");
        int connfd = accept(listenfd, (struct sockaddr*) &client_addr, &len);

        if(connfd < 0) {
            printf("accept() error: %s.\n", strerror(errno));
            closeSockets();
            return -1;
        }
        printf("conn accept - %s.\n", inet_ntoa(client_addr.sin_addr));

        pthread_t tid;
        pthread_create(&tid, NULL, worker_thread, (void *)connfd);
    }

    closeSockets();
    return 0;
}
