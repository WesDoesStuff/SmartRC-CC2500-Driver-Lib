/*
  Based on:
            CC2500.cpp module library
            Copyright (c) 2010 Michael.
              Author: Michael, <www.elechouse.com>
              Version: November 12, 2010

  This library is designed to use CC2500 module on Arduino platform.
  CC2500 module is an useful wireless module. Using the functions of the 
  library, you can easily send and receive data by the CC2500 module. 
  Just have fun!
  For the details, please refer to the datasheet of CC2500.
  
    ________________________________________________________
    |         MARCSATE - Main Radio Control FSM State      |
    |------------------------------------------------------|
    |  Value     |  State name       |  State              |
    |  0 (0x00)  |  SLEEP            |  SLEEP              |
    |  1 (0x01)  |  IDLE             |  IDLE               |
    |  2 (0x02)  |  XOFF             |  XOFF               |
    |  3 (0x03)  |  VCOON_MC         |  MANCAL             |
    |  4 (0x04)  |  REGON_MC         |  MANCAL             |
    |  5 (0x05)  |  MANCAL           |  MANCAL             |
    |  6 (0x06)  |  VCOON            |  FS_WAKEUP          |
    |  7 (0x07)  |  REGON            |  FS_WAKEUP          |
    |  8 (0x08)  |  STARTCAL         |  CALIBRATE          |
    |  9 (0x09)  |  BWBOOST          |  SETTLING           |
    | 10 (0x0A)  |  FS_LOCK          |  SETTLING           |
    | 11 (0x0B)  |  IFADCON          |  SETTLING           |
    | 12 (0x0C)  |  ENDCAL           |  CALIBRATE          |
    | 13 (0x0D)  |  RX               |  RX                 |
    | 14 (0x0E)  |  RX_END           |  RX                 |
    | 15 (0x0F)  |  RX_RST           |  RX                 |
    | 16 (0x10)  |  TXRX_SWITCH      |  TXRX_SETTLING      |
    | 17 (0x11)  |  RXFIFO_OVERFLOW  |  RXFIFO_OVERFLOW    |
    | 18 (0x12)  |  FSTXON           |  FSTXON             |
    | 19 (0x13)  |  TX               |  TX                 |
    | 20 (0x14)  |  TX_END           |  TX                 |
    | 21 (0x15)  |  RXTX_SWITCH      |  RXTX_SETTLING      |
    | 22 (0x16)  |  TXFIFO_UNDERFLOW |  TXFIFO_UNDERFLOW   |
    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
----------------------------------------------------------------------------------------------------------------
cc2500 Driver. Wesley Smoyak [Feat. Little Satan and Wilson Shen (ELECHOUSE).]
----------------------------------------------------------------------------------------------------------------
*/
#ifndef CC2500_SRC_DRV_h
#define CC2500_SRC_DRV_h

#include <Arduino.h>

//***************************************CC2500 define**************************************************//

