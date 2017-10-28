/**
 * @file i2c.c
 * @brief Library with functions to reading an i2c device.
 *
 * @author Matteo Stringher (<matteo.stringher@studenti.unipd.it>)
 * @version 1.0
 * @since 1.0
 *
 */


/** 
 * @brief Read multiple registers
 *
 * @param regAddr First register to read
 * @param length Number of registers to read
 * @param buffer Container of data
 *
 * @return Number of bytes read
 */ 
int8_t readRegisters(uint8_t regAddr, uint8_t length, uint8_t *buffer);

/** 
 * @brief Read single register
 *
 * @param regAddr Register to read
 * @param buffer Container for byte value read from device
 * 
 * @return Status of read operation: 1 = 1 read register, 0 = error
 */ 
int8_t readRegister(uint8_t regAddr, uint8_t *buffer);

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
int8_t readBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);

/** 
 * @brief Read a single bit from an 8 bit register
 * 
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param data Container for right-aligned value
 *
 * @return Status of read operation (true = success)
 */ 
int8_t readBit(uint8_t regAddr, uint8_t bitStart, uint8_t *data);

/** 
 * @brief Write multiple bytes to an 8-bit device register.
 *
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param buffer Buffer to copy new data from
 
 * @return Number of written registers
 */ 
int8_t writeRegisters(uint8_t regAddr, uint8_t length, uint8_t* buffer);

/** Write single byte to an 8-bit device register.
 *
 * @param regAddr Register address to write to
 * @param data New byte value to write
 *
 * @return Status of operation (1 = OK)
 */
int8_t writeRegister(uint8_t regAddr, uint8_t data);

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
int8_t writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);

/** Edit a single bit in a 8 bit register
 *
 * @param regAddr Register regAddr to write to
 * @param bit Bit to edit
 * @param data Right-aligned value to write
 *
 * @return Status of operation (1 = success)
 */
int8_t writeBit(uint8_t regAddr, uint8_t bit, uint8_t data);
