/**
 * @file i2c.c
 * @brief Library with functions to reading an i2c device.
 *
 * @author Matteo Stringher (<matteo.stringher@studenti.unipd.it>)
 * @version 1.0
 * @since 1.0
 *
 */


/* Library for printf, fprint functions */
#include <stdio.h>

/* Defines for integer types. For example, the most used in this library int8_t */
#include <stdint.h>

/* libc include for the memory functions */
#include <stdlib.h>

/* Library for open, read, write, close functions */
#include <fcntl.h>

/* Include for stderr format */ 
#include <unistd.h>

/* Manipulate strings */
#include <string.h>

/* Give details about the errors, while accessing to the kernel */
#include <errno.h>

/* Containing macros for ioctl */
#include <sys/ioctl.h>

/* Kernel's library for i2c */
#include <linux/i2c-dev.h>

/* Used for timing functions */ 
#include <sys/types.h>
#include <sys/stat.h>

/* Header for this library */
#include "i2c.h"
 
/*  Address of the device.
	Write "i2cdetect -y 1" on command line to see all the attached device. */
#define ADDR 0x68

/* Here devices are controlled by the kernel */
#define I2C "/dev/i2c-1"


/** 
 * @brief Read multiple registers
 *
 * @param regAddr First register to read
 * @param length Number of registers to read
 * @param buffer Container of data
 *
 * @return Number of bytes read
 */ 
int8_t readRegisters(uint8_t regAddr, uint8_t length, uint8_t *buffer) {

    // Number of read registers
    int8_t i = 0;

    // File descriptor
    int fd = open(I2C, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return -1;
    }
    
    // Select the device as slave
    if (ioctl(fd, I2C_SLAVE, ADDR) < 0) {
        fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    // Write on the line the first register we want to read. 1 is the nuber of register to write
    if (write(fd, &regAddr, 1) != 1) {
        fprintf(stderr, "Failed to write reg: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    // Read the registers 
    i = read(fd, buffer, length);
    
    if (i < 0) {
        fprintf(stderr, "Failed to read device: %s\n", strerror(errno));
        close(fd);
        return -1;
    } else if (i != length) {
        fprintf(stderr, "Error: failed to read all the registers.\n");
        close(fd);
        return -1;
    }
    
    close(fd);

    return i;
}

/** 
 * @brief Read single register
 *
 * @param regAddr Register to read
 * @param buffer Container for byte value read from device
 * 
 * @return Status of read operation: 1 = 1 read register, 0 = error
 */ 
int8_t readRegister(uint8_t regAddr, uint8_t *buffer) {
    int a = readRegisters(regAddr, 1, buffer);
    return a;
}

/** 
 * @brief Read multiple bits from an 8-bit device register.
 *
 * @detail 7th bit is the most significative. Reading goes from the start bit to the right
 *	
 *
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value 
 *
 * @return Status of read operation (true = success)
 */ 
int8_t readBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data) {
    // MSB is bit 7
    // LSB is bit 0
    // Reading goes from bitStart to the right
    
    if (length > 8)
    	printf("Too many bits asked!\n");
    	
    uint8_t i, b;
    
    if ((i = readRegister(regAddr, &b)) != 0) {
    	// Create a mask. Just like the subnet mask.
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        // Use the mask with a logic AND bit to bit
        b &= mask;
        // Move data to the right
        b >>= (bitStart - length + 1);
        // Save data on the variable linked by the pointer
        *data = b;
    }
    
    return i;
}

/** 
 * @brief Read a single bit from an 8 bit register
 * 
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param data Container for right-aligned value
 *
 * @return Status of read operation (true = success)
 */ 
int8_t readBit(uint8_t regAddr, uint8_t bitStart, uint8_t *data) {
    return readBits(regAddr, bitStart, 1, data);
}

/** 
 * @brief Write multiple bytes to an 8-bit device register.
 *
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param buffer Buffer to copy new data from
 
 * @return Number of written registers
 */ 
int8_t writeRegisters(uint8_t regAddr, uint8_t length, uint8_t* buffer) {
    int8_t i = 0;
    uint8_t buf[128];
    int fd;

    if (length > 127) {
        fprintf(stderr, "Byte write count (%d) > 127\n", length);
        return -1;
    }
	// Get the file descriptor
    fd = open(I2C, O_RDWR);
    
    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return -1;
    }
    // Select the device as slave
    if (ioctl(fd, I2C_SLAVE, ADDR) < 0) {
        fprintf(stderr, "Failed to select device: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    // I2C wants the register address as the first data to send on the line
    buf[0] = regAddr;
    
    // Copy data from buffer to buf
    memcpy(buf+1,buffer,length);
    
    // Write on the registers
    i = write(fd, buf, length + 1);
    
    if (i < 0) {
        fprintf(stderr, "Failed to write device(%d): %s\n", i, strerror(errno));
        close(fd);
        return -1;
    } else if (i != length + 1) { //
        fprintf(stderr, "Short write to device, expected %d, got %d\n", length + 1, i);
        close(fd);
        return -1;
    }
    close(fd);

    return i;
}

/** Write single byte to an 8-bit device register.
 *
 * @param regAddr Register address to write to
 * @param data New byte value to write
 *
 * @return Status of operation (1 = OK)
 */
int8_t writeRegister(uint8_t regAddr, uint8_t data) {
    return writeRegisters(regAddr, 1, &data);
}

/** 
 * @brief Write multiple bits in an 8-bit device register.
 * @detail Firstly, the register is read and modified, then overwritten.
 *
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write
 * @param data Right-aligned value to write
 *
 * @return Status of operation (1 = success)
 */ 
int8_t writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    
    // MSB is bit 7
    // LSB is bit 0
    
    if (length > 8)
    	printf("Too many bits asked!\n");
    	
    uint8_t b;
    
    /*
    	We should pay attenction here. 
    	We don't want to overwrite the whole register. So we first read the register and after write the right bits.
    */
    if (readRegister(regAddr, &b) != 0) {
    	// Create a mask
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        
        // Shift data into correct position
        data <<= (bitStart - length + 1); 
        
        // Zero all non-important bits in data
        data &= mask;
        
        // zero all important bits in existing byte
        b &= ~(mask); 
        
        // combine data with existing byte
        b |= data; 
        
        return writeRegister(regAddr, b);
    } 
    else 
        return -1;
}

/** Edit a single bit in a 8 bit register
 *
 * @param regAddr Register regAddr to write to
 * @param bit Bit to edit
 * @param data Right-aligned value to write
 *
 * @return Status of operation (1 = success)
 */
int8_t writeBit(uint8_t regAddr, uint8_t bit, uint8_t data) {
    return writeBits(regAddr, bit, 1, data);
}