// CC2500 CONFIG REGSITER
#define CC2500_IOCFG2		    	0x00	// GDO2Output Pin Configuration
#define CC2500_IOCFG1			    0x01	// GDO1Output Pin Configuration
#define CC2500_IOCFG0		    	0x02	// GDO0Output Pin Configuration
#define CC2500_FIFOTHR			  0x03	// RX FIFO and TX FIFO Thresholds
#define CC2500_SYNC1			    0x04	// Sync Word, High Byte
#define CC2500_SYNC0			    0x05	// Sync Word, Low Byte
#define CC2500_PKTLEN		    	0x06	// Packet Length
#define CC2500_PKTCTRL1			  0x07	// Packet Automation Control
#define CC2500_PKTCTRL0		  	0x08	// Packet Automation Control
#define CC2500_ADDR			    	0x09	// Device Address
#define CC2500_CHANNR			    0x0A	// Channel Number
#define CC2500_FSCTRL1		  	0x0B	// Frequency Synthesizer Control
#define CC2500_FSCTRL0		  	0x0C	// Frequency Synthesizer Control
#define CC2500_FREQ2		    	0x0D	// Frequency Control Word, High Byte
#define CC2500_FREQ1			    0x0E	// Frequency Control Word, Middle Byte
#define CC2500_FREQ0			    0x0F	// Frequency Control Word, Low Byte
#define CC2500_MDMCFG4		  	0x10	// Modem Configuration
#define CC2500_MDMCFG3		  	0x11	// Modem Configuration
#define CC2500_MDMCFG2		  	0x12	// Modem Configuration
#define CC2500_MDMCFG1			  0x13	// Modem Configuration
#define CC2500_MDMCFG0		  	0x14	// Modem Configuration
#define CC2500_DEVIATN			  0x15	// Modem Deviation Setting
#define CC2500_MCSM2			    0x16	// Main Radio Control State Machine Configuration
#define CC2500_MCSM1		    	0x17	// Main Radio Control State Machine Configuration
#define CC2500_MCSM0			    0x18	// Main Radio Control State Machine Configuration
#define CC2500_FOCCFG		     	0x19	// Frequency Offset Compensation Configuration
#define CC2500_BSCFG			    0x1A	// Bit Synchronization Configuration
#define CC2500_AGCCTRL2		  	0x1B	// AGC Control
#define CC2500_AGCCTRL1			  0x1C	// AGC Control
#define CC2500_AGCCTRL0			  0x1D	// AGC Control
#define CC2500_WOREVT1		  	0x1E	// High Byte Event0 Timeout
#define CC2500_WOREVT0		  	0x1F	// Low Byte Event0 Timeout
#define CC2500_WORCTRL			  0x20	// Wake On Radio Control
#define CC2500_FREND1			    0x21	// Front End RX Configuration
#define CC2500_FREND0		    	0x22	// Front End TX configuration
#define CC2500_FSCAL3			    0x23	// Frequency Synthesizer Calibration
#define CC2500_FSCAL2			    0x24	// Frequency Synthesizer Calibration
#define CC2500_FSCAL1		  	  0x25	// Frequency Synthesizer Calibration
#define CC2500_FSCAL0			    0x26	// Frequency Synthesizer Calibration
#define CC2500_RCCTRL1	   		0x27	// RC Oscillator Configuration
#define CC2500_RCCTRL0			  0x28	// RC Oscillator Configuration
#define CC2500_FSTEST		    	0x29	// Frequency Synthesizer Calibration Control
#define CC2500_PTEST		    	0x2A	// Writing 0xBF to this register makes the on-chip temperature sensor available in the IDLE state. The default 0x7F value should then be written back before leaving the IDLE state. Other use of this register is for test only
#define CC2500_AGCTEST		  	0x2B	// AGC Test
#define CC2500_TEST2		    	0x2C	// Set to 0x81 for improved sensitivity at data rates ≤100 kBaud. The temperature range is then from 0oC to +85oC.
#define CC2500_TEST1			    0x2D	// Set to 0x35 for improved sensitivity at data rates ≤100 kBaud. The temperature range is then from 0oC to +85oC.
#define CC2500_TEST0		    	0x2E	// Various Test Settings

//CC2500 Strobe commands
#define CC2500_SRES			    	0x30	// Reset chip
#define CC2500_SFSTXON		   	0x31	// Enable and calibrate frequency synthesizer
#define CC2500_SXOFF		    	0x32	// Turn off crystal oscillator
#define CC2500_SCAL			    	0x33	// Calibrate frequency synthesizer and turn it off
#define CC2500_SRX				    0x34	// Enable RX
#define CC2500_STX			    	0x35	// Enable TX
#define CC2500_SIDLE			    0x36	// Exit Radio mode
#define CC2500_SWOR			    	0x38	// Start automatic RX polling
#define CC2500_SPWD		    		0x39	// Enter power down mode when CSn goes high.
#define CC2500_SFRX			  	  0x3A	// Flush the RX FIFO buffer.
#define CC2500_SFTX			    	0x3B	// Flush the TX FIFO buffer.
#define CC2500_SWORRST		  	0x3C	// Reset real time clock
#define CC2500_SNOP				    0x3D	// No operation. May be used to get access to the chip status byte.

