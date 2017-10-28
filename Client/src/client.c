/**
 * @file client.c 
 * @brief This is the client, where data is received. 
 * @detail This client can connect either in TCP or UDP
 *
 * @author Matteo Stringher (<matteo.stringher@studenti.unipd.it>)
 * @version 1.0
 * @since 1.0 
*/

/* Standard library for input/output operations */
#include <stdio.h>

/* This program needs exit(), NULL and other some important utilities */
#include <stdlib.h>

/* Macros for integer types */
#include <stdint.h>
#include <unistd.h>

/* Defines the integer variable errno */
#include <errno.h>

/* Maniuplate strings */
#include <string.h>
#include <ctype.h>

#include <netdb.h>

/* Macros and functions for TCP/UDP */
#include <sys/types.h>
#include <sys/socket.h>

/* Macros for in_addr */
#include <netinet/in.h>

/* Convert IP addresses from a dots-and-number string to a struct in_addr and back */
#include <arpa/inet.h>

// Client will be connecting here in TCP
#define PORT "3490" 

/* Port used for exchanging data in UDP */
#define UDP_PORT "4950"

// Max number of bytes we can get for each packet
#define MAXDATASIZE 6000 

#define SIZE_PACKET 500

#define DEBUG

/**
 * @brief Get address from a node
 *
 * @param struct sockaddr *sa
 * 
 * @return Pointer to the address
*/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	char reply[4];
	int tcp = 0;
	int udp = 0;
	
	// Ask the user to select TCP or UDP
	while ((tcp != 1) && (udp != 1)) {
		printf("Would you like TCP or UDP? (TCP/UDP)\n");
		
		// Read the reply and convert it to lower case
		scanf("%3s", reply);
		int i;
		for (i = 0; i < 3; i++) {
			reply[i] = tolower(reply[i]);
		}
		
		if (strcmp(reply, "tcp") == 0)
			tcp = 1;
		else if (strcmp(reply, "udp") == 0)
			udp = 1;
	}
	
	struct sockaddr_storage their_addr;
	
	/* Socket for exchanging data */
	int sockfd, numbytes;  
	
	/* A buffer where each packet is saved */
	int16_t buf[SIZE_PACKET * 6];
	
	// We need them to create the socket propertly
	struct addrinfo hints, *servinfo, *p;
	
	// Just an int to save the output from gettaddrinfo()
	int rv;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	// Reset all the fields
	memset(&hints, 0, sizeof hints);
	
	// IPv4 or 6. It works anyway
	hints.ai_family = AF_UNSPEC;
	
	if (tcp == 1) {
		
		printf("TCP has been chosen!\n");
		
		// We choose to use TCP
		hints.ai_socktype = SOCK_STREAM;
		
		char ip_address[21];
		printf("Write client's IP\n");
		scanf("%20s", ip_address);
		
		if ((rv = getaddrinfo(ip_address, PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
		
		/* 
			servinfo is a linked list.
			Loop through all the results and connect to the first we can.
		*/
		for (p = servinfo; p != NULL; p = p->ai_next) {
		
			// Try to create a socket, otherwise start again the cycle
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			// Try to connect to the server
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				perror("Client: connect");
				close(sockfd);
				continue;
			} 
		
			// If we get here we have a socket. We are ready to talk
			break;
		}

		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}
	
		/* Convert the address of the client to make it readable and print it. */	
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
		printf("client: connecting to %s\n", s);

		/* Not useful anymore */
		freeaddrinfo(servinfo);
	
		double ax_d, ay_d, az_d;
		int i;
		
		while (1) {

			if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			else {
				#ifdef DEBUG
				printf("Nuovo pacchetto.\n");
				#endif
				
				for (i = 0; i < SIZE_PACKET; i++) {
					ax_d = (((double) buf[i * 6 + 0]) / 16384) * 2 * 9.81;
					ay_d = (((double) buf[i * 6 + 1]) / 16384) * 2 * 9.81;
					az_d = (((double) buf[i * 6 + 2]) / 16384) * 2 * 9.81;
					printf("ax:	%f, ay: %f, az: %f\n", ax_d, ay_d, az_d);
				}  
			}
		}
		
		// Close the socket
		close(sockfd);
	}
	
	else if (udp == 1) {
	
		// Select UDP
		hints.ai_socktype = SOCK_DGRAM;
		// Use my IP
		hints.ai_flags = AI_PASSIVE;
		
		// Get info about 
		if ((rv = getaddrinfo(NULL, UDP_PORT, &hints, &servinfo)) != 0) {
		    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		    return 1;
		}
		
		// Loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next) {
			// Try to create a socket
		    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		        perror("listener: socket");
		        continue;
		    }
			// Try to bind 
		    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		        close(sockfd);
		        perror("listener: bind");
		        continue;
		    }
		    
			// Once here we are ready to talk
		    break;
		}

		if (p == NULL) {
		    fprintf(stderr, "listener: failed to bind socket.\n");
		    return 2;
		}
		
		// We don't need anymore
		freeaddrinfo(servinfo);
		
		printf("Listener: waiting to recvfrom...\n");
		
		// Size of server's address
		addr_len = sizeof their_addr;
		
		double ax_d, ay_d, az_d;
		int i;
		while (1) {
			if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
				perror("recvfrom\n");
				exit(1);
			}
			else {
				#ifdef DEBUG
				printf("Nuovo pacchetto ricevuto.\n");
				#endif
				for (i = 0; i < SIZE_PACKET; i++) {
					ax_d = (((double) buf[i * 6 + 0]) / 16384) * 2 * 9.81;
					ay_d = (((double) buf[i * 6 + 1]) / 16384) * 2 * 9.81;
					az_d = (((double) buf[i * 6 + 2]) / 16384) * 2 * 9.81;
					printf("ax:	%f, ay: %f, az: %f\n", ax_d, ay_d, az_d);
				}  
			}
			#ifdef DEBUG
			printf("listener: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
				    get_in_addr((struct sockaddr *)&their_addr),
				    s, sizeof s));
			printf("listener: packet is %d bytes long\n", numbytes);
			#endif
		}
		
		// Close the socket 
		close(sockfd);
	}

	return 0;
}

