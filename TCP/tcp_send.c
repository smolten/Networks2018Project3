#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

//Intercept exit signal and close sockets before exiting
int sockfd;
void closeSockets() {
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    printf("sockets closed\n");
}
void sigintHandler(int sig_num)
{   
    closeSockets();
    exit(0);
}

int main(int argc, char *argv[])
{

  //Redirect close to custom function
  signal(SIGINT, sigintHandler);

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

  //TCP Connect
  int ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if (ret < 0)
  {
    printf("connect() error: %s.\n", strerror(errno));
    return -1;
  }

  while (1)
  {
    //Read input
    fgets(send_buffer, sizeof(send_buffer), stdin);

    //Exit command
    if (strncmp(send_buffer, "exit", strlen("exit")) == 0)
      break;

    //Check for file send command
    bool sendFileCommand = true;
    char* expect = "you->server$";
    for(int i=0; i<12; i++)
    {
      if (send_buffer[i] != expect[i])
      {
        sendFileCommand = false;
        break;
      }
    }

    if (sendFileCommand == false) {
      printf("invalid command %s\n", send_buffer);
      continue;
    }
    
    //Attempt to send the file

    //Construct filename
    //replace newline with end of string
    char* ptr = strchr(send_buffer, '\n');
    if (ptr != NULL) { *ptr = '\0'; }
    //Check for file
    char filename[1012];
    memcpy(filename, &send_buffer[12], 1012);
    if ( access (filename, F_OK) == -1)
    {
      //File doesn't exist
      printf("File \"%s\" does not exist.\n", filename);
      continue;
    } else
    {

      //Send the file
      printf("Sending \"%s\"...\n", filename);

      //Open file
      FILE* file = fopen(filename, "rb");
      //Get filesize by seeking end then rewinding
      fseek(file, 0L, SEEK_END);
      int fileSize = ftell(file);
      rewind(file);

      //Alert receiever of filesize and name
      sprintf(send_buffer, "%d RCV%s", fileSize, filename);
      ret = send(sockfd, send_buffer, strlen(send_buffer) + 1, 0);
      if(ret < 0) {
        printf("send() error: %s.\n", strerror(errno));
        break;
      }

      //Send chunks
      int bytesSent = 0;
      while (bytesSent < fileSize)
      {
        memset(send_buffer,'\0', 1024);
        int bytesRead = fread(send_buffer, 1, 1024, file);
        bytesSent += bytesRead;
        printf("Read %d of %d bytes...\n", bytesSent, fileSize);
        
        ret = send(sockfd, send_buffer, strlen(send_buffer), 0);
        
        //if(ret != bytesRead) {
        //  printf("ERROR: Sent %d bytes but read %d\n", ret, bytesRead);
        if (ret < 0) {
          printf("send() error: %s.\n", strerror(errno));
          break;
        }

      }
      printf("Finished reading %s\n\n", filename);
      fclose(file);
    }
    

  }

  closeSockets();
  return 0;
}
