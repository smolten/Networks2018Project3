#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdbool.h>
#include <signal.h>

//Intercept exit signal and close sockets before exiting
int socket_listen, socket_send;
void sigintHandler(int sig_num) {
    close(socket_listen);
    close(socket_listen);
    printf("sockets closed\n");
    exit(0);
}

int main(int argc, char *argv[]) {

    //Redirect close to custom function
    signal(SIGINT, sigintHandler);

    //LISTEN

    //Claim listen socket
    socket_listen = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_listen < 0)
    {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }
    //make listen address (any)
    struct sockaddr_in listen_address;
    memset(&listen_address, 0, sizeof(listen_address));
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_address.sin_port = htons(32001);
    //Attempt to bind to address
    int retBind = bind(socket_listen, (struct sockaddr *) &listen_address, sizeof(listen_address));
    if (retBind < 0) {
        printf("bind() error: %s.\n", strerror(errno));
        return -1;
    } else {
        printf("Successfully bound to socket...\n");
    }

    //SENDING
    //Claim socket
    socket_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_send < 0)
    {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }
    //make remote address
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(32000);


    //Run the server
    char message_buffer[1024];
    struct sockaddr_in client_address;
    while (1) {

    	//Listen for message from client
        socklen_t len = sizeof(client_address);
        int retRecv = recvfrom(socket_listen, 
            message_buffer, 
            sizeof(message_buffer), 
            0,
            (struct sockaddr *) &client_address,
            &len);
        //Error message
        if (retRecv <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }

        //Print message
        char* srcAddress = inet_ntoa(client_address.sin_addr);
        printf("[from %s] %s\n", srcAddress, message_buffer);

        //Prepare response
        bool correctFormat = true;
        char* expect = "you->server#";
        for(int i=0; i<retRecv && i<12; i++) {
            if (message_buffer[i] != expect[i]) {
                correctFormat = false;
                break;
            }
        }
        if (correctFormat) {
            expect = "server->you#";
            for(int i=0; i<12; i++) {
                message_buffer[i] = expect[i];
            }
        } else {
            sprintf(message_buffer,"server->you#ERROR: Unrecognized message format\n");
        }

        //Respond
        int retSnd = sendto(socket_send, 
			message_buffer, 
			sizeof(message_buffer), 
			0,
			(struct sockaddr *) &remote_addr,
			sizeof(remote_addr));
		if (retSnd <= 0)
		{
			printf("recvfrom() error: %s.\n", strerror(errno));
			return -1;
		}
    }

    return 0;
}

