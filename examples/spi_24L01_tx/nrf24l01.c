#include "nrf24l01.h"

/*nRF24L01+ features, enable / disable as needed*/
static uint8_t NRF24_en_ack = ENABLE;
static uint8_t NRF24_en_no_ack = ENABLE;
static uint8_t NRF24_en_dynamic_payload = ENABLE;

/*global variables related to this file*/
static uint8_t SPI_command;                                       /*1 byte spi command*/
static uint8_t register_current_value;                            /*in order to change some bits of internal registers or to check their content*/
static uint8_t register_new_value;                                /*used to write new value to nrf24l01+ registers*/
static uint8_t write_pointer;                                     /*used as an input for read and write functions (as a pointer)*/
static uint8_t current_address_width;                             /*current address width for receiver pipe addresses (up to 6 pipes), from 3 to 5 bytes*/
static uint8_t reset_flag = 0;                                    /*reset flag lets the software know if the nrf24l01+ has ever been reset or not*/
static uint8_t current_mode = DEVICE_NOT_INITIALIZED;             /*current mode of operation: DEVICE_NOT_INITIALIZED, PRX, PTX, STANDBYI, STANDBYII, POWER_DOWN*/
static uint8_t current_payload_width;                             /*payload width could be from 1 to 32 bytes, in either dynamic or static forms*/
static uint8_t current_acknowledgement_state = NO_ACK_MODE;

/*2 dimensional array of pipe addresses (5 byte address width) by default. you can change addresses using a new array later.
  Pipe 1 address could be anything. pipe 3 to 6 addresses share the first 4 bytes with pipe 2 and only differ in byte 5*/
uint8_t datapipe_address[MAXIMUM_NUMBER_OF_DATAPIPES][ADDRESS_WIDTH_DEFAULT] = {
  {0X20, 0XC3, 0XC2, 0XC1, 0XA0},
  {0X20, 0XC3, 0XC2, 0XC1, 0XA1},
  {0X20, 0XC3, 0XC2, 0XC1, 0XA2},
  {0X20, 0XC3, 0XC2, 0XC1, 0XA3},
  {0X20, 0XC3, 0XC2, 0XC1, 0XA4},
  {0X20, 0XC3, 0XC2, 0XC1, 0XA5}
};

/*function for PTX device to transmit 1 to 32 bytes of data, used for both dynamic payload length
   and static payload length methods. acknowledgemet state could be NO_ACK_MODE or ACK_MODE*/
uint8_t nrf24_transmit(uint8_t *payload, uint8_t payload_width, uint8_t acknowledgement_state)
{
  nrf24_read(STATUS_ADDRESS, &register_current_value, 1, CLOSE);         /*in order to check TX_FIFO status*/
  if ((!(register_current_value & (1 << TX_FULL))) && (current_mode == PTX))
  {
    current_acknowledgement_state = acknowledgement_state;      /*setting the acknowledgement state to either NO_ACK or ACK, based on input*/
    if (NRF24_en_dynamic_payload == ENABLE)
      current_payload_width = payload_width;
    nrf24_send_payload(payload, payload_width);          /*the actual function to send data*/
    return (TRANSMIT_BEGIN);                                     /*TX FIFO is not full and nrf24l01+ mode is standby ii or ptx*/
  }
  else
  {
    return (TRANSMIT_FAIL);            /*TX FIFO full or wrong mode*/
  }
}

/*used by nrf24_transmit function to send the actual data*/
void nrf24_send_payload(uint8_t *payload, uint8_t payload_width)
{
  nrf24_SPI(SPI_ON);
  if (current_acknowledgement_state == NO_ACK_MODE)
    SPI_command = W_TX_PAYLOAD_NOACK;
  else
    SPI_command = W_TX_PAYLOAD;
  SPI_send_command(SPI_command);
  for (; payload_width; payload_width--)
  {
    SPI_command = *payload;
    SPI_send_command(SPI_command);
    payload++;
  }
  nrf24_SPI(SPI_OFF);
}

