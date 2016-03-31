
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define  BROADCAST_ADDRESS        "000000000000FFFF"      // Define broadcast address
#define  LAB4_SOURCE_MY_ID        "0B0B"                  // Define sourve address
#define  LAB4_DESTINATION_MY_ID   "0A0A"                  // Define destination address
#define  NODE_IDENT               "NODE_2"                // Define node identifier


packetXBee   *packet;                                   
char         data_out[52];                              // Package payload
uint8_t      PANID[2]           = {0x07, 0xF8};         // Define PANID as OX07F8
uint8_t      SOURCE_HB          = 0x0B;                 // High Byte of SOURCE_ADDRESS
uint8_t      SOURCE_LB          = 0x0B;                 // Low Byte of SOURCE_ADDRESS 
uint8_t      CHANNEL            = 0x11;                 // Define Channel number as 0x11 = 17


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
  RTC.setTime("16:03:12:06:19:30:00");
  // ------------------------------------------------------------------------
}



void build_packet(char *data_in)
{
  float temperature = RTC.getTemperature();
  
  char temp_string[5];
  Utils.float2String(temperature, temp_string, 1);
  snprintf(data_out, sizeof(data_out), "%s%s%s", data_in, temp_string, RTC.getTime());
  
  XBee.print(data_in);
  XBee.print(", ");
  XBee.print(temperature);
  XBee.print(", ");
  //XBee.println(get_epoch());
  XBee.println(data_out);
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

        XBee.println("Incomming:");
        int datalen = (int)xbee802.packet_finished[xbee802.pos-1]->data_length;
        XBee.print("  Datalength: ");
        XBee.println(datalen);

        char *data_in = (char *)xbee802.packet_finished[xbee802.pos-1]->data; 

        XBee.print("  Data: ");
        XBee.println(data_in);

        XBee.print("  RSSI: ");
        XBee.println(xbee802.packet_finished[xbee802.pos-1]->RSSI, DEC);
        XBee.println();

        XBee.println("Sending packet...");
        build_packet(data_in);  
        send_packet();

        free(xbee802.packet_finished[xbee802.pos-1]);   
        xbee802.packet_finished[xbee802.pos-1] = NULL;
        xbee802.pos--;
      }      

    } 
    else {
      XBee.println("Error receiving packet");  
    }

  }
}






