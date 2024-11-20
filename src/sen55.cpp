#include "sen55.h"

void printSen55ModuleVersions() {

    uint16_t error;
    char errorMessage[256];

    unsigned char productName[32];
    uint8_t productNameSize = 32;

    error = sen5x.getProductName(productName, productNameSize);

    if (error) printSen55ErrorMessage("Error trying to execute getProductName():", error);
    else {
      printf("ProductName: %s\n", productName);
    }

    uint8_t firmwareMajor;
    uint8_t firmwareMinor;
    bool firmwareDebug;
    uint8_t hardwareMajor;
    uint8_t hardwareMinor;
    uint8_t protocolMajor;
    uint8_t protocolMinor;

    error = sen5x.getVersion(firmwareMajor, firmwareMinor, firmwareDebug,
                             hardwareMajor, hardwareMinor, protocolMajor,
                             protocolMinor);
    if (error) printSen55ErrorMessage("Error trying to execute getVersion():", error);
    else {
			printf("Firmware: %d.%d, Hardware: %d.%d\n", firmwareMajor, firmwareMinor, hardwareMajor, hardwareMinor);
    }
}

void printSen55SerialNumber() {
    uint16_t error;
    char errorMessage[256];
    unsigned char serialNumber[32];
    uint8_t serialNumberSize = 32;

    error = sen5x.getSerialNumber(serialNumber, serialNumberSize);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("SerialNumber:");
        Serial.println((char*)serialNumber);
    }
}

void printSen55ErrorMessage(char *msg, uint16_t error)
{
	char errorMessage[256];
  errorToString(error, errorMessage, 256);
  printf("%s %s\n,", msg, errorMessage);
}
