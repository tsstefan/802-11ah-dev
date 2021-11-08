/* ********************************************************************
 * Program to capture GNSS corrections data transmitted in a beacon's *
 * vendor specific element. The output from an iw command is recorded *
 * and transmitted to the CSG Test Suite running on a computer        * 
 * connected over Ethernet                                            *
 * ********************************************************************/
 
#include<stdio.h>
#include<stdlib.h> 
#include<string.h> //for memset
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<net/ethernet.h>
#define PORT 8888

void SendData (char *, int);

int main()
{
	unsigned char element[1024], buffer[1024], final[1024];
	FILE *fp;
	int i = 0;
	int j = 14;
	int x = 0;
	int length = 0;
	int ones = 0;		// to translate the strings from the output
	int tens = 0;		// into useable numbers
	char *p;

	fp = popen("iw event -f", "r");		// capture the stream
	
	while(fgets(buffer, sizeof(buffer), fp))	// line by line
	{
		//p = strchr(buffer, '\n');
		if(buffer[0] != 119)
		{
		j = 14; // skip the first 14 chars each line, only contain "vendor_event"
		for(i = 0; i < strlen(buffer); i++)
		{
			if(j > 62) // end of line
			{
				break;
			}
			
			// look for the start of a new vendor information element
			// there are certain specific (ASCII) characters you can find there
			
			if(buffer[i] == 118 && buffer[i+5] == 114 && buffer[i+27] == 50 && buffer[i+32] == 57)
			{
				memset(element, 0, 1024); 	// reset the array
				x = 0; 						// reset a counter
				
				//printf("%c %c\n", buffer[i+14], buffer[i+15]); // contains the length
				
				if(buffer[i+14] < 58) 	// < 58 = decimal numbers
				{
					tens = (buffer[i+14]-48)*16;
				}else{					// > 58 = hex numbers (a-f -- always seem to arrive lower case)
					switch(buffer[i+14])
					{
					case 97: // a
						tens = 160;
						break;
					case 98: // b
						tens = 176;
						break;
					case 99: // c
						tens = 192;
						break;
					case 100: // d
						tens = 208;
						break;
					case 101: // e
						tens = 224;
						break;
					case 102: // f
						tens = 240;
						break;
					}	
				}
				
				if(buffer[i+15] > 64) // second part of hexadecimal
				{
					ones = buffer[i+15] - 87; // a-f
				}else{
					ones = buffer[i+15] - 48; // 0-9
				}
				length = (tens + ones - 3)*2; // length of the vendor IE
				//printf("tens: %i, ones: %i\n", tens, ones);
				//printf("found; length: %i\n", length);
				
			}
			
			if(buffer[i] == 112 && buffer[i+4] == 35) // marks the start of a new vendor IE
			{	
				// to avoid a "dirty" first packet
				if(element[8] == 48 && element[9] == 50) // this vendor's packets always start with 02
				{
					//printf("\n%s\n", element);
					printf("Length: %i\n", strlen(element));
					
					/* ********************************************************** *
					 * the following is used to convert the ASCII representation  *
					 * of hexadecimals into its decimal bytewise representation   *
					 * bd (98 100) becomes 189 (dec representation of hex bd)     *
					 * ********************************************************** */
					
					char hexbyte[3] = {0};
					unsigned char octets[sizeof(element) / 2];
					for(int k = 0; k < strlen(element); k += 2)
					{
						hexbyte[0] = element[k];
						hexbyte[1] = element[k+1];
						
						sscanf(hexbyte, "%x", &octets[k/2]);
						
						printf("%02x ", octets[k/2]);
					}
					SendData(octets, sizeof(octets));
					memset(octets, 0, sizeof(octets)); 
				}
				memset(element, 0, 1024);
				printf("array reset\n\n");
				continue;
			}

			element[x] = buffer[j];

			if(buffer[j] == 32 || buffer[j] == 10)
			{
				x--;
			}
			
			x++;
			j++;
		}
		}
	}
	
	pclose(fp);
	return 0;
}

/* ****************************************************************** *
 * This function is used to send the data to the server maintaining a *
 * connection with the a Test Suite (needs a constant connection)     *
 * ****************************************************************** */

void SendData(char * data, int size)
{
	struct sockaddr_in address;
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) <0)
	{
		printf("\n Socket creation error \n");
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
	}

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection failed \n");
	}
	send(sock, data, size, 0);
	printf("\ndata sent\n");
	close(sock);
}
