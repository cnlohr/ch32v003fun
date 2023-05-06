#ifndef NRF24L01_H
#define NRF24L01_H
/*nrf24l01: MSbit to LSbit, LSbyte to MSbyte*/
#include <stdio.h>
#include <stdint.h>

#define STARTUP_DELAY                 150             /*in milliseconds*/
#define POWER_DOWN_DELAY              2
#define STANDBYI_DELAY                2
#define PRX_MODE_DELAY                100
#define ADDRESS_WIDTH_DEFAULT         5               /*address width in bytes, for default value*/
#define RF_CHANNEL_DEFAULT            32        
#define RF_DATARATE_DEFAULT           1000            /*250, 1000, 2000*/
#define RF_PWR_DEFAULT                6               /*0, -6, -12, -18*/
#define STATIC_PAYLOAD_WIDTH_DEFAULT  1               /*for static payload mode, configurable between 1 and 32 bytes for PRX device ONLY (RX_PW_Pn, n for data pipe n)(no register for payload length in PTX device)*/
#define NUMBER_OF_DP_DEFAULT          1               /*number of datapipes, 1 to 6*/ 
#define RETRANSMIT_DELAY_DEFAULT      500             /*in uS*/
#define RETRANSMIT_COUNT_DEFAULT      3
 
#define OPEN                          1
#define CLOSE                         0
#define ENABLE                        1
#define DISABLE                       0
#define SPI_OFF                       1
#define SPI_ON                        0
#define CE_OFF                        0
#define CE_ON                         1

#define CONFIG_REGISTER_DEFAULT       0X08
#define EN_AA_REGISTER_DEFAULT        0X3F
#define EN_RXADDR_REGISTER_DEFAULT    0X00
#define SETUP_AW_REGISTER_DEFAULT     0X03
#define SETUP_RETR_REGISTER_DEFAULT   0X03
#define RF_CH_REGISTER_DEFAULT        0X02
#define RF_SETUP_REGISTER_DEFAULT     0X0E
#define STATUS_REGISTER_DEFAULT       0X0E
#define MAXIMUM_NUMBER_OF_DATAPIPES   6

#define POWER_DOWN                    0X00
#define STANDBYI                      0X01
#define STANDBYII                     0X02
#define PTX                           0X03
#define PRX                           0X04
#define DEVICE_NOT_INITIALIZED        0X05

#define TRANSMITTER                   0X00
#define RECEIVER                      0X01
#define POWER_SAVING                  0X02
#define TURN_OFF                      0X03

#define RESET                         1
#define NO_RESET                      0
#define NO_ACK_MODE                   1
#define ACK_MODE                      0
#define TRANSMIT_BEGIN                1
#define TRANSMIT_FAIL                 0
#define TRANSMIT_IN_PROGRESS          0
#define TRANSMIT_DONE                 1
#define TRANSMIT_FAILED               0XFF
#define OPERATION_DONE                1
#define OPERATION_ERROR               0
#define RECEIVE_FIFO_EMPTY            2
#define TX_BUFFER                     1
#define RX_BUFFER                     0
// return states for nrf24_rf_channel_test_busy
#define CHANNEL_CLEAR                 0
#define CHANNEL_BUSY                  1