/*reports back transmit status: TRANSMIT_DONE, TRANSMIT_FAILED (in case of reaching maximum number of retransmits in auto acknowledgement mode)
  and TRANSMIT_IN_PROGRESS, if neither flags are set. automatically resets the '1' flags.*/
uint8_t nrf24_transmit_status()
{
  nrf24_read(STATUS_ADDRESS, &register_current_value, 1, CLOSE);      /*status register is read to check TX_DS flag*/
  if (register_current_value & (1 << TX_DS))                          /*if the TX_DS == 1, */
  {
    nrf24_write(STATUS_ADDRESS, &register_current_value, 1, CLOSE);   /*reseting the TX_DS flag. as mentioned by datasheet, writing '1' to a flag resets that flag*/
    return TRANSMIT_DONE;
  }
  else if (register_current_value & (1 << MAX_RT))
  {
    nrf24_write(STATUS_ADDRESS, &register_current_value, 1, CLOSE);   /*reseting the MAX_RT flag. as mentioned by datasheet, writing '1' to a flag resets that flag*/
    return TRANSMIT_FAILED;
  }
  else
    return TRANSMIT_IN_PROGRESS;
}

/*the receive function output is used as a polling method to check the received data inside RX FIFOs. 
If there is any data available, it will be loaded inside payload array*/
uint8_t nrf24_receive(uint8_t *payload, uint8_t payload_width)
{
  if (current_mode == PRX)
  {
    nrf24_read(STATUS_ADDRESS, &register_current_value, 1, CLOSE);
    if (register_current_value & (1 << RX_DR))                         /*if received data is ready inside RX FIFO*/
    {
      if(NRF24_en_dynamic_payload == DISABLE)                                    /*if dynamif payload width is disabled, use the static payload width and ignore the input*/
        payload_width = current_payload_width;
        
      nrf24_SPI(SPI_ON);                                                /*sending the read payload command to nrf24l01+*/                          
      SPI_command = R_RX_PAYLOAD;
      SPI_send_command(SPI_command);
       
      for (; payload_width; payload_width--)
      {
        SPI_command = NOP_CMD;
        *payload = SPI_send_command(SPI_command);
        payload++;
      }
      nrf24_SPI(SPI_OFF); 
      nrf24_read(FIFO_STATUS_ADDRESS, &register_current_value, 1, CLOSE);   /*in order to check the RX_EMPTY flag*/
      if(register_current_value & (1 << RX_EMPTY))                        /*if the RX FIFO is empty, reset the RX_DR flag inside STATUS register*/
      {
        nrf24_read(STATUS_ADDRESS, &register_current_value, 1, CLOSE);
        register_new_value = register_current_value | (1 << RX_DR);
        nrf24_write(STATUS_ADDRESS, &register_new_value, 1, CLOSE); 
      }      
      return OPERATION_DONE;
    }
    else
    {
      return RECEIVE_FIFO_EMPTY;
    }
  }
  else
    return OPERATION_ERROR;
}

/*function which uses TX_FLUSH or RX_FLUSH command to flush the fifo buffers. if successful, output is OPERATION_DONE.
   if not successful (wrong input or wrong mode of operation) output will be OPERATION_ERROR*/
uint8_t nrf24_flush(uint8_t fifo_select)
{
  switch (fifo_select)
  {
    case TX_BUFFER:
      if (current_mode == PTX)
      {
        nrf24_SPI(SPI_ON);
        SPI_command = FLUSH_TX;
        SPI_send_command(SPI_command);
        nrf24_SPI(SPI_OFF);
        return OPERATION_DONE;
      }
      else
        return OPERATION_ERROR;
    case RX_BUFFER:
      if (current_mode == PRX)
      {
        nrf24_SPI(SPI_ON);
        SPI_command = FLUSH_RX;
        SPI_send_command(SPI_command);
        nrf24_SPI(SPI_OFF);
        return OPERATION_DONE;
      }
      else
        return OPERATION_ERROR;
    default:
      return OPERATION_ERROR;
  }
}

