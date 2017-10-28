/**
 * @file server.c
 * @brief Read data from MPU-6050 on the raspberry, make packets and send them to a client.
 * @detail This server can send data either with TCP or UDP.
 * 
 * @author Matteo Stringher (<matteo.stringher@studenti.unipd.it>)
 * @version 1.0
 * @since 1.0
 *
 */
 
/* Standard library for input/output operations */
#include <stdio.h>

/* This program needs exit(), NULL and other some imporant utilities */
#include <stdlib.h>

/* Library for usleep(), close(), fork() */
#include <unistd.h>

/* #defines for integer types */
#include <stdint.h>

/* Maniuplate strings */
#include <string.h>
#include <ctype.h>

/* Timing library for delay() */
#include <time.h>

/* Defines the integer variable errno */
#include <errno.h>

/* Macros and functions for TCP/UDP */
#include <sys/types.h>
#include <sys/socket.h>

/* Macros for in_addr */
#include <netinet/in.h>

/* ake available the type in_port_t and the type in_addr_t */
#include <netdb.h>

/* Convert IP addresses from a dots-and-number string to a struct in_addr and back */
#include <arpa/inet.h>

/* Signal Manipulation */
#include <signal.h>

/* i2c library for collecting data */
#include "i2c.h" 

/* Header containing important informations from datasheet and declaration of functions */
#include "server.h"

/* Port used for exchanging data in TCP */
#define PORT "3490"

/* Port used for exchanging data in UDP */
#define UDP_PORT "4950"

/* Max pending connections. Not useful we establish only one conncection. Number of connections allowed on the incoming queue */
#define BACKLOG 10

/* Number of structures contained in one packet */
#define SIZE_PACKET 500

#define DEBUG

/* Just a buffer to keep data read from the device */
uint8_t buf[100];

/* Struct where data is stored before sending */

/** @struct accData
 *  @brief Struct where data is stored before sending.
 *  Acceleration over x,y,z-axis
 *
 */
struct accData { 
	int16_t ax, ay, az;
};

/** @struct gyroData
 *  @brief Struct where data is stored before sending. 
 *  Gyroscope measurements over x,y,z-axis
 */
struct gyroData { 
	int16_t gx, gy, gz;
} gyroData;

/**
 * @brief Set some important settings and disable sleep mode. Also prints an error if the number of the device isn't right.
 * @detail Set:
 * - source of the clock
 * - limit of the gyroscope to 250°/s
 * - limit of accelerometer to 2g
 * - sleep bit to 0
 * 
 * @return nothing
*/
void startMPU() {
	// Set clock source
	writeBits(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, MPU6050_CLOCK_PLL_XGYRO);
	
	// Set limits gyrocope
	writeBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS_250);
	
	#ifdef RESET
		writeBit(0x6B, 7, 1);
	#endif
	
	// Set limits acc
	writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
	
	// Enable MPU6050
	writeBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, 0);
	
	// Testing connection
	uint8_t device = readBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buf);
	
	if (buf[0] != 0x34)
		printf("Error connecting to MPU-6050!\n");
	else
		printf("Connection: OK\n");
}


/**
 * @brief Get data from the registers and store it into one structure
 * 
 * @return Structure accData with all the data	
*/

struct accData readAcc() {
	struct accData data;
    if (readRegisters(0x3B, 6, buf) != -1) {
		data.ax = (((int16_t) buf[0]) << 8) | buf[1];
		data.ay = (((int16_t) buf[2]) << 8) | buf[3];
		data.az = (((int16_t) buf[4]) << 8) | buf[5];
	}
	
	return data;
}

/**
 * @brief Get data from the registers and store it into one structure
 * 
 * @return Structure gyroData with all the data	
*/
struct gyroData readGyro() {
	struct gyroData data;
    if (readRegisters(0x3B, 6, buf) != -1) {
		data.gx = (((int16_t) buf[0]) << 8) | buf[1];
		data.gy = (((int16_t) buf[2]) << 8) | buf[3];
		data.gz = (((int16_t) buf[4]) << 8) | buf[5];
	}
	
	return data;
}

/**
 * @brief Collects data and fills the array
 *
 * @param *packet Pointer where data is stored
 *
 * @return Nothing
*/
void makePacket(int16_t packet[]) {
	int i, j;
	for (i = 0; i < SIZE_PACKET; i++) {
		struct accData a = readAcc();
		struct gyroData b = readGyro();
		packet[i * 6 + 0] = a.ax;
		packet[i * 6 + 1] = a.ay;
		packet[i * 6 + 2] = a.az;
		packet[i * 6 + 3] = b.gx;
		packet[i * 6 + 4] = b.gy;
		packet[i * 6 + 5] = b.gz;
		usleep(1000);
	}
	
}


/**
 * @brief Get address from a node
 *
 * @param struct sockaddr *sa
 * 
 * @return Pointer to the address
*/
void *get_in_addr(struct sockaddr *sa) {

	if (sa->sa_family == AF_INET) {
		return &(( (struct sockaddr_in*) sa)->sin_addr);
	} else {
		return &(( (struct sockaddr_in6*) sa)->sin6_addr);
	}
}

