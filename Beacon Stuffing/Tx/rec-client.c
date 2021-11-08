/***************************************************
 * This is a C script that connects to a specified *
 * IP address and Port to receive GNSS corrections *
 * and insert them into a Wi-Fi beacon             *
 ***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

//#define PORT 5013    /* the port client will be connecting to */

#define MAXDATASIZE 1024 /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    int sockfd, numbytes, len;
    char buffer[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */
    char newWord[MAXDATASIZE];
    char tempWord[MAXDATASIZE];

    if (argc != 3)
    {
        fprintf(stderr,"usage: ./client ip-address port\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL)
    {  /* get the host info */
        herror("gethostbyname");
        exit(1);
    }

    if(argv[2] == NULL)
    {
        fprintf(stderr,"Incorrect port\n");
        exit(1);
    }else{
        char *port = argv[2];
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(5013);    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    while (1)
    {


        if ((numbytes=recv(sockfd, buffer, MAXDATASIZE, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }

        buffer[numbytes] = '\0';
        len = numbytes;
        memset(tempWord, 0, 1024);
        int n = 0;
        for(int x = 0; x < len; x++)
        {
            printf("%02X ", buffer[x]);
            n += sprintf(&tempWord[n], "%02X", buffer[x]);
        }

        int length = strlen(tempWord);
        printf("length: %i\n", length);
        //since the CMRx length is variable, the arrays need to be reset
        memset(newWord, 0, 1024);

        for(int i = 0, j = 0; i < length; i++, j++)
        {
            if(i == 0 || i%2 == 0)
            {
                newWord[j]= ' ';
                newWord[j+1] = '0';
                newWord[j+2] = 'x';
                newWord[j+3] = tempWord[i];
                j += 3;
                continue;
            }
            newWord[j] = tempWord[i];
        }

        char command[1024] = {""};
        strcpy(command, "sudo iw dev wlan0 vendor recv 0xFCFFAA 0xF0");
        strcat(command, newWord);
        printf("Command: %s \n", command);
        system(command);

    }

    close(sockfd);

    return 0;
}
