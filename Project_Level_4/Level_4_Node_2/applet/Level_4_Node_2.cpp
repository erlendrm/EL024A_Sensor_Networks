
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define  BROADCAST_ADDRESS        "000000000000FFFF"      // Define broadcast address
#define  LAB4_SOURCE_MY_ID        "0B0B"                  // Define sourve address
#define  LAB4_DESTINATION_MY_ID   "0A0A"                  // Define destination address
#define  NODE_IDENT               "NODE_2"                // Define node identifier
#define  MEASUREMENT_ON           0
#define  MEASUREMENT_OFF          1


#include "WProgram.h"
void setup();
uint32_t convert_timestamp_to_seconds(void);
void build_packet(void);
void send_packet(void);
void loop();
packetXBee   *packet;                                   
char         data_out[48];                              // Package payload
char         time_start[32];                            // Time stamp string for start of measurement
char         time_stop[32];                             // Time stamp string for end of measurement
char         duration[6];                               // Measurement duration string
char         avg_temp[5];                               // Average temperature string
float        current_temp;                              // Variable for current temperature
float        sum_temp;                                  // Variable for sum of temperatures
float        avg_temp_float;                            // Variable for average temperature
uint8_t      PANID[2]            = {0x07, 0xF8};        // Define PANID as OX07F8
uint8_t      SOURCE_HB           = 0x0B;                // High Byte of SOURCE_ADDRESS
uint8_t      SOURCE_LB           = 0x0B;                // Low Byte of SOURCE_ADDRESS 
uint8_t      CHANNEL             = 0x11;                // Define Channel number as 0x11 = 17
uint8_t      loop_identifier     = MEASUREMENT_OFF;     // Identifier for the main switch structure
uint32_t     counter             = 0;                   // Counter variable
uint32_t     start_time_seconds  = 0;                   
uint32_t     stop_time_seconds   = 0;
uint32_t     duration_seconds    = 0;


void setup()
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
  // Enable RTC for temperature reading
  RTC.ON();
  // Set RTC
  RTC.setTime("16:03:13:01:23:21:00");
  // USART debug
  XBee.println("\r\n--------------------------------");
  // ------------------------------------------------------------------------
}


uint32_t convert_timestamp_to_seconds(void)
{
  // Declare local variables 
  uint32_t  dd    = (uint32_t) RTC.date;
  uint32_t  hh    = (uint32_t) RTC.hour;
  uint32_t  mm    = (uint32_t) RTC.minute;
  uint32_t  ss    = (uint32_t) RTC.second;
  uint32_t sum   = 0;
  
  // Calculate sum
  sum = ( dd * 24 * 60 * 60 ) +
        ( hh * 60 * 60 ) +
        ( mm * 60 ) +
        ( ss );
  
  return sum;
}


void build_packet(void)
{
  // Calculate the duration of the measurement in seconds   
  duration_seconds = stop_time_seconds - start_time_seconds;
  sprintf(duration, "%.5lu", duration_seconds);
  
  // Convert average temperature to string
  Utils.float2String(avg_temp_float, avg_temp, 2);
  
  // Join the results in one string
  snprintf(data_out, sizeof(data_out), "%s%s%s", avg_temp, duration, time_start );
  
  // Print to USART
  XBee.println("\r\nMeasurement details:");
  XBee.print("Start: ");
  XBee.println(time_start);
  XBee.print("Stop: ");
  XBee.println(time_stop);
  XBee.print("Start in seconds: ");
  XBee.println(start_time_seconds);
  XBee.print("Stop in seconds: ");
  XBee.println(stop_time_seconds);
  XBee.print("Duration: ");
  XBee.println(duration);
  XBee.print("Temperature sum: ");
  XBee.println(sum_temp);
  XBee.print("Counter value: ");
  XBee.println(counter);
  XBee.print("Average temperature: ");
  XBee.println(avg_temp);
  XBee.println("Packet contents: ");
  XBee.println(data_out);
  XBee.println("\r\n--------------------------------");
}


