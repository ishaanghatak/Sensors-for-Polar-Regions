/*
 * Filename: Host-System-Write.ino
 * Author: Ishaan Ghatak
 * Student ID: 10695869
 * Date Created: 12 March 2023
 * Last Modified: 24 April 2023
 * Description: Main file combining all components for the host system and writing to SD card.
 */

#include <ADXL345.h>
#include <File.h>
#include <GNSS.h>
#include <Wire.h>


#define TMP1075_ADDR 0x48 // I2C address of TMP1075 temperature sensor
// #define CXD5602_ADDR 0x10 // I2C address of CXD5602 microcontroller

const float pi = 3.14159265;
const float frequency = 100; // in Hz
const float amplitude = 10; // in V
const float wait_time = 1; // in seconds (wait time between readings)
const long wait_time_ms = wait_time * 1000;


ADXL345 adxl; // create an instance of the ADXL345 library
File myFile; // File object
static SpGnss Gnss;


void setup() {
  // Set up LED pins
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
  
  Wire.begin(); // initialize I2C communication (TMP1075)
  Serial.begin(115200); // initialize serial communication at 115200 baud
  adxl.powerOn(); // power on the accelerometer (ADXL345)
  
  Gnss.begin(); //initialize GNSS
  Gnss.setInterval(wait_time); //update every time interval
  Gnss.start();


while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Open the file. Note that only one file can be open at a time
  myFile = File("/Data/test.csv", FILE_WRITE);

  // If the file opened okay, write to it
  if (myFile) {
    Serial.print("Writing header to test.csv...");
    // Header
    myFile.println("Elevator 1");
    myFile.print("t(s)\tX(g)\tY(g)\tZ(g)\tT(°C)\tLAT(°)\tLON(°)\tALT(m)\tSAT(#)\tGPR\tIMD\n");
    /* Close the file */
    myFile.close();
    Serial.println("done.");
  } else {
    // If the file didn't open, print an error
    Serial.println("error opening test.csv");
  }
  
  
}

void loop() {
  
  // Mock data for ground-penetrating radar and inductive metal detection sensors (16-bit sine waves)
  float time = (float)millis() / 1000; // in seconds
  int16_t GPR = (int16_t)(amplitude * sin(2 * pi * frequency * time));
  int16_t IMD = (int16_t)(amplitude * sin(2 * pi * frequency * time + pi/2)); // out of phase by 90 degrees
  
  
  // ADXL345 - Accelerometer
  int x_raw, y_raw, z_raw;
  adxl.readXYZ(&x_raw, &y_raw, &z_raw); // read raw accelerometer values and store them in variables x_raw, y_raw, z_raw
  double xyz[3];
  double x_g, y_g, z_g;
  adxl.getAcceleration(xyz); // read accelerometer values in g-force and store them in variables x_g, y_g, z_g
  x_g = xyz[0];
  y_g = xyz[1];
  z_g = xyz[2];
  

  // TMP1075 - Temperature Sensor
  Wire.beginTransmission(TMP1075_ADDR); // start communication with TMP1075
  Wire.write(0x00); // set pointer to temperature register
  Wire.endTransmission(false); // end transmission without releasing the bus
  
  Wire.requestFrom(TMP1075_ADDR, 2); // request 2 bytes of data from TMP1075
  int16_t temp_raw = 0;
  float temp_c = 0;
  if (Wire.available() == 2) { // check if data was received
    temp_raw = (Wire.read() << 8) | Wire.read(); // combine 2 bytes of data into a signed 16-bit integer
    temp_c = raw_to_float_celsius(temp_raw); // convert raw temperature data to Celsius
  }
  
  
  // GNSS - Position/Navigation Sensor
  if (Gnss.waitUpdate(-1)) { //wait for GNSS update (Note: this uses the GNSS interval, not wait_time)
    SpNavData NavData;
    Gnss.getNavData(&NavData); //get navigation data
    float lat = NavData.latitude, lon = NavData.longitude, alt = NavData.altitude;
    int sat = NavData.numSatellites;

  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }

  // Open the file. Note that only one file can be open at a time
  myFile = File("/Data/test.csv", FILE_WRITE);

  // If the file opened okay, write to it
  if (myFile) {
    Serial.print("Writing to sensor data to test.csv...");
    // Print ALL data in a tabulated format
    myFile.print(time, 2); //elapsed time
    myFile.print("\t");
    myFile.print(x_g, 2);
    myFile.print("\t");
    myFile.print(y_g, 2);
    myFile.print("\t");
    myFile.print(z_g, 2);
    myFile.print("\t");
    myFile.print(temp_c, 2);
    myFile.print("\t");
    myFile.print(lat, 2);
    myFile.print("\t");
    myFile.print(lon, 2);
    myFile.print("\t");
    myFile.print(alt, 2);
    myFile.print("\t");
    myFile.print(sat);
    myFile.print("\t");
    myFile.print(GPR);
    myFile.print("\t");
    myFile.print(IMD);
    myFile.println("");
    // Close the file
    myFile.close();
    Serial.println("done.");
  } else {
    // If the file didn't open, print an error
    Serial.println("error opening test.csv");
  }
   
  }

// Flash LEDs to show program is running
    digitalWrite(LED0, HIGH);
    delay(100);
    digitalWrite(LED1, HIGH);
    delay(100);
    digitalWrite(LED2, HIGH);
    delay(100);
    digitalWrite(LED3, HIGH);
    delay(100);

    digitalWrite(LED0, LOW);
    delay(100);
    digitalWrite(LED1, LOW);
    delay(100);
    digitalWrite(LED2, LOW);
    delay(100);
    digitalWrite(LED3, LOW);
    delay(100);
}

float raw_to_float_celsius(int32_t x){
    x >>= 4; // Shift raw register value to form a proper Q4 value
    return ((float)x * 0.0625f); // Convert Q4 value to a float
}