/**
 * @brief Reads data from MPU-6050. It keeps sending data.
 * The user is asked to use TCP or UDP. If user chooses TCP, he must fill informations about IP address. 
 * 
 * @return Should return 0
*/
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
		printf("%d\n", strcmp(reply, "tcp") == 0);
		
		printf("%d\n", strcmp(reply, "udp") == 0);
		if (strcmp(reply, "tcp") == 0)
			tcp = 1;
		else if (strcmp(reply, "udp") == 0)
			udp = 1;
	}
	
	startMPU();
	
	// Just a buffer where data is stored
		int16_t packet[SIZE_PACKET * 6];
		
	/* Socket for listening in TCP, while it is used to send data over a port in UDP */
	int sockfd; 
	
	/* Socket for exchaning data in UTP */
	int data_fd;
	
	int yes = 1;
	
	/* hints is used to fill some basic informations */
	struct addrinfo hints, *servinfo, *p;
	// Connector's address information
	struct sockaddr_storage their_addr; 
	// Size of the socket 	
	socklen_t sin_size;
	// IP address. The size is maximum, in case of IPv6
	char s[INET6_ADDRSTRLEN];
	// Just an int to save the output from gettaddrinfo()
	int rv;
	
	// Set to 0 all the fields
	memset(&hints, 0, sizeof hints);
	
	/* Fill some basic informations */
	
	// IPv4 or 6. It works anyway
	hints.ai_family = AF_UNSPEC;
	
	
	if (tcp == 1) {
		printf("TCP has ben chosen!\n");
		
		// We choose to use TCP
		hints.ai_socktype = SOCK_STREAM;
		// Use my IP
		hints.ai_flags = AI_PASSIVE;
		/* 	getaddrinfo() now fills all other informations on the pointer servinfo.
			servinfo is a linked list with all kinds of address information.
		*/
		if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
		// sockfd = tryToBind(servinfo, p);
		// loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next) {
		    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		        perror("server: socket");
		        continue;
		    }

		    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		        perror("setsockopt");
		        exit(1);
		    }

		    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		        close(sockfd);
		        perror("server: bind");
		        continue;
		    }

		    break;
		}
		
		// We don't need it anymore
		freeaddrinfo(servinfo); 

		if (p == NULL)  {
			fprintf(stderr, "Server: failed to bind.\n");
			exit(1);
		}

		if (listen(sockfd, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}

		printf("Server: waiting for connections...\n");
	
	
		while (1) {  
		
			sin_size = sizeof their_addr;
			/*
				It'll return to you a new socket to send data and receive acks.
			*/
			data_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		
			if (data_fd == -1) {
				perror("It hasn't be accepted");
				continue;
			}
		
			/* Convert the address of the client to make it readable and print it. */
			inet_ntop(their_addr.ss_family, get_in_addr( (struct sockaddr *) &their_addr), s, sizeof s);
			printf("Server: got connection from %s\n", s);
		
			while (1) {
				makePacket(packet);
				int i;
				#ifdef DEBUG
				printf("Nuovo pacchetto");
				for (i = 0; i < SIZE_PACKET; i++) {
					double ax_d = (((double) packet[i * 6 + 0]) / 16384) * 2 * 9.81;
					double ay_d = (((double) packet[i * 6 + 1]) / 16384) * 2 * 9.81;
					double az_d = (((double) packet[i * 6 + 2]) / 16384) * 2 * 9.81;
					printf("ax:	%f, ay: %f, az: %f\n", ax_d, ay_d, az_d);
				}
				#endif
			
				if (!fork()) { // Child process
					close(sockfd); // child doesn't need the listener
					if (send(data_fd, packet, SIZE_PACKET * 6 * sizeof(int16_t), 0) == -1)
						perror("send");
					close(data_fd);
					exit(0);
				}
		
			}
			
			// parent doesn't need this
			close(data_fd);  
		}
	}
	
	else if (udp == 1) {
		printf("UDP has ben chosen!\n");
		
		char *ip_address = (char *) malloc(41 * sizeof(char));
		printf("Write client's IP.\n");
		scanf("%40s", ip_address);
		
		// Select UDP
		hints.ai_socktype = SOCK_DGRAM;

		if ((rv = getaddrinfo(ip_address, UDP_PORT, &hints, &servinfo)) != 0) {
		    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		    return 1;
		}

		/*
			Trying to create a socket. We don't need anymore to bind on a port with UDP. 
		*/
		for (p = servinfo; p != NULL; p = p->ai_next) {
		    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		        perror("Talker: i got an error trying to create the socket");
		        continue;
		    }

		    break;
		}

		if (p == NULL) {
		    fprintf(stderr, "Talker: failed to create socket\n");
		    return 2;
		}
		
		// We don't need it anymore.
		freeaddrinfo(servinfo);
		
		int numbytes;
		
		while(1) {
						
			makePacket(packet);
			#ifdef DEBUG
				printf("Nuovo pacchetto inviato");
				int i;
				for (i = 0; i < SIZE_PACKET; i++) {
					double ax_d = (((double) packet[i * 6 + 0]) / 16384) * 2 * 9.81;
					double ay_d = (((double) packet[i * 6 + 1]) / 16384) * 2 * 9.81;
					double az_d = (((double) packet[i * 6 + 2]) / 16384) * 2 * 9.81;
					printf("ax:	%f, ay: %f, az: %f\n", ax_d, ay_d, az_d);
				}
				#endif
			if ((numbytes = sendto(sockfd, packet, SIZE_PACKET * 6 * sizeof(int16_t), 0, p->ai_addr, p->ai_addrlen)) == -1) {
				perror("Talker: sendto");
				exit(1);
			}
			
			#ifdef DEBUG
			printf("talker: sent %d bytes to %s\n", numbytes, ip_address);
			#endif
		}
		
		close(sockfd);
	}
	
	
	
	return 0;
}
