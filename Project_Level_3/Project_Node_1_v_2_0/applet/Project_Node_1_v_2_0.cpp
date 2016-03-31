
#include <stdio.h>
#include <string.h>

#define  BROADCAST_ADDRESS    "000000000000FFFF"  // Define broadcast address
#define  SOURCE_ADDRESS       "0C0C"              // Define source address
#define  DEST_ADDRESS         "0B0B"              // Define destination address
#define  NODE_IDENT           "NODE_1"            // Define node identifier
#define  SOCKET_3             SENS_SOCKET3
#define  SOCKET_6             SENS_SOCKET6  


#include "WProgram.h"
void setup(void);
static void transmit_package(void);
void loop(void);
packetXBee   *paq_sent;
char         data[6];                                   // Package payload
float        light_value;                                // Temporary variable used in calculations
uint8_t      PANID[2]           = {0x07, 0xF8};         // Define PANID as OX07F8
uint8_t      SOURCE_HB          = 0x0C;                 // High Byte of SOURCE_ADDRESS
uint8_t      SOURCE_LB          = 0x0C;                 // Low Byte of SOURCE_ADDRESS 
uint8_t      CHANNEL            = 0x11;                 // Define Channel number as 0x11 = 17
float        THRESHOLD_LOW      = 3.21;                  // Define voltage threshold for light off condition
float        THRESHOLD_HIGH     = 3.22;                  // Define voltage threshold for light on condition


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
  
  // ---------------------------- SENSOR CONFIG -----------------------------
  // Activate power to the Events sensor board
  SensorEvent.setBoardMode(SENS_ON);
  // Define light sensor threshold for the sensor in socket 3
  SensorEvent.setThreshold(SOCKET_3, THRESHOLD_HIGH);
  // Define light sensor threshold for the sensor in socket 6
  SensorEvent.setThreshold(SOCKET_6, THRESHOLD_LOW);
  // ------------------------------------------------------------------------
  
  // ---------------------------- OTHER CONFIGS -----------------------------
  
  // ------------------------------------------------------------------------
}

static void transmit_package(void)
{
 // Set package parameters
  paq_sent=(packetXBee*) calloc(1,sizeof(packetXBee));
  paq_sent->MY_known  =  0;
  paq_sent->mode      =  UNICAST;
  paq_sent->packetID  =  0x07;
  paq_sent->opt       =  0;

  // Define maximum number of hops
  xbee802.hops=0;

  // Set origin parameters
  xbee802.setOriginParams(paq_sent, SOURCE_ADDRESS, MY_TYPE);

  // Set destination parameters
  xbee802.setDestinationParams(paq_sent, DEST_ADDRESS, data, MY_TYPE, DATA_ABSOLUTE);

  // Send package and write status to USART
  xbee802.sendXBee(paq_sent);
  if(!xbee802.error_TX)
  {
    XBee.println("\r\nTransmission OK");
  }
  else 
  {
    XBee.println("\r\nError: Transmission failed!");
  }
  free(paq_sent);
  paq_sent = NULL;

  // Write end string to USART
  XBee.println("\r--------------------------------"); 
}

void loop(void)
{
  // Write start string to USART
  XBee.println("\r--------------------------------");
  
  // Enable interrupts from the Events sensor board
  SensorEvent.attachInt();
  
  // Enable sleep mode
  PWR.sleep(UART0_OFF | UART1_OFF | BAT_OFF | RTC_OFF);
  
  // Re-initialize the XBee library
  xbee802.init(XBEE_802_15_4, FREQ2_4G, NORMAL);
  // Power on the XBee after sleep
  xbee802.ON();
  
  // Disable interrupts from the Events sensor board
  SensorEvent.detachInt();
  
  // Store contents of interrupt register
  SensorEvent.loadInt();
  
  // Determine which socket caused the interrupt
  if  (SensorEvent.intFlag & SOCKET_3)
  {
    // Read light sensor value (float)
    light_value = SensorEvent.readValue(SOCKET_3);
    
    // Convert to string and print results on USART
    Utils.float2String(light_value, data, 4);
    XBee.println("Light is ON");
    XBee.println(data);
    
    // Send package
    transmit_package();
    
    // Clear interrupt flag
    clearIntFlag();
  }
  
  else if  (SensorEvent.intFlag & SOCKET_6)
  {
    // Read light sensor value (float)
    light_value = SensorEvent.readValue(SOCKET_6);
    
    // Convert to string and print results on USART
    Utils.float2String(light_value, data, 4);
    XBee.println("Light is OFF");
    XBee.println(data);
    
    // Send package
    transmit_package();
    
    // Clear interrupt flag
    clearIntFlag();
  }
  
  // A delay in order to stop multiple simultaneous interrupt
  //delay(10);
}

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

