
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define  BROADCAST_ADDRESS    "000000000000FFFF"  // Define broadcast address
#define  SOURCE_ADDRESS       "0A0A"              // Define source address
//#define  DEST_ADDRESS         "0B0B"              // Define destination address
#define  NODE_IDENT           "MASTER"            // Define node identifier


#include "WProgram.h"
void setup(void);
void convert_to_lux(void);
void loop(void);
packetXBee   *paq_sent;
char         light_data_string[7];                      // Light sensor data string
char         light_lux_string[9];                       // Luminosity string
char         temperature_data_string[5];                // Temperature data string
char         time_data_string[28];                      // Timestamp string
uint8_t      PANID[2]           = {0x07, 0xF8};         // Define PANID as OX07F8
uint8_t      SOURCE_HB          = 0x0A;                 // High Byte of SOURCE_ADDRESS
uint8_t      SOURCE_LB          = 0x0A;                 // Low Byte of SOURCE_ADDRESS 
uint8_t      CHANNEL            = 0x11;                 // Define Channel number as 0x11 = 17


void setup(void)
{
  // ---------------------------- NETWORK CONFIG ----------------------------
  // Initialize the XBee library
  xbee802.init(XBEE_802_15_4, FREQ2_4G, NORMAL);
  // Power on the XBee
  xbee802.ON();
  // Set own 16-bit network address
  xbee802.setOwnNetAddress(SOURCE_HB, SOURCE_LB);
  // Set Node Identifier 
  xbee802.setNodeIdentifier(NODE_IDENT);
  // Set cahnnel
  xbee802.setChannel(CHANNEL);
  // Set PANID
  xbee802.setPAN(PANID);
  // Disable security
  xbee802.encryptionMode(0); 
  // Store values in non-volitile memory
  xbee802.writeValues();
  // ------------------------------------------------------------------------
  
  // ---------------------------- OTHER CONFIGS -----------------------------
  // Clearing terminal at startup
  XBee.print('\x0C');
  XBee.println("                                                          ");
  XBee.println("                                                          ");
  XBee.println("                                                          ");
  XBee.println("                                                          ");
  XBee.println("                                                          ");
  XBee.print('\x0C');
  // Printin start message
  XBee.println("-----------------------------------");
  XBee.println("\r\n   Sensor Network Project 2016");
  XBee.println("             Group 7");
  XBee.println("          Petter Haugen");
  XBee.println("         Jorgen R. Hoem");
  XBee.println("       Erlend R. Myklebust");
  XBee.println("\r\n-----------------------------------");
  // ------------------------------------------------------------------------
}


void convert_to_lux(void)
{
  // Declare local variables
  char       temp_string[32];
  float      vcc                = 3.3;      // 3.3V supply on Events board
  float      r2                 = 10000.0;  // 10 kOhm pull down resistor on socket 3 and 6
  float      light_in_lux       = 0;
  float      light_data_float   = 0;
  float      sensor_resistance  = 0;
  
  
  uint32_t   light_in_lux_int   = 0;
  
  // Convert light_data to integer
  light_data_float = atof(light_data_string);
  
  // Find the resistance of the sensor
  sensor_resistance = ((vcc * r2)/light_data_float) - r2;
  
  if (sensor_resistance == 0)
  {
    // Write error message to light_lux
    sprintf(light_lux_string, "Sensor error!");
  }
  else
  {
    // Convert resistance value to lux value
    light_in_lux = 100000.0 / sensor_resistance;
    
    // Convert lux value to string
    Utils.float2String(light_in_lux, light_lux_string, 2);
  }
}


void loop(void)
{
  // Declare local variables
  uint8_t i = 0;                // Counter variable 
  
  // If something is received on the XBee
  if (XBee.available())
  {
    // Process recieved data
    xbee802.treatData();
    
    // If there are no errors
    if (!xbee802.error_RX)
    {
      
      while (xbee802.pos > 0)
      {
        // Get light sensor data from received package
        for (i=0; i<6; i++) light_data_string[i] = xbee802.packet_finished[xbee802.pos-1]->data[i];
        light_data_string[6] = '\0';
        
        // Get temperature sensor data from received package
        for (i=0; i<4; i++) temperature_data_string[i] = xbee802.packet_finished[xbee802.pos-1]->data[i+6];
        temperature_data_string[4] = '\0';
        
        // Get timestamp from received package
        for (i=0; i<27; i++) time_data_string[i] = xbee802.packet_finished[xbee802.pos-1]->data[i+10];
        time_data_string[27] = '\0';
        
        // Convert light sensor data from voltage to lux
        convert_to_lux();
        
        XBee.println("\r\nNew measurements received!");
        XBee.print("Luminosity: ");
        XBee.print(light_lux_string);
        XBee.println(" lux");
        XBee.print("Temperature: ");
        XBee.print(temperature_data_string);
        XBee.println(" *C");
        XBee.println(time_data_string);
        XBee.println("\r\n-----------------------------------");
        
        // Free buffer and package
        free(xbee802.packet_finished[xbee802.pos-1]);   
        xbee802.packet_finished[xbee802.pos-1] = NULL;
        xbee802.pos--;
      }
      
    }
    
    else
    {
      XBee.println("Error receving package...\r\n");
    }  
    
  }
  
}

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

