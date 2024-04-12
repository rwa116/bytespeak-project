#include "hal/nfc.hpp"

/**
 * @brief Constructs an NFCReader object.
 *
 * This constructor initializes the NFCReader object with the specified device and address.
 * It also initializes the file descriptor and sends a setup command to the NFC reader.
 *
 * @param device The device name or path.
 * @param address The address of the NFC reader.
 */
NFCReader::NFCReader(const char *device, int address, ShutdownManager *shutdownManager)
    : device(device), fileDescriptor(-1), address(address), shutdownManager(shutdownManager)
{
    // config pin
    system("config-pin P9_19 i2c");
    system("config-pin P9_20 i2c");

    
    fileDescriptor = initI2C();

    unsigned char response[256];
    unsigned char setupCommand[] = {
        0x00, // Reserved byte
        0x00, // Reserved byte
        0xFF, // Start of packet
        0x03, // Packet length
        0xFD, // Length checksum (0x100 - 0x03 = 0xFD)
        0xD4, // Data Exchange command

        0x14, // SAMConfiguration command
        0x01, //

        0x17, // Checksum for the command bytes (0x100 - 0xD4 - 0x14 - 0x01 = 0x17)
        0x00, // Reserved byte
    };

    if (sendCommandAndWaitForResponse(setupCommand, sizeof(setupCommand), response, sizeof(response), false))
    {
        std::cout << "NFC Reader setup complete" << std::endl;
    } else {
        std::cerr << "Failed to setup the NFC reader" << std::endl;
    }
}


NFCReader::~NFCReader()
{
    if (fileDescriptor >= 0)
    {
        close(fileDescriptor);
    }
}



/**
 * @brief Initializes the I2C bus for the NFC reader.
 * 
 * This function opens the I2C bus and sets the slave address for communication with the NFC reader.
 * 
 * @return The file descriptor of the opened I2C bus, or -1 if an error occurred.
 */
int NFCReader::initI2C()
{
    int file;
    if ((file = open(device, O_RDWR)) < 0)
    {
        std::cerr << "Failed to open the I2C bus" << std::endl;
        return -1;
    }
    if (ioctl(file, I2C_SLAVE, address) < 0)
    {
        std::cerr << "Failed to acquire bus access and/or talk to slave." << std::endl;
        close(file);
        return -1;
    }
    return file;
}




/**
 * Sends a command to the NFC reader and waits for a response.
 *
 * @param command The command to send.
 * @param commandLength The length of the command.
 * @param response The buffer to store the response.
 * @param responseLength The length of the response buffer.
 * @param need_response Flag indicating whether a response is needed.
 * @return True if the command was sent successfully and a response was received, false otherwise.
 */
bool NFCReader::sendCommandAndWaitForResponse(unsigned char *command, int commandLength, unsigned char *response, int responseLength, bool need_response)
{
    if (write(fileDescriptor, command, commandLength) != commandLength)
    {
        std::cerr << "Failed to write to the I2C bus." << std::endl;
        return false;
    }

    usleep(250000); // 25 milliseconds

    // Now check for ACK
    unsigned char ackBuffer[7];
    if (read(fileDescriptor, ackBuffer, sizeof(ackBuffer)) != sizeof(ackBuffer) ||
        ackBuffer[1] != 0x00 || ackBuffer[2] != 0x00 || ackBuffer[3] != 0xFF ||
        ackBuffer[4] != 0x00 || ackBuffer[5] != 0xFF || ackBuffer[6] != 0x00)
    {
        std::cerr << "ACK frame not received." << std::endl;
        // print the received data
        for (unsigned int i = 0; i < sizeof(ackBuffer); i++)
        {
            std::cout << "0x" << std::hex << (int)ackBuffer[i] << " ";
        }
        return false;
    }

    sleep(1);   // give her some time

    if (!need_response)
    {
        return true;
    }

    int bytesRead;
    int i = 0;
    do
    {   
        usleep(10000); // Wait for 10 milliseconds before retrying
        bytesRead = read(fileDescriptor, response, responseLength);
        i++;
        
        if (bytesRead < 0)
        {
            std::cerr << "Failed to read from the device." << std::endl;
            return false;
        }

    } while (bytesRead == 0 || (int)response[0] == 0 || !shutdownManager->isShutdown());


 
    return true;
}



/**
 * @brief The main function of the wait for card, will poll the NFC reader and return results.
 * 
 * @return The UID of the card that was detected.
 */
std::string NFCReader::waitForCardAndReadUID()
{
    unsigned char detectCardCommand[] = {
        0x00, // Reserved byte
        0x00, // Reserved byte
        0xFF, // Start of packet
        0x04, // Packet length
        0xFC, // Length checksum (0x100 - 0x04 = 0xFC)
        0xD4, // Data Exchange command
        0x4A, // InListPassiveTarget command
        0x01, // Max number of targets
        0x00, // Baud rate
        0xE1, // Checksum for the command bytes (0x200 - 0xD4 - 0x4A - 0x01 - 0x00 = 0xE1)
        0x00, // Reserved byte
    };

    unsigned char response[256];
    std::string uid;

    while (true)
    {
        if (sendCommandAndWaitForResponse(detectCardCommand, sizeof(detectCardCommand), response, sizeof(response), true))
        {
            // Check for a valid response length before processing
            if (response[3] > 0)
            {                           // assuming response[3] contains the length of the coming data
                int uidStartIndex = 11; // The index where UID starts in the response might need adjustment
                int uidLength = 4;      // Adjust based on actual UID length
                std::stringstream ss;
                for (int i = uidStartIndex; i < uidStartIndex + uidLength; i++)
                {
                    ss << std::hex << std::setfill('0') << std::setw(2) << (int)response[i];
                    if (i < uidStartIndex + uidLength - 1)
                    {
                        ss << ":";
                    }
                }
                uid = ss.str();
                break;
            }
        }
        sleep(1);
    }

    return uid;
}
