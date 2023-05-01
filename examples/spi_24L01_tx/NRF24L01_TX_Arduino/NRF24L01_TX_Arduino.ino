//transmitter code example, transmits an ascending number every TIME_GAP milliseconds in NO_ACK_MODE. can be switched to ACK_MODE
//payload length of 1 or 16 byte, 1Mbps datarate, -6 dbm rf transmit power, channel 32 of 125

extern "C"{
	#include "nrf24l01.h"
}



#define TIME_GAP 1000
uint8_t ascending_number = 0;
char txt[16];



//######### debug fn

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



//######### LED fn

//LED_BUILTIN is pin 13 is SCK of SPI, already using that
void led_on() {
  digitalWrite(4, HIGH);
}

void led_off() {
  digitalWrite(4, LOW);
}


//######### TX fn

uint8_t sendnumber() {
  return nrf24_transmit(&ascending_number, 1, ACK_MODE);
}

uint8_t sendstr() {
  sprintf(txt, "Hello, %u", ascending_number);
  return nrf24_transmit(reinterpret_cast<uint8_t*>(&txt), 16, ACK_MODE);
}

void send() {
  //uint8_t tx_cmd_status = sendnumber();
  uint8_t tx_cmd_status = sendstr();
  switch (tx_cmd_status) {
    case TRANSMIT_BEGIN:
      led_on();
      Serial.println("***   sending package");
      break;
    case TRANSMIT_FAIL:
      Serial.println("EEE   unable to send package");
      break;
  }

  delay(50);					// give the nRF some time to send
  print_debug();

  switch (nrf24_transmit_status()) {
    case TRANSMIT_DONE:
      Serial.print("*OK   sent: ");
      char str[4];
      sprintf(str, "%u", ascending_number);
      Serial.println(str);      //payload is sent without acknowledge successfully
      led_off();
      break;
    case TRANSMIT_FAILED:
      Serial.println("EEE   no ACK received!!");
      break;
    case TRANSMIT_IN_PROGRESS:
      Serial.println("EEE   still transmitting???");
      break;
  }
}



//######### MAIN

void setup()
{
  Serial.begin(115200);

  Serial.print("\r\r\n\nspi_24L01_TX\n\r");

  Serial.print("initializing radio as TX...");
  nrf24_device(TRANSMITTER, RESET);     //initializing nrf24l01+ as a transmitter using one simple function
  nrf24_rf_power(18);                   //default TX power is -6dB, pretty strong, reduce to -18dBm for one room
  Serial.print("done.\n\r");

  pinMode(4, OUTPUT);

  print_debug();

  delay(1000);

  Serial.print("entering loop\n\r");
}

void loop()
{
  delay(TIME_GAP);
  send();
  
  ascending_number++;
  
  char str[4];
  Serial.print("***   next number: ");
  sprintf(str, "%u", ascending_number);
  Serial.println(str);
}
