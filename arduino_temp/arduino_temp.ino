// Arduino Uno + DS18B20 + BL999

#include <OneWire.h>
#include <DallasTemperature.h>
#include <lib_bl999.h>

// 
#define ONE_WIRE_BUS 7
#define TEMPERATURE_PRECISION 12

static BL999Info info;
int c;
float t_inside;
float t_outside;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  
  // Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  

  // set the resolution to 9 bit per device
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

 bl999_set_rx_pin(2);
 bl999_rx_start();
 if (bl999_wait_rx_max(60000)) {
     Serial.println("433mhz ok");
  }else {
    Serial.println("433mhz problem...");
    
    }
 
 c = 0;
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();    
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();

}

/*
 * Main function, calls the temperatures in a loop.
 */
void loop(void)
{ 
 
  c++;
    //read message to info and if check sum correct - outputs it to the serial port
  if (bl999_have_message() && bl999_get_message(info)) {
        Serial.println("====== Got message from BL999 sensor: ");
        Serial.println(c);
        Serial.println("Channel: " + String(info.channel));
        Serial.println("Battery is " + String(info.battery == 0 ? "Ok" : "Low"));
        t_outside = info.temperature / 10.0;
        Serial.println("Temperature: " + String(t_outside));

        Serial.print("Requesting ds18b20...");
        sensors.requestTemperatures();
        Serial.println("DONE");
        t_inside = sensors.getTempC(insideThermometer);

        
        Serial.print("TempC=");
        Serial.print(t_inside);
        Serial.print(",");
        Serial.print(t_outside);
        Serial.println(" ");
    } else {
      
        Serial.print("Requesting ds18b20...");
        sensors.requestTemperatures();
        Serial.println("DONE");
        float t_inside = sensors.getTempC(insideThermometer); 
        Serial.print("TempC=");
        Serial.print(t_inside);
        Serial.print(",");
        Serial.print(t_outside);
        Serial.println(" ");
        Serial.println("BL999 no data..");
      
      }

delay(60000);
  
}