/*must be called atleast once, which happens with calling nrf24_device function*/
void nrf24_reset()
{
  reset_flag = RESET;
  nrf24_CE(CE_OFF);
  register_new_value = CONFIG_REGISTER_DEFAULT;
  nrf24_write(CONFIG_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = EN_AA_REGISTER_DEFAULT;
  nrf24_write(EN_AA_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = EN_RXADDR_REGISTER_DEFAULT;
  nrf24_write(EN_RXADDR_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = SETUP_AW_REGISTER_DEFAULT;
  nrf24_write(SETUP_AW_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = RF_CH_REGISTER_DEFAULT;
  nrf24_write(RF_CH_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = RF_SETUP_REGISTER_DEFAULT;
  nrf24_write(RF_SETUP_ADDRESS, &register_new_value, 1, CLOSE);
  register_new_value = STATUS_REGISTER_DEFAULT;
  nrf24_write(STATUS_ADDRESS, &register_new_value, 1, CLOSE);

  nrf24_mode(PTX);
  nrf24_flush(TX_BUFFER);
  nrf24_mode(PRX);
  nrf24_flush(RX_BUFFER);

  nrf24_read(STATUS_ADDRESS, &register_current_value, 1, CLOSE);
  register_new_value = register_current_value | (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT);
  nrf24_write(STATUS_ADDRESS, &register_new_value, 1, CLOSE);
  
  nrf24_interrupt_mask(ENABLE, ENABLE, ENABLE);
  nrf24_crc_configuration(ENABLE, 1);
  nrf24_address_width(ADDRESS_WIDTH_DEFAULT);
  nrf24_rf_datarate(RF_DATARATE_DEFAULT);
  nrf24_rf_power(RF_PWR_DEFAULT);
  nrf24_rf_channel(RF_CHANNEL_DEFAULT);
  nrf24_datapipe_enable(NUMBER_OF_DP_DEFAULT);
  /*nrf24_datapipe_address_configuration();*/
  /*nrf24_datapipe_ptx(1);*/
  nrf24_prx_static_payload_width(STATIC_PAYLOAD_WIDTH_DEFAULT, NUMBER_OF_DP_DEFAULT);
  nrf24_automatic_retransmit_setup(RETRANSMIT_DELAY_DEFAULT, RETRANSMIT_COUNT_DEFAULT);
  nrf24_auto_acknowledge_datapipe(NUMBER_OF_DP_DEFAULT);
  nrf24_auto_acknowledge_datapipe(0);
  nrf24_dynamic_payload(NRF24_en_dynamic_payload, NUMBER_OF_DP_DEFAULT);
  nrf24_payload_without_ack(NRF24_en_no_ack);
  nrf24_payload_with_ack(NRF24_en_ack);
}

/*used by firmware to set the nrf24 mode in TRANSMITTER, RECEIVER, POWER_SAVING or TURN_OFF states, and reseting the device
  if it has not been done yet. This is the initializer, and everything starts by calling nrf24_device first.It has a higher
  level of abstraction than nrf24_mode and must be used by user*/
void nrf24_device(uint8_t device_mode, uint8_t reset_state)
{
  SPI_Initializer();
  pinout_Initializer();
  delay_function(STARTUP_DELAY);

  if ((reset_state == RESET) || (reset_flag == 0))
  {
    nrf24_reset();
  }

  switch (device_mode)
  {
    case TRANSMITTER:
      nrf24_mode(POWER_DOWN);
      nrf24_interrupt_mask(ENABLE, DISABLE, DISABLE);                /*disabling tx interrupt mask*/
      nrf24_mode(PTX);
      break;
    case RECEIVER:
      nrf24_mode(POWER_DOWN);
      nrf24_interrupt_mask(DISABLE, ENABLE, ENABLE);                /*disabling rx interrupt mask*/
      nrf24_mode(PRX);
      delay_function(PRX_MODE_DELAY);                              /*100ms for PRX mode*/
      break;
    case POWER_SAVING:
      nrf24_mode(POWER_DOWN);
      nrf24_interrupt_mask(ENABLE, ENABLE, ENABLE);
      nrf24_mode(STANDBYI);
      break;
    case TURN_OFF:
      nrf24_mode(POWER_DOWN);
      nrf24_interrupt_mask(ENABLE, ENABLE, ENABLE);
      break;
    default:
      nrf24_mode(POWER_DOWN);
      nrf24_interrupt_mask(ENABLE, ENABLE, ENABLE);
      break;
  }
}

/*setting automatic retransmit delay time and maximum number of retransmits*/
void nrf24_automatic_retransmit_setup(uint16_t delay_time, uint8_t retransmit_count)
{
  register_new_value = 0x00;
  for (; (delay_time > 250) && (register_new_value < 0X0F); delay_time -= 250)
    register_new_value++;
  register_new_value <<= ARD_0;
  if ((retransmit_count > 0) && (retransmit_count < 16))
    register_new_value |= retransmit_count;
  else
    register_new_value |= 0;
  nrf24_write(SETUP_RETR_ADDRESS, &register_new_value, 1, CLOSE);
}

/*setting auto acknoledgement on datapipes*/
void nrf24_auto_acknowledge_datapipe(uint8_t datapipe)
{
  if (datapipe < 7)
    register_new_value = (1 << datapipe);
  nrf24_write(EN_AA_ADDRESS, &register_new_value, 1, CLOSE);
}

/*turns on or off the dynamic payload width capability*/
void nrf24_dynamic_payload(uint8_t state, uint8_t datapipe)
{
  nrf24_auto_acknowledge_datapipe(datapipe);                        /*setting auto acknowledgment before setting dynamic payload*/
  nrf24_auto_acknowledge_datapipe(0);
  nrf24_read(FEATURE_ADDRESS, &register_current_value, 1, CLOSE);
  if (state == ENABLE)
  {
    register_new_value = register_current_value | (1 << EN_DPL);    /*EN_DPL bit turns dynamic payload width on or off on all datapipes*/
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
    if (datapipe < 7)
      register_new_value = (1 << datapipe) - 1;                       /*turning on dynamic payload width on chosen datapipes, using DYNPD register*/
    nrf24_write(DYNPD_ADDRESS, &register_new_value, 1, CLOSE);
    NRF24_en_dynamic_payload = ENABLE;
  }
  else
  {
    register_new_value = register_current_value & (~(1 << EN_DPL));
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
    NRF24_en_dynamic_payload = DISABLE;
  }
}

/*function to enable or disable sending without acknowledge.
   if disabled, TX must send a payload with ACK-request and receiver must be able to answer it.
   manipulates EN_DYN_ACK inside FEATURE*/
void nrf24_payload_without_ack(uint8_t state)
{
  if (state == ENABLE)
  {
    nrf24_read(FEATURE_ADDRESS, &register_current_value, 1, CLOSE);
    register_new_value = register_current_value | (1 << EN_DYN_ACK);
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
  }
  else
  {
    nrf24_read(FEATURE_ADDRESS, &register_current_value, 1, CLOSE);
    register_new_value = register_current_value & (~(1 << EN_DYN_ACK));
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
  }
}

/*function to enable or disable sending with acknowledge.
   if disabled, the payload can be sent only without ACK-request.
   manipulates EN_ACK_PAY and EN_DPL inside FEATURE as Dynamic Payload Length is required.*/
void nrf24_payload_with_ack(uint8_t state)
{
  if (state == ENABLE)
  {
    nrf24_read(FEATURE_ADDRESS, &register_current_value, 1, CLOSE);
    register_new_value = register_current_value | (1 << EN_ACK_PAY) | (1 << EN_DPL);
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
    nrf24_read(DYNPD_ADDRESS, &register_current_value, 1, CLOSE);
	// enable dynamic payload for all pipes
    register_new_value = register_current_value | 0b111111;
    nrf24_write(DYNPD_ADDRESS, &register_new_value, 1, CLOSE);
  }
  else
  {
    nrf24_read(FEATURE_ADDRESS, &register_current_value, 1, CLOSE);
    register_new_value = register_current_value & (~((1 << EN_ACK_PAY) | (1 << EN_DPL)));
    nrf24_write(FEATURE_ADDRESS, &register_new_value, 1, CLOSE);
  }
}

/*on nrf24l01+ there is only one address for PTX device which must be the same as PRX data pipe address 0*/
void nrf24_datapipe_ptx(uint8_t datapipe_number)
{
  nrf24_write(TX_ADDR_ADDRESS, &datapipe_address[datapipe_number - 1][0], current_address_width, CLOSE);
}

/*setting the 6 datapipe addresses using the datapipe_address[][]*/
void nrf24_datapipe_address_configuration()
{
  uint8_t address = RX_ADDR_P0_ADDRESS;
  for (uint8_t counter = 0; counter < 6; counter++)
  {
    nrf24_write(address, &datapipe_address[counter][0], current_address_width, CLOSE);
    address++;
  }
}

/*function to change static payload width, from 1 to 32 bytes in each payload*/
void nrf24_prx_static_payload_width(uint8_t static_payload_width, uint8_t number_of_datapipes)
{
  for (uint8_t address = RX_PW_P0_ADDRESS; number_of_datapipes; number_of_datapipes--)
  {
    nrf24_write(address, &static_payload_width, 1, CLOSE);
    address++;
  }
  current_payload_width = static_payload_width;
}

/*datapipes are turned on and off using EN_RXADDR register, PRX datapipe addresses are located in RX_ADDR_Pn, TX address is located inside TX_ADDR*/
void nrf24_datapipe_enable(uint8_t datapipe)
{
  nrf24_read(EN_RXADDR_ADDRESS, &register_current_value, 1, CLOSE);
  if (NRF24_en_ack) {
    register_current_value |= (1 << ERX_P0);
  }
  register_new_value = register_current_value | (1 << datapipe);
  nrf24_write(EN_RXADDR_ADDRESS, &register_new_value, 1, CLOSE);
}

/*function to set the nrf24l01+ address width, from 3 to 5 bytes*/
void nrf24_address_width(uint8_t address_width)
{
  if ((address_width <= 5) && (address_width >= 3))
  {
    write_pointer = address_width - 2;
  }
  else
  {
    write_pointer = 3;
  }
  nrf24_write(SETUP_AW_ADDRESS, &write_pointer, 1, CLOSE);                    /*5 bytes is the maximum address width available*/
  current_address_width = address_width;
}

/*datarate settings, you can choose between 2mbps, 1mbps, 250kbps*/
void nrf24_rf_datarate(uint16_t rf_datarate)
{
  nrf24_read(RF_SETUP_ADDRESS, &register_current_value, 1, CLOSE);
  register_current_value &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
  switch (rf_datarate)
  {
    case 2000:
      register_new_value = register_current_value | (1 << RF_DR_HIGH);
      break;
    case 1000:
      register_new_value = register_current_value;
      break;
    case 250:
      register_new_value = register_current_value | (1 << RF_DR_LOW);
      break;
    default:
      register_new_value = register_current_value;
      break;
  }
  nrf24_write(RF_SETUP_ADDRESS, &register_new_value, 1, CLOSE);
}

/*nrf24l01+ RF power settings. 0dbm, -6dbm, -12dbm, -18dbm*/
void nrf24_rf_power(uint8_t rf_power)
{
  nrf24_read(RF_SETUP_ADDRESS, &register_current_value, 1, CLOSE);
  register_current_value &= ~((1 << RF_PWR_1) | (1 << RF_PWR_0));
  switch (rf_power)
  {
    case 0:
      register_new_value = register_current_value | ((1 << RF_PWR_1) | (1 << RF_PWR_0));
      break;
    case 6:
      register_new_value = register_current_value | (1 << RF_PWR_1);
      break;
    case 12:
      register_new_value = register_current_value | (1 << RF_PWR_0);
      break;
    case 18:
      register_new_value = register_current_value;
      break;
    default:
      register_new_value = register_current_value | (1 << RF_PWR_1);
      break;
  }
  nrf24_write(RF_SETUP_ADDRESS, &register_new_value, 1, CLOSE);
}

/*read whether the current channel is busy (has traffic), needs to be called from RX mode*/
uint8_t nrf24_rf_channel_read_busy(uint8_t rf_channel)
{
  uint8_t signals_detected;
  nrf24_read(RPD_REG_ADDRESS, &signals_detected, 1, CLOSE);
  if (signals_detected) {
    return CHANNEL_BUSY;
  }
  else {
    return CHANNEL_CLEAR;
  }
}

/*test whether a channel is busy (has traffic), waiting for ms_to_test*/
uint8_t nrf24_rf_channel_test_busy(uint8_t rf_channel, uint16_t ms_to_test)
{
  if ((rf_channel <= 125) && (rf_channel >= 1))
  {
    // back up old channel
    uint8_t previous_channel;
    nrf24_read(RF_CH_ADDRESS, &previous_channel, 1, CLOSE);
    // back up old mode
    uint8_t previous_mode = current_mode;
    // switch to new channel
    nrf24_rf_channel(rf_channel);
    // switch to RX, Received Power Detector is set to 0 and begins sampling
    if (previous_mode != PRX) {
      nrf24_mode(PRX);
    }
    // wait at least 1 ms before declaring channel clear
    delay_function(1 > ms_to_test ? 1 : ms_to_test);
    // Received Power Detector latches to 1 if there was a signal >-64dBm for at least 40 uS consecutively since RX mode was enabled
    uint8_t signals_detected = nrf24_rf_channel_read_busy(rf_channel);
    // switch back to old channel
    nrf24_rf_channel(previous_channel);
    // switch back to old mode
    if (previous_mode != PRX) {
      nrf24_mode(previous_mode);
    }
    if (signals_detected) {
      return CHANNEL_BUSY;
    }
    else {
      return CHANNEL_CLEAR;
    }
  }
  else
  {
		return CHANNEL_BUSY;
  }
}

/*nrf24l01+ RF channel selection, from 1 to 125*/
void nrf24_rf_channel(uint8_t rf_channel)
{
  if ((rf_channel <= 125) && (rf_channel >= 1))
  {
    uint8_t write_pointer = rf_channel;
    nrf24_write(RF_CH_ADDRESS, &write_pointer, 1, CLOSE);
  }
  else
  {
    uint8_t write_pointer = 1;
    nrf24_write(RF_CH_ADDRESS, &write_pointer, 1, CLOSE);
  }
}

/*interrupt mask settings. 3 seperate masks for RX, TX, and RT (maximum numbers of retransmission reached*/
void nrf24_interrupt_mask(uint8_t rx_mask, uint8_t tx_mask, uint8_t max_rt_mask)
{
  nrf24_read(CONFIG_ADDRESS, &register_current_value, 1, CLOSE);
  if (rx_mask)
    register_new_value = (register_current_value) | (1 << MASK_RX_DR);
  else
    register_new_value &= (~(1 << MASK_RX_DR));
  if (tx_mask)
    register_new_value |= (1 << MASK_TX_DS);
  else
    register_new_value &= (~(1 << MASK_TX_DS));
  if (max_rt_mask)
    register_new_value |= (1 << MASK_MAX_RT);
  else
    register_new_value &= (~(1 << MASK_MAX_RT));

  nrf24_write(CONFIG_ADDRESS, &register_new_value, 1, CLOSE);
}

/*enabling or disabling crc in payload; setting crc encoding scheme between 1 or 2 bytes*/
void nrf24_crc_configuration(uint8_t crc_enable, uint8_t crc_encoding_scheme)
{
  nrf24_read(CONFIG_ADDRESS, &register_current_value, 1, CLOSE);
  if (crc_enable)
    register_new_value = (register_current_value) | (1 << EN_CRC);
  else
    register_new_value &= (~(1 << EN_CRC));
  if (crc_encoding_scheme == 2)
    register_new_value |= (1 << CRCO);
  else
    register_new_value &= (~(1 << CRCO));

  nrf24_write(CONFIG_ADDRESS, &register_new_value, 1, CLOSE);
}

/*mode selector: power down, standby i, standby ii, ptx, prx. used by nrf24_device function*/
void nrf24_mode(uint8_t mode)
{
  nrf24_read(CONFIG_ADDRESS, &register_current_value, 1, CLOSE);
  switch (mode)
  {
    case POWER_DOWN:
      nrf24_CE(CE_OFF);
      register_new_value = (register_current_value) & (~(1 << PWR_UP));
      delay_function(POWER_DOWN_DELAY);
      break;
    case STANDBYI:                                 /*standby I is defined by 'PWR_UP = 1' and 'CE pin LOW'*/
      nrf24_CE(CE_OFF);
      register_new_value = (register_current_value) | (1 << PWR_UP);
      delay_function(STANDBYI_DELAY);
      break;
    case STANDBYII:                                 /*standby ii is related to a ptx device*/
      nrf24_CE(CE_ON);
      register_new_value = ((register_current_value) | (1 << PWR_UP)) & (~(1 << PRIM_RX));
      delay_function(STANDBYI_DELAY);
      break;
    case PTX:
      nrf24_CE(CE_ON);
      register_new_value = ((register_current_value) | (1 << PWR_UP)) & (~(1 << PRIM_RX));
      delay_function(STANDBYI_DELAY);
      break;
    case PRX:
      nrf24_CE(CE_ON);
      register_new_value = (register_current_value) | (1 << PWR_UP) | (1 << PRIM_RX);
      delay_function(STANDBYI_DELAY);
      break;
    default:
      nrf24_CE(CE_OFF);
      register_new_value = (register_current_value) & (~(1 << PWR_UP));
      delay_function(POWER_DOWN_DELAY);
      break;
  }
  nrf24_write(CONFIG_ADDRESS, &register_new_value, 1, CLOSE);
  current_mode = mode;
}

/*reads the number of bytes (data_length) from the register in nrf24l01+ (address) and stores them inside an array (value),
  then closes the spi connection (spi_state = CLOSE) or leaves it open (spi_state = OPEN)*/
void nrf24_read(uint8_t address, uint8_t *value, uint8_t data_length, uint8_t spi_state)
{
  nrf24_SPI(SPI_ON);
  SPI_command = R_REGISTER | address;    /*in order to read CONFIG, then change one bit*/
  SPI_send_command(SPI_command);
  SPI_command = NOP_CMD;
  for (; data_length ; data_length--)
  {
    *value = SPI_send_command(SPI_command);
    value++;
  }
  if (spi_state == CLOSE)
    nrf24_SPI(SPI_OFF);
}

/*writes the number of bytes (data_length) from an array (value) inside registers in nrf24l01+ (address),
  then closes the spi connection (spi_state = CLOSE) or leaves it open (spi_state = OPEN)*/
void nrf24_write(uint8_t address, uint8_t *value, uint8_t data_length, uint8_t spi_state)
{
  nrf24_SPI(SPI_ON);
  SPI_command = W_REGISTER | address;    /*in order to read CONFIG, then change one bit*/
  SPI_send_command(SPI_command);
  for (; data_length ; data_length--)
  {
    SPI_command = *value;
    value++;
    SPI_send_command(SPI_command);
  }
  if (spi_state == CLOSE)
    nrf24_SPI(SPI_OFF);
}
