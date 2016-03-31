
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define  BROADCAST_ADDRESS    "000000000000FFFF"  // Define broadcast address
#define  SOURCE_ADDRESS       "0A0A"              // Define source address
//#define  DEST_ADDRESS         "0B0B"              // Define destination address
#define  NODE_IDENT           "MASTER"            // Define node identifier


#include "WProgram.h"
void setup(void);
void convert_duration_to_time(void);
void loop(void);
packetXBee   *paq_sent;
char         average_temperature[6];                    // Temperature string
char         duration[6];                               // Duration string
char         timestamp[32];                             // Timestamp string
uint8_t      PANID[2]           = {0x07, 0xF8};         // Define PANID as OX07F8
uint8_t      SOURCE_HB          = 0x0A;                 // High Byte of SOURCE_ADDRESS
uint8_t      SOURCE_LB          = 0x0A;                 // Low Byte of SOURCE_ADDRESS 
uint8_t      CHANNEL            = 0x11;                 // Define Channel number as 0x11 = 17
uint32_t      hours              = 0;
uint32_t      minutes            = 0;
uint32_t      seconds            = 0;


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


void convert_duration_to_time(void)
{
  // Declare local variables
  uint32_t  input       = atof(duration);
  uint32_t  remainder   = 0;
  
  // Convert number of seconds to days, hours, minutes and seconds
  hours        = input / ( 60 * 60 );
  remainder    = input % ( 60 * 60 );
  minutes      = remainder / 60;
  seconds      = remainder % 60; 
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
        // Get average temperature data from received package
        for (i=0; i<5; i++) average_temperature[i] = xbee802.packet_finished[xbee802.pos-1]->data[i];
        average_temperature[5] = '\0';
        
        // Get measurement duration from received package
        for (i=0; i<5; i++) duration[i] = xbee802.packet_finished[xbee802.pos-1]->data[i+5];
        duration[5] = '\0';
        
        // Get timestamp from received package
        for (i=0; i<31; i++) timestamp[i] = xbee802.packet_finished[xbee802.pos-1]->data[i+10];
        timestamp[31] = '\0';
        
        // Convert duraion in seconds to days, hours, minutes and seconds
        convert_duration_to_time();
        
        XBee.println("\r\nNew measurements received!");
        XBee.print("Measurement start: ");
        XBee.println(timestamp);
        XBee.print("Measurement duration: ");
        XBee.print(hours);
        XBee.print(" hr  ");
        XBee.print(minutes);
        XBee.print(" min  ");
        XBee.print(seconds);
        XBee.println(" sec");
        XBee.print("Average temperature: ");
        XBee.print(average_temperature);
        XBee.println(" *C");
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

