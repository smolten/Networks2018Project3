#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int ret;
    int sockfd = 0;
    char send_buffer[1024];
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
    printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // Note that this is the server address that the client will connect to.
    // We do not care the source IP address and port number. 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(31000);

    ret = connect(sockfd, 
                  (struct sockaddr *) &serv_addr,
                  sizeof(serv_addr));
    if (ret < 0) {
    printf("connect() error: %s.\n", strerror(errno));
        return -1;
    }

    while (1) {
        fgets(send_buffer, 
              sizeof(send_buffer), 
              stdin);

    // TODO: You need to parse the string you read from the keyboard.
    // If it follows the format "you->server$file_name", extract the
    // file name and open the file, read each chunk and send the
    // chunk. You may need to write an inner loop to read and send
    // each chunk. 
    
    // You may also need to design a way to tell the length of the
    // file to the receiver, or use a special message to indicate the
    // end of the file.

        // I add two lines to allow the client to "gracefully exit".
        // You not not need to use these lines.
        if (strncmp(send_buffer, "exit", strlen("exit")) == 0)
    break;

        ret = send(sockfd, 
                   send_buffer, 
                   strlen(send_buffer) + 1, 
                   0);
        if(ret < 0) {
    printf("send() error: %s.\n", strerror(errno));
    break;
        }
    }

    close(sockfd);

    return 0;
}
