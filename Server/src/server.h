/* These are macros imported from MPU's datasheet */

#define MPU6050_RA_PWR_MGMT_1 0x6B
#define MPU6050_PWR1_CLKSEL_BIT 2
#define MPU6050_PWR1_CLKSEL_LENGTH 3
#define MPU6050_CLOCK_PLL_XGYRO 1
#define MPU6050_RA_GYRO_CONFIG 0x1B
#define MPU6050_GCONFIG_FS_SEL_BIT 4
#define MPU6050_GCONFIG_FS_SEL_LENGTH 2
#define MPU6050_GYRO_FS_250 0
#define MPU6050_RA_ACCEL_CONFIG 0x1C
#define MPU6050_ACONFIG_AFS_SEL_BIT 4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH 2
#define MPU6050_ACCEL_FS_2 0
#define MPU6050_PWR1_SLEEP_BIT 6
#define MPU6050_RA_WHO_AM_I 0x75
#define MPU6050_WHO_AM_I_BIT 6
#define MPU6050_WHO_AM_I_LENGTH 6



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
void startMPU();


/**
 * @brief Get data from the registers and store it into one structure
 * 
 * @return Structure gyroData with all the data	
*/
struct gyroData readGyro();

/**
 * @brief Get data from the registers and store it into one structure
 * 
 * @return Structure accData with all the data	
*/

struct accData readAcc();

/**
 * @brief Collects data and fills the array
 *
 * @param *packet Pointer where data is stored
 *
 * @return Nothing
*/
void makePacket(int16_t packet[]);

/**
 * @brief Get address from a node
 *
 * @param struct sockaddr *sa
 * 
 * @return Pointer to the address
*/
void *get_in_addr(struct sockaddr *sa);


