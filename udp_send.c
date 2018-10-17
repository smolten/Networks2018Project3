#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {

    int ret;
    int sockfd;
    struct sockaddr_in servaddr;
    char send_buffer[1024];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
	   printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr 
        = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(32000);

    while (1) {

        fgets(send_buffer, 
              sizeof(send_buffer), 
              stdin);

        ret = sendto(sockfd, 
               send_buffer, 
               sizeof(send_buffer), 
               0,
               (struct sockaddr *) &servaddr,
               sizeof(servaddr));

        if (ret <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }

    }

    return 0;
}

