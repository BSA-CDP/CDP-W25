#include <Wire.h>              // For I2C communication
#include <Adafruit_Sensor.h>   // Adafruit sensor library
#include "Adafruit_BMP3XX.h"   // Library for BMP388 pressure/temperature sensor
#include <SD.h>                // SD card library for reading and writing to SD card

#define SEALEVELPRESSURE_HPA (1013.25)  // Define the sea level pressure in hPa (standard atmospheric pressure)

Adafruit_BMP3XX bmp;           // Create an object for the BMP388 sensor
File dataFile;                 // File object to handle writing data to the SD card
unsigned long startTime;       // Start time to track elapsed time

void setup() {
  // Initialize serial communication for debugging (with baud rate 115200)
  Serial.begin(115200);
  
  // Wait for the serial connection to be established (if connected to a computer)
  while (!Serial);

  // Print a message to the serial monitor to indicate that the setup has started
  Serial.println("Teensy 4.1 BMP388 Unit Test");

  // Initialize the SD card. BUILTIN_SDCARD refers to Teensy 4.1's built-in SD card slot
  if (!SD.begin(BUILTIN_SDCARD)) {
    // If SD card initialization fails, print an error message and halt execution
    Serial.println("SD card initialization failed!");
    while (1);  // Infinite loop to stop the program
  }
  Serial.println("SD card initialized.");  // SD card initialized successfully

  // Delete the existing data file if it exists
  if (SD.exists("BMP388_data.csv")) {
    SD.remove("BMP388_data.csv");  // Remove existing file
    Serial.println("Existing data file deleted.");
  }

  // Create a new CSV file on the SD card to store data
  dataFile = SD.open("BMP388_data.csv", FILE_WRITE);  // Open (or create) a file for writing
  if (dataFile) {
    // Write the header line to the CSV file (labels for the columns)
    dataFile.println("Timestamp,Temperature (C),Pressure (hPa),Altitude (m)");
  } else {
    // If the file can't be opened, print an error message and halt execution
    Serial.println("Error opening BMP388_data.csv");
    while (1);  // Infinite loop to stop the program
  }

  // Initialize the BMP388 sensor with I2C communication
  if (!bmp.begin_I2C()) {  // Default I2C pins for Teensy 4.1 are SDA (pin 18) and SCL (pin 19)
    // If sensor initialization fails, print an error message and halt execution
    Serial.println("Could not find a valid BMP388 sensor, check wiring!");
    while (1);  // Infinite loop to stop the program
  }

  // Configure BMP388 sensor settings for data acquisition
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);  // Set temperature oversampling to 8x for higher precision
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);     // Set pressure oversampling to 4x
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);        // Set the IIR filter to reduce noise
  bmp.setOutputDataRate(BMP3_ODR_100_HZ);                  // Set output data rate to 100 Hz (100 sample per second)

  startTime = millis();  // Initialize startTime when the setup is completed
}

void loop() {
  // Check if 1 minute has passed
  if (millis() - startTime >= 60000) { // Compare milliseconds to 1 minute (60000 ms)
    dataFile.close();  // Close the file after 1 minute of data logging
    Serial.println("1 minute has passed. Stopping the data logging.");
    while(1);  // Stop the program after data logging completes
  }
  
  // Take a reading from the BMP388 sensor
  if (!bmp.performReading()) {
    // If reading fails, print an error message and skip to the next loop iteration
    Serial.println("Failed to perform reading :(");
    return;
  }

  // Log data to the file only if it was opened successfully in setup()
  if (dataFile) {
    // Calculate the timestamp (in seconds) since the program started
    unsigned long timestamp = millis() / 1000;  // `millis()` returns milliseconds, so divide by 1000 to get seconds

    // Write the timestamp and sensor data to the SD card in CSV format
    dataFile.print(timestamp);               // Write the timestamp
    dataFile.print(",");                     // CSV delimiter (comma)
    dataFile.print(bmp.temperature);         // Write the temperature in Celsius
    dataFile.print(",");                     // CSV delimiter
    dataFile.print(bmp.pressure / 100.0);    // Write the pressure in hPa (Pa to hPa conversion)
    dataFile.print(",");                     // CSV delimiter
    dataFile.println(bmp.readAltitude(SEALEVELPRESSURE_HPA));  // Write the calculated altitude in meters

    // Data is written continuously, but the file is not closed until the 1-minute limit
  } else {
    // If the file can't be accessed (although it should have been opened in setup), print an error
    Serial.println("Error writing to BMP388_data.csv");
  }

  // Wait for 1 second before taking another reading
  delay(1000);  // 1000 milliseconds = 1 second
}