/*bits definition section*/
#define MASK_RX_DR          6               /*mask interrupt caused by RX_DR: 1 interrupt not reflected on IRQ pin (IRQ is active low), inside CONFIG register*/
#define MASK_TX_DS          5               /*mask interrupt caused by TX_DS: 1 interrupt not reflected on IRQ pin (IRQ is active low), inside CONFIG register*/
#define MASK_MAX_RT         4               /*mask interrupt caused by MAX_RT means maximum number of retransmissions reached: 1 interrupt not reflected on IRQ pin (IRQ is active low), inside CONFIG register*/
#define EN_CRC              3               /*enale CRC, forced high if one of the bits in EN_AA is high, inside CONFIG register*/
#define CRCO                2               /*CRC encoding scheme, 0 is 1 byte, 1 is 2 bytes, inside CONFIG register*/
#define PWR_UP              1               /*1 is power up, inside CONFIG register*/
#define PRIM_RX             0               /*RX/TX control, 1: PRX, inside CONFIG register*/
#define ENAA_P5             5               /*enable auto acknowledgement data pipe 5*/
#define ENAA_P4             4
#define ENAA_P3             3
#define ENAA_P2             2
#define ENAA_P1             1
#define ENAA_P0             0
#define ERX_P5              5               /*part of EN_RXADDR, enable data pipe 5*/
#define ERX_P4              4
#define ERX_P3              3
#define ERX_P2              2
#define ERX_P1              1
#define ERX_P0              0
#define AW_1                1               /*RX/TX address field width, 00 illegal, 01 3 bytes, 10 4 bytes, 11 5 bytes*/
#define AW_0                0
#define ARD_3               7               /*auto retransmit delay, 0000 250us, 0001 500us ...> 1111 4000us*/
#define ARD_2               6
#define ARD_1               5
#define ARD_0               4
#define ARC_3               3               /*auto retransmit count, 0000 retransmit deisabled, 1111 up to 15 retransmit on failure of AA. (inside SETUP_RETR register)*/
#define ARC_2               2
#define ARC_1               1
#define ARC_0               0
#define RF_CH_6             6               /*sets the frequencvy channel nRF24L01+ operates on*/
#define RF_CH_5             5
#define RF_CH_4             4
#define RF_CH_3             3
#define RF_CH_2             2
#define RF_CH_1             1
#define RF_CH_0             0
#define CONT_WAVE           7               /*enables continuous carrier transmit when high*/
#define RF_DR_LOW           5               /*sets the RF data rate to 250kbps*/
#define PLL_LOCK            4               /*force PLL lock signal. used for testing ONLY*/
#define RF_DR_HIGH          3               /*select between high speed data rates and works ONLY when RF_DR_LOW is 0. 0 for 1Mbps, 1 for 2Mbps*/
#define RF_PWR_1            2
#define RF_PWR_0            1
#define RX_DR               6               /*IRQ for new packet in RX FIFO (newly received)*/
#define TX_DS               5               /*IRQ for ACK received in TX mode*/
#define MAX_RT              4 
#define RX_P_NO_2           3
#define RX_P_NO_1           2
#define RX_P_NO_0           1
#define PLOS_CNT_3          7               /*inside OBSERVE_TX register, counts the total number of retransmissions since last channel change. reset by writing to RF_CH*/
#define PLOS_CNT_2          6
#define PLOS_CNT_1          5
#define PLOS_CNT_0          4
#define ARC_CNT_3           3               /*inside OBSERVE_TX register, counts the number of retransmissions for current transaction. reset by initiating new transaction*/
#define ARC_CNT_2           2
#define ARC_CNT_1           1
#define ARC_CNT_0           0
#define RPD                 0               /*received power detector, if received power is less than -64dbm, RPD = 0*/
#define TX_REUSE            6
#define TX_FULL             5
#define TX_EMPTY            4
#define RX_FULL             1
#define RX_EMPTY            0
#define DPL_P5              5
#define DPL_P4              4
#define DPL_P3              3
#define DPL_P2              2
#define DPL_P1              1
#define DPL_P0              0                 /*must be set on PTX in dynamic payload length mode*/
#define EN_DPL              2                 /*set to enable dynamic payload length*/
#define EN_ACK_PAY          1                 /*used to enable auto acknowledgement with payload in PRX (inside FEATURE register)*/
#define EN_DYN_ACK          0                 /**/

/*registers definition section*/
#define CONFIG_ADDRESS              0X00
#define EN_AA_ADDRESS               0X01              /*enable auto acknowledgement feature*/
#define EN_RXADDR_ADDRESS           0X02              /*register containing bits to enable 6 data pipes individually*/
#define SETUP_AW_ADDRESS            0X03              /*address field length is configured in here to be 3, 4 or 5 bytes long*/
#define SETUP_RETR_ADDRESS          0X04              /*setup ARC bits to configure auto retransmission count*/
#define RF_CH_ADDRESS               0X05
#define RF_SETUP_ADDRESS            0X06
#define STATUS_ADDRESS              0X07              /*contains RX_DR, TX_DS, MAX_RT, RX_P_NO, TX_FULL, send R_REGISTER then NOP to read*/ 
#define OBSERVE_TX_ADDRESS          0X08              /*contains ARC_CNT and PLOS_CNT, two counters for retransmission. these counters could be used to assess the network quality*/
#define RPD_REG_ADDRESS             0X09
#define RX_ADDR_P0_ADDRESS          0X0A              /*the address for PRX device. if a packet contains this address, enhanced shockburst starts validating the packet*/
#define RX_ADDR_P1_ADDRESS          0X0B              /*a total of 6 unique addresses could be assigned to a PRX device (Multiceiver feature)*/
#define RX_ADDR_P2_ADDRESS          0X0C              /*these addresses must NOT be the same*/
#define RX_ADDR_P3_ADDRESS          0X0D
#define RX_ADDR_P4_ADDRESS          0X0E
#define RX_ADDR_P5_ADDRESS          0X0F
#define TX_ADDR_ADDRESS             0X10              /*40 bits long register, transmit address, used for a PTX device only. configure address legth in SETUP_AW register. set RX_ADDR_P0 equal to this address to handle automatic acknowledge*/
#define RX_PW_P0_ADDRESS            0X11              /*these registers are for setting the static payload length in static payload length mode (receiver side)*/
#define RX_PW_P1_ADDRESS            0X12
#define RX_PW_P2_ADDRESS            0X13
#define RX_PW_P3_ADDRESS            0X14
#define RX_PW_P4_ADDRESS            0X15
#define RX_PW_P5_ADDRESS            0X16
#define FIFO_STATUS_ADDRESS         0X17
#define DYNPD_ADDRESS               0X1C              /*on receiver side (RX mode), this register must be set to enable dynamic payload length. a PTX in dynamic mode, must have the DYNPD_P0 set*/
#define FEATURE_ADDRESS             0X1D              /*contains the EN_DPL bit to enable dynamic payload length*/