//CC2500 STATUS REGISTER
#define CC2500_PARTNUM			  0x30	// Chip ID
#define CC2500_VERSION			  0x31	// Chip ID
#define CC2500_FREQEST		   	0x32	// Frequency Offset Estimate from Demodulator
#define CC2500_LQI			    	0x33	// Demodulator Estimate for Link Quality
#define CC2500_RSSI			  	  0x34	// Received Signal Strength Indication
#define CC2500_MARCSTATE	  	0x35	// Main Radio Control State Machine State
#define CC2500_WORTIME1		  	0x36	// High Byte of WOR Time
#define CC2500_WORTIME0		  	0x37	// Low Byte of WOR Time
#define CC2500_PKTSTATUS		  0x38	// Current GDOxStatus and Packet Status
#define CC2500_VCO_VC_DAC		  0x39	// Current Setting from PLL Calibration Module
#define CC2500_TXBYTES			  0x3A	// Underflow and Number of Bytes
#define CC2500_RXBYTES        0x3B	// Underflow and Number of Bytes
#define CC2500_RCCTRL1_STATUS 0x3C	// Last RC Oscillator Calibration Result
#define CC2500_RCCTRL0_STATUS 0x3D	// Last RC Oscillator Calibration Result

//CC2500 PATABLE,TXFIFO,RXFIFO
#define CC2500_PATABLE			  0x3E	// power output table
#define CC2500_TX_FIFO			  0x3F	// transmit FIFO
#define CC2500_RX_FIFO			  0x3F	// receive FIFO

//************************************* class **************************************************//
class CC2500 {
private:
  void SpiStart(void);
  void SpiEnd(void);
  void GDO_Set (void);
  void GDO0_Set (void);
  void Reset (void);
  void setSpi(void);
  void RegConfigSettings(void);
  void Calibrate(void);
  void Split_PKTCTRL0(void);
  void Split_PKTCTRL1(void);
  void Split_MCSM0(void);
  void Split_MDMCFG1(void);
  void Split_MDMCFG2(void);
  void Split_MDMCFG4(void);
  
public:
  void Init(void);
  byte SpiReadStatus(byte addr);
  void setSpiPin(byte sck, byte miso, byte mosi, byte ss);
  void addSpiPin(byte sck, byte miso, byte mosi, byte ss, byte modul);
  void setGDO(byte gdo0, byte gdo2);
  void setGDO0(byte gdo0);
  void addGDO(byte gdo0, byte gdo2, byte modul);
  void addGDO0(byte gdo0, byte modul);
  void setModul(byte modul);
//  void setCCMode(bool s);
  void setModulation(byte m);
  void setPA(int p);
  void setMHZ(float mhz);
  void setChannel(byte chnl);
  void setChsp(float f);
  void setRxBW(float f);
  void setDRate(float d);
  void setDeviation(float d);
  void SetTx(void);
  void SetRx(void);
  void SetTx(float mhz);
  void SetRx(float mhz);
   int getRssi(void);
  byte getLqi(void);
  void setSres(void);
  void setSidle(void);
  void goSleep(void);
  void SendData(byte *txBuffer, byte size);
  void SendData(char *txchar);
  void SendData(byte *txBuffer, byte size, int t);
  void SendData(char *txchar, int t);
  byte CheckReceiveFlag(void);
  byte ReceiveData(byte *rxBuffer);
  bool CheckCRC(void);
  void SpiStrobe(byte strobe);
  void SpiWriteReg(byte addr, byte value);
  void SpiWriteBurstReg(byte addr, byte *buffer, byte num);
  byte SpiReadReg(byte addr);
  void SpiReadBurstReg(byte addr, byte *buffer, byte num);
  void setClb(byte b, byte s, byte e);
  bool getCC2500(void);
  byte getMode(void);
  void setSyncWord(byte sh, byte sl);
  void setAddr(byte v);
  void setWhiteData(bool v);
  void setPktFormat(byte v);
  void setCrc(bool v);
  void setLengthConfig(byte v);
  void setAutoCal(byte v);
  void setPOTimeout(byte v);
  void setPinCtrl(bool v);
  void setXOSCForceOn(bool v);
  void setPacketLength(byte v);
  void setDcFilterOff(bool v);
  void setManchester(bool v);
  void setSyncMode(byte v);
  void setFEC(bool v);
  void setPRE(byte v);
  void setPQT(byte v);
  void setCRC_AF(bool v);
  void setAppendStatus(bool v);
  void setAdrChk(byte v);
  bool CheckRxFifo(int t);
};

extern CC2500 cc2500;

#endif
