//receiver code example, prints the received payload to the serial port
//payload length of 1 or 16 byte, 1Mbps datarate, -6 dbm rf transmit power, channel 32 of 125

extern "C"{
  #include "nrf24l01.h"
}

#define TIME_GAP      300

uint8_t ascending_number = 0;
char txt[16];


void uint8_to_binary_string(uint8_t value, char* output, int len) {
    for (int i = 0; i < len; i++) {
        output[len - i - 1] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    output[len] = '\0';
}

void print_reg(char* name, uint8_t addr) {
  Serial.print("         ");
  Serial.print(name);
  Serial.print(" register:");
  char str[9];
  uint8_t REG;
  nrf24_read(addr, &REG, 1, CLOSE);
  uint8_to_binary_string(REG, str, 8);
  Serial.println(str);
}

void print_debug() {
  print_reg("FEATURE      ", FEATURE_ADDRESS);
  print_reg("TX OBSERVE   ", OBSERVE_TX_ADDRESS);
  print_reg("STATUS       ", STATUS_ADDRESS);
  print_reg("RX_PW_P0 ADDR", RX_ADDR_P0_ADDRESS);
  print_reg("TX ADDR      ", TX_ADDR_ADDRESS);
  print_reg("EN_AA        ", EN_AA_ADDRESS);
  print_reg("EN_RXADDR    ", EN_RXADDR_ADDRESS);
}

//LED_BUILTIN is pin 13 is SCK of SPI, already using that
void led_on() {
  digitalWrite(4, HIGH);
}

void led_off() {
  digitalWrite(4, LOW);
}



uint8_t recvnumber() {
  return nrf24_receive(&ascending_number, 1);
}

uint8_t recvstr() {
  return nrf24_receive((uint8_t*)&txt, 16);
}

void receive() {
  // to switch between sending an uint8_t and a 16-byte-char-array, just uncomment one of these two:
  //uint8_t result = recvnumber();
  uint8_t result = recvstr();
  // also uncomment the corresponding one for case OPERATION_DONE
  
  //print_debug();
  switch(result) {
    case OPERATION_ERROR:
      Serial.println("EEE   RX operation error");
      break;
    case RECEIVE_FIFO_EMPTY:
      Serial.println("      RX empty");
      //Serial.print("      RX empty, last received: ");
      //Serial.println(ascending_number);
      break;
    case OPERATION_DONE:
      led_on();
      Serial.print("***   RX success, received: ");
      // pick one of these two:
      //Serial.println(ascending_number);
      Serial.println(txt);
      
      break;
  }
  delay(TIME_GAP*1/2);
  led_off();
  delay(TIME_GAP*1/2);
}


void setup()
{
  Serial.begin(115200);

  Serial.println("\n\n\n   kys!\n");

  Serial.print("\r\r\n\nspi_24L01_RX\n\r");
  
  Serial.print("initializing radio as RX...");
  nrf24_device(RECEIVER, RESET);      //initializing nrf24l01+ as a receiver device with one simple function call
  nrf24_rf_power(18);                 //default TX power is -6dB, pretty strong, reduce to -18dBm for one room (ACK = TX)

  Serial.println("done");
  
  pinMode(4, OUTPUT);

  print_debug();
  
  delay(1000);
  
  Serial.println("entering loop()");
}

void loop()
{
  receive();
}