/*commands definition section*/
#define R_REGISTER          0X00              /*read commmand and STATUS registers, 5 bit register map address*/
#define W_REGISTER          0X20              /*write commmand and STATUS registers, 5 bit register map address, executable in POWER DOWN or STANDBY modes only*/
#define R_RX_PAYLOAD        0X61              /*read RX payload, 1-32 bytes. read operation starts at byte 0. payload is deleted from FIFO after its read*/
#define W_TX_PAYLOAD        0XA0              /*write TX payload, starts at byte 0, 1-32 bytes*/
#define FLUSH_TX            0XE1              /*flush TX FIFO, used in TX mode*/
#define FLUSH_RX            0XE2              /*flush RX FIFO, used in RX mode*/
#define REUSE_TX_PL         0XE3              /*used for a PTX device, reuse last transmitted payload for an exact number. alternative to auto retransmission*/
#define R_RX_PL_WID         0X60              /*command for receiver side, in order to read the payload length in dynamic payload length mode*/
#define W_ACK_PAYLOAD       0XA0              /*used in RX mode, to write payload in TX FIFO and later transmit the payloads along with ACK packet to PTX, if DPL is enabled*/
#define W_TX_PAYLOAD_NOACK  0XB0              /*used in TX mode, disables AUTOACK on this specific packet. must be first enabled in FEATURE register by setting the EN_DYN_ACK bit. if used, PTX will not wait for ACK and goes directly to standby I*/
#define NOP_CMD             0XFF              /*might be used to read the status register*/

void nrf24_reset();                            
void nrf24_device(uint8_t device_mode, uint8_t reset_state);
uint8_t SPI_send_command(uint8_t command);          
void pinout_Initializer();         
void SPI_Initializer();
void nrf24_mode(uint8_t mode);
void nrf24_SPI(uint8_t input);
void nrf24_CE(uint8_t input);
void nrf24_address_width(uint8_t address_width);
uint8_t nrf24_rf_channel_read_busy(uint8_t rf_channel);
uint8_t nrf24_rf_channel_test_busy(uint8_t rf_channel, uint16_t ms_to_test);
void nrf24_rf_channel(uint8_t rf_channel);
void nrf24_rf_power(uint8_t rf_power);
void nrf24_rf_datarate(uint16_t rf_datarate);
void nrf24_read(uint8_t address, uint8_t *value, uint8_t data_length, uint8_t spi_state);
void nrf24_write(uint8_t address, uint8_t *value, uint8_t data_length, uint8_t spi_state);
void delay_function(uint32_t duration_ms);
void nrf24_crc_configuration(uint8_t crc_enable, uint8_t crc_encoding_scheme);
void nrf24_interrupt_mask(uint8_t rx_mask, uint8_t tx_mask, uint8_t max_rt_mask);
void nrf24_datapipe_enable(uint8_t number_of_datapipes);
void nrf24_prx_static_payload_width(uint8_t static_payload_width, uint8_t number_of_datapipes);
void nrf24_datapipe_address_configuration();
void nrf24_datapipe_ptx(uint8_t datapipe_number);
void nrf24_automatic_retransmit_setup(uint16_t delay_time, uint8_t retransmit_count);
void nrf24_auto_acknowledge_datapipe(uint8_t datapipe);
void nrf24_payload_without_ack(uint8_t state);
void nrf24_payload_with_ack(uint8_t state);
void nrf24_dynamic_payload(uint8_t state, uint8_t datapipe);
void nrf24_device(uint8_t device_mode, uint8_t reset_state);
void nrf24_send_payload(uint8_t *payload, uint8_t payload_width);
uint8_t nrf24_receive(uint8_t *payload, uint8_t payload_width);
uint8_t nrf24_transmit(uint8_t *payload, uint8_t payload_width, uint8_t acknowledgement_state);
uint8_t nrf24_transmit_status();
void nrf24_dynamic_ack(uint8_t state);
uint8_t nrf24_flush(uint8_t fifo_select);

#endif