void send_packet(void)
{
  // Set package parameters
  packet = (packetXBee *)calloc(1, sizeof(packetXBee)); 
  packet->MY_known   = 0;
  packet->mode       = UNICAST;
  packet->packetID   = 0x07;
  packet->opt        = 0; 

  // Define maximum number of hops
  xbee802.hops = 0;
  
  // Set origin parameters
  xbee802.setOriginParams(packet, LAB4_SOURCE_MY_ID, MY_TYPE);
  
  // Set destination parameters
  xbee802.setDestinationParams(packet, LAB4_DESTINATION_MY_ID, data_out, MY_TYPE, DATA_ABSOLUTE);
  
  // Send package, write status to USART and clear buffer
  xbee802.sendXBee(packet);
  if (!xbee802.error_TX) XBee.println("  Packet sent");
  else XBee.println("  Error sending packet");
  free(packet);
  packet = NULL;
}



void loop()
{
  if (XBee.available()) {

    xbee802.treatData();

    // Check for error
    if (!xbee802.error_RX) { // 0 = no ellol !

      while (xbee802.pos > 0) {
        
        // If data in received package is anything other than '1'
        if((xbee802.packet_finished[xbee802.pos-1]->data[0]) != '1') XBee.println("\r\nError: wrong packet data!");
        
        // Otherwise if it is equal to '1'
        else
        {
          switch (loop_identifier)
          {
            case MEASUREMENT_ON:
            // Reset loop identifier
            loop_identifier = MEASUREMENT_OFF;
            // Get stop time for measurement
            sprintf(time_stop, "%s", RTC.getTime());
            // Convert timestamp to seconds
            stop_time_seconds = convert_timestamp_to_seconds();
            // Build and send new packet
            build_packet();
            send_packet();
            // Reset counter and measurement variables
            counter = 0;
            sum_temp = 0;
            avg_temp_float = 0;
            break;
            
            
            case MEASUREMENT_OFF:
            // Set measurement identifier
            loop_identifier = MEASUREMENT_ON;
            // Get start time for measurement
            sprintf(time_start, "%s", RTC.getTime());
            // Convert timestamp to seconds
            start_time_seconds = convert_timestamp_to_seconds();
            // Set alarm 1 to go off in two seconds
            RTC.setAlarm1("00:00:00:02", RTC_OFFSET, RTC_ALM1_MODE2);
            // Write to USART
            XBee.println("\r\n Measurement started");
            break;
            
            default:
            // Do nothing
            break; 
          }
        }
        
        // Free buffer and package
        free(xbee802.packet_finished[xbee802.pos-1]);   
        xbee802.packet_finished[xbee802.pos-1] = NULL;
        xbee802.pos--;
      }      

    } 
    else {
      XBee.println("Error receiving packet");  
    }

  }
  
  
  switch (loop_identifier)
  {
    case MEASUREMENT_ON:
    if( intFlag & RTC_INT )
    {
      // Get current temperature
      current_temp = RTC.getTemperature();
      
      // Increment counter
      counter++;
      
      // Calculate average temperature
      sum_temp       = sum_temp + current_temp;
      avg_temp_float = sum_temp / (float) counter;
      
      // Clear the interrupt flag
      intFlag &= ~(RTC_INT);
      
      // Set alarm 1 to go off in two seconds
      RTC.setAlarm1("00:00:00:02", RTC_OFFSET, RTC_ALM1_MODE2);
      
      // Write to USART
      XBee.print("\r\nCounter: ");
      XBee.println(counter);
      XBee.print("Temperature sum: ");
      XBee.println(sum_temp);
      XBee.print("Average temperature: ");
      XBee.println(avg_temp_float);
    }
    
    break;
    
    default:
    // Do nothing
    break;
    
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

